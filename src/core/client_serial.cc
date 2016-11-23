#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_client_serial.h"
#include <expio.h>
#include <systemc.h>

using yapsc::client_serial;

client_serial::client_serial(const char *name, yapsc_domain_t domain)
    : yapsc::client(name, domain) {
	ended = false;
}

client_serial::~client_serial() {
	modules.clear();
	local_initiators.clear();
	initiators.clear();
	targets.clear();
}

void client_serial::get_modules(void) {
	yapsc::Message p;
	std::string *s = recv_string(true);
	if (!s) {
		EXPIO_LOG_CRIT("Unexpected empty string");
	}
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);

	for (int i = 0; i < p.names_size(); i++) {
		EXPIO_LOG_DBG("Domain's module: %s", p.names(i).c_str());
		modules.insert(
		    yapsc_module_domain_pair(p.names(i), domain_get()));
	}
}

bool client_serial::module_domain(const std::string module_name) {
	return modules.find(module_name) != modules.end();
}

void client_serial::initiator_socket(yapsc_initiator_socket &init,
				     const std::string module_name,
				     const std::string socket_name) {
	std::string name = module_name + "_" + socket_name;
	if (module_domain(module_name)) {
		local_initiators.insert(yapsc_initiator_pair(name, init));
	} else {
		initiators.insert(yapsc_initiator_pair(name, init));
	}
}

void client_serial::target_socket(const std::string module_name,
				  const std::string socket_name,
				  yapsc_target_socket &target) {
	std::string name = module_name + "_" + socket_name;
	targets.insert(yapsc_target_pair(name, target));
}

void client_serial::start(void) {
	bind();
	action_barrier();
	try {
		EXPIO_LOG_INFO("Starting kernal");
		sc_start();
		EXPIO_LOG_INFO("Exited kernal");
	} catch (std::exception &e) {
		EXPIO_LOG_WARN("Caught exception %s", e.what());
	} catch (...) {
		EXPIO_LOG_ERR("Caught exception during simulation.");
	}

	if (not sc_end_of_simulation_invoked()) {
		EXPIO_LOG_ERR("Simulation stopped without explicit sc_stop()");
		sc_stop();
		action_end(1);
	} else {
		action_end(0);
	}
	EXPIO_LOG_INFO("The end.");
}

bool client_serial::can_continue() {
	yapsc::Message p;
	std::string *s;
	s = recv_string(false);
	if (!s) {
		return true;
	}
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.END) {
		EXPIO_LOG_WARN("Unexpected message from Controller");
	}
	return false;
}

void client_serial::action_barrier() {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.BARRIER);
	p.set_domain(domain_get());
	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	EXPIO_LOG_DBG("Barrier request");
	send_string(s);
	delete (s);
	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	if (p.type() != p.BARRIER) {
		EXPIO_LOG_INFO("Unexpected barrier release");
	}
	delete (s);
	EXPIO_LOG_INFO("Barrier released");
}

void client_serial::action_end(int status) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.END);
	p.set_domain(domain_get());
	p.set_status(status);
	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	EXPIO_LOG_DBG("End notify");
	send_string(s);
	ended = true;
	delete (s);
}

size_t client_serial::syscall_read(int fildes, void *buf, size_t nbytes) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->READ);
	sc->set_filedescriptor(fildes);
	sc->set_offset(nbytes);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	memcpy(buf, sc->buffer().c_str(), sc->buffer().length());
	errno = sc->error();
	return sc->buffer().length();
}

size_t client_serial::syscall_write(int fildes, const void *buf,
				    size_t nbytes) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->WRITE);
	sc->set_filedescriptor(fildes);
	sc->set_buffer(buf, nbytes);

	EXPIO_LOG_INFO("WRITE [%s] %d", sc->buffer().c_str(), sc->buffer().length());

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->offset();
}

int client_serial::syscall_open(const char *pathname, int oflags, mode_t mode) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->OPEN);
	sc->set_filepath(pathname, strlen(pathname));
	sc->set_flags(oflags);
	sc->set_mode(mode);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->filedescriptor();
}

int client_serial::syscall_close(int fildes) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->CLOSE);
	sc->set_filedescriptor(fildes);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->mode();
}

int client_serial::syscall_creat(const char *pathname, mode_t mode) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->CREATE);
	sc->set_filepath(pathname, strlen(pathname));
	sc->set_mode(mode);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->filedescriptor();
}

off_t client_serial::syscall_lseek(int fildes, off_t offset, int whence) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->LSEEK);
	sc->set_filedescriptor(fildes);
	sc->set_offset(offset);
	sc->set_mode(whence);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->offset();
}

int client_serial::syscall_isatty(int fildes) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->ISATTY);
	sc->set_filedescriptor(fildes);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return sc->mode();
}

void *client_serial::syscall_sbrk(intptr_t increment) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->SBRK);
	sc->set_offset(increment);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	return (void *)sc->pointer();
}

void client_serial::syscall_exit(int status) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->EXIT);
	sc->set_mode(status);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}
}

char *client_serial::syscall_getcwd(char *buf, size_t nbytes) {
	yapsc::Message p;
	std::string *s = new std::string();
	p.set_type(p.SYSCALL);
	p.set_domain(domain_get());

	yapsc::Syscall *sc = p.mutable_syscall();
	sc->set_type(sc->GETCWD);
	sc->set_offset(nbytes);

	if (!p.SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	send_string(s);
	delete (s);

	s = recv_string(true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);
	if (p.type() != p.SYSCALL) {
		EXPIO_LOG_INFO("Unexpected syscall return");
	}

	sc = p.mutable_syscall();
	errno = sc->error();
	if (sc->filepath().length()) {
		memcpy(buf, sc->filepath().c_str(), sc->filepath().length() + 1);
		return buf;
	}
	return NULL;
}
