#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_controller_serial.h"
#include <expio.h>
#include <systemc.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using yapsc::controller_serial;

controller_serial::controller_serial(const char *name)
    : yapsc::controller(name) {
	ending = 0;
}

controller_serial::~controller_serial() {}

void controller_serial::before_start(void) {}

void controller_serial::start(void) {
	before_start();

	while (true) {
		for (int i = 1; i <= domains; i++) {
			yapsc::Message p;
			std::string *s = recv_string(i, false);
			if (!s) {
				if (ending && !ending_notify[i - 1]) {
					EXPIO_LOG_DBG("Ending. Notifing: %d",
						      i);
					p.Clear();
					p.set_type(p.END);
					p.set_domain(0);
					p.set_status(ending_status);
					s = new std::string();
					if (!p.SerializeToString(s)) {
						EXPIO_LOG_CRIT(
						    "Serialize error");
					}
					send_string(s, i);
					ending_notify[i - 1] = true;
					delete (s);
				}
				continue;
			}

			p.Clear();
			if (!p.ParseFromString(*s)) {
				EXPIO_LOG_CRIT("Parser error");
			}
			delete (s);

			bool run = false;

			run |= action_socket_registry(&p, i);
			run |= action_socket_query(&p, i);
			run |= action_barrier(&p, i);
			run |= action_syscall(&p, i);

			if (p.type() == p.END) {
				action_end(i, p.status());

				if (!ending) {
					return;
				}
			} else if (!run) {
				EXPIO_LOG_CRIT("Unknown type %d from domain %d",
					       p.type(), p.domain());
			}
		}
	}
}

void controller_serial::action_end(yapsc_domain_t domain, int status) {
	if (!ending) {
		ending = domains;
		ending_status = status;
		if (ending_status) {
			success_simulation = NULL;
			if (error_simulation)
				error_simulation();
			error_simulation = NULL;
		} else {
			error_simulation = NULL;
			if (success_simulation)
				success_simulation();
			success_simulation = NULL;
		}
		ending_notify = (bool *)calloc(domains, sizeof(bool));
	}
	ending_notify[domain - 1] = true;
	ending--;
	EXPIO_LOG_DBG("End requested. Missing: %d", ending);
}

bool controller_serial::action_socket_registry(yapsc::Message *p,
					       yapsc_domain_t domain) {
	if (p->type() != p->TARGET_SOCKET_REGISTRY) {
		return false;
	}
	EXPIO_LOG_DBG("Registering %s to domain %d at port %d",
		      p->socketname().c_str(), p->domain(), p->port());

	yapsc_domain_port_pair addr;
	addr.first = p->domain();
	addr.second = p->port();
	targets.insert(yapsc_serial_target_pair(p->socketname(), addr));
	return true;
}

const std::string *
controller_serial::domain_address_get(yapsc_domain_t domain) {
	static std::string str;
	return &str;
}

bool controller_serial::action_socket_query(yapsc::Message *p,
					    yapsc_domain_t domain) {
	if (p->type() != p->TARGET_SOCKET_QUERY) {
		return false;
	}
	if (targets.find(p->socketname()) == targets.end()) {
		p->set_port(0);
		p->set_domain(0);
		EXPIO_LOG_DBG("Quering %s: missing", p->socketname().c_str());
	} else {
		yapsc_domain_port_pair &addr = targets.at(p->socketname());
		const std::string *s = domain_address_get(addr.first);
		p->set_domain(addr.first);
		p->set_port(addr.second);
		p->set_address(*s);
		EXPIO_LOG_DBG("Quering %s: %d %s:%d", p->socketname().c_str(),
			      addr.first, s->c_str(), addr.second);
	}
	std::string *s = new std::string();
	if (!p->SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s, domain);
	delete (s);
	return true;
}

