#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_client_uds.h"
#include "yapsc_connection_uds.h"
#include "yapsc_controller_uds.h"
#include "yapsc_uds.h"
#include <expio.h>
#include <systemc.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using yapsc::client_uds;

client_uds::client_uds(const char *name, yapsc_domain_t domain)
    : yapsc::client_serial(name, domain) {
	sockfd = -1;
	connfd = -1;

	this->buffer = NULL;
	this->buffer_sz = 0;

	for (int i = 0; i < 20; i++) {
		connect_server();
		if (connfd < 0) {
			EXPIO_LOG_DBG("Failed to connect, sleep(5) to retry");
			sleep(5);
		} else {
			break;
		}
	}
	if (connfd < 0) {
		EXPIO_LOG_CRIT("Failed to connect");
	}
	EXPIO_LOG_DBG("Connected to controller");
	get_modules();
}

client_uds::~client_uds() {
	if (connfd >= 0) {
		if (!ended) {
			action_end(2);
		}
		close(connfd);
		connfd = -1;
	}
	if (sockfd >= 0) {
		close(sockfd);
		sockfd = -1;
	}
	if (buffer) {
		free(buffer);
		buffer_sz = 0;
		buffer = NULL;
	}
}

const char *client_uds::backend_name() { return "UDS"; }

void client_uds::bind(void) {
	for (auto &x : local_initiators) {
		yapsc_initiator_socket &init = x.second;
		yapsc_target_socket &target = targets.at(x.first);
		targets.erase(x.first);
		EXPIO_LOG_INFO("Binding: %s", x.first.c_str());
		init.bind(target);
	}
	yapsc::Message p;
	std::string s;
	yapsc::remote_uds *RW;
	int i = 10;
	for (auto &x : targets) {
		std::string wrapper = x.first + "_RW";
		char *ns;
		asprintf(&ns, "yapsc:%s:%d:%d", name, domain, ++i);
		RW = new yapsc::remote_uds(wrapper.c_str(), ns);
		yapsc_target_socket &target = x.second;
		RW->isocket.bind(target);

		EXPIO_LOG_DBG("Socket %s at %s", x.first.c_str(), ns);
		p.Clear();
		p.set_type(p.TARGET_SOCKET_REGISTRY);
		p.set_socketname(x.first);
		p.set_port(i);
		p.set_domain(domain_get());
		if (!p.SerializeToString(&s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		send_string(&s);
		free(ns);
	}
	yapsc::proxy_uds *PW;
	for (auto &x : initiators) {
		yapsc_domain_t d;
		int port = 0;
		const char *addr;
		while (!port) {
			p.Clear();
			p.set_type(p.TARGET_SOCKET_QUERY);
			p.set_socketname(x.first);
			if (!p.SerializeToString(&s)) {
				EXPIO_LOG_CRIT("Serialize error");
			}
			send_string(&s);
			std::string *str = recv_string(true);
			if (!p.ParseFromString(*str)) {
				EXPIO_LOG_CRIT("Parser error");
			}
			delete (str);
			port = p.port();
			if (port) {
				d = p.domain();
				break;
			}
			EXPIO_LOG_DBG("Failed to query, sleep(5) to retry");
			sleep(5);
		}
		std::string wrapper = x.first + "_PW";
		char *ns;
		asprintf(&ns, "yapsc:%s:%d:%d", name, d, port);
		EXPIO_LOG_DBG("Got socket %s at %s", x.first.c_str(), ns);
		PW = new yapsc::proxy_uds(wrapper.c_str(), ns);
		yapsc_initiator_socket &init = x.second;
		init.bind(PW->tsocket);
		free(ns);
	}
}

void client_uds::connect_server() {
	int fd = -1;
	struct sockaddr_un addr;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		EXPIO_LOG_CRIT("Socket open");
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 2, "yapsc:%s",
		 this->name);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
		close(fd);
		fd = -1;
		EXPIO_LOG_DBG("Connect error to %s", &addr.sun_path[1]);
		return;
	}
	connfd = fd;
	{
		yapsc::Message p;
		p.set_type(p.DOMAIN_REGISTRY);
		p.set_domain(domain_get());

		std::string *s = new std::string();
		if (!p.SerializeToString(s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		EXPIO_LOG_DBG("Registering %d to controller", p.domain());
		send_string(s);
		delete (s);
	}
	return;
}

void client_uds::send_string(std::string *str) {
	yapsc::send_string_uds(connfd, str);
}

std::string *client_uds::recv_string(bool blocking) {
	return yapsc::recv_string_uds(connfd, blocking);
}

yapsc::client *get_client_uds(int *argc, char ***argv) {

	const char *domain_env = getenv("YAPSC_DOMAIN");

	if (!domain_env) {
		return NULL;
	}

	char backend[16];
	char name[64];
	int32_t domain;
	sscanf(domain_env, "%[^:]:%[^:]:%" SCNd32 "", backend, name, &domain);

	if (strcasecmp(backend, "uds")) {
		return NULL;
	}

	if (domain) {
		return new client_uds(name, domain);
	} else {
		return new yapsc::controller_uds(name);
	}
}