bool controller_serial::action_barrier(yapsc::Message *p,
				       yapsc_domain_t domain) {
	static int counter = 0;
	if (p->type() != p->BARRIER) {
		return false;
	}
	if (!counter) {
		counter = domains;
	}
	counter--;
	if (!counter) {
		EXPIO_LOG_DBG("All clients achieved the barrier");
		std::string *s = new std::string();
		p->set_domain(0);
		if (!p->SerializeToString(s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		if (before_simulation) {
			before_simulation();
			before_simulation = NULL;
		}
		for (int i = 1; i <= domains; i++) {
			send_string(s, i);
		}
		delete (s);
	}
	return true;
}

bool controller_serial::action_syscall(yapsc::Message *p,
				       yapsc_domain_t domain) {
	if (p->type() != p->SYSCALL) {
		return false;
	}

	yapsc::Syscall *sc = p->mutable_syscall();
	switch (sc->type()) {
	case sc->READ: {
		int fildes = sc->filedescriptor();
		sc->clear_filedescriptor();
		size_t nbytes = sc->offset();
		sc->clear_offset();
		void *buf = malloc(nbytes);
		size_t ret = read(fildes, buf, nbytes);
		sc->set_error(errno);
		sc->set_buffer(buf, ret);
		free(buf);
		break;
	}
	case sc->WRITE: {
		int fildes = sc->filedescriptor();
		sc->clear_filedescriptor();
		const void *buf = (void *)sc->buffer().c_str();
		size_t nbytes = sc->buffer().length();
		EXPIO_LOG_INFO("Write {%s}",sc->buffer().c_str());
		size_t ret = write(fildes, buf, nbytes);
		sc->clear_buffer();
		sc->set_error(errno);
		sc->set_offset(ret);
		break;
	}
	case sc->OPEN: {
		const char *pathname = sc->filepath().c_str();
		int oflags = sc->flags();
		sc->clear_flags();
		mode_t mode = sc->mode();
		sc->clear_mode();
		int ret = open(pathname, oflags, mode);
		sc->set_error(errno);
		sc->clear_filepath();
		sc->set_filedescriptor(ret);
		break;
	}
	case sc->CLOSE: {
		int fildes = sc->filedescriptor();
		sc->clear_filedescriptor();
		int ret = close(fildes);
		sc->set_error(errno);
		sc->set_mode(ret);
		break;
	}
	case sc->CREATE: {
		const char *pathname = sc->filepath().c_str();
		mode_t mode = sc->mode();
		sc->clear_mode();
		int ret = creat(pathname, mode);
		sc->set_error(errno);
		sc->clear_filepath();
		sc->set_filedescriptor(ret);
		break;
	}
	case sc->LSEEK: {
		int fildes = sc->filedescriptor();
		sc->clear_filedescriptor();
		off_t offset = sc->offset();
		sc->clear_offset();
		int whence = sc->mode();
		sc->clear_mode();
		off_t ret = lseek(fildes, offset, whence);
		sc->set_error(errno);
		sc->set_offset(ret);
		break;
	}
	case sc->ISATTY: {
		int fildes = sc->filedescriptor();
		sc->clear_filedescriptor();
		int ret = isatty(fildes);
		sc->set_error(errno);
		sc->set_mode(ret);
		break;
	}
	case sc->SBRK: {
		intptr_t increment = sc->offset();
		sc->clear_offset();
		static uint64_t heap_sbrk = 0;
		sc->set_error(0);
		sc->set_pointer(heap_sbrk);
		heap_sbrk += increment;
		break;
	}
	case sc->EXIT: {
		action_end(domain, sc->mode());

		break;
	}
	case sc->GETCWD: {
		size_t nbytes = sc->offset();
		sc->clear_offset();
		char *buf = (char *)malloc(nbytes);
		;
		char *buf2 = getcwd(buf, nbytes);
		sc->set_error(errno);
		if (buf2)
			sc->set_filepath(buf2);
		free(buf);
		break;
	}
	default: { EXPIO_LOG_CRIT("Unknown syscall"); }
	}

	std::string *s = new std::string();
	if (!p->SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s, domain);
	delete (s);
	return true;
}
