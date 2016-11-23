#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_client_tcp.h"
#include "yapsc_controller_tcp.h"
#include "yapsc_connection_tcp.h"
#include "yapsc_tcp.h"
#include <expio.h>
#include <systemc.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/tcp.h>

using yapsc::client_tcp;

client_tcp::client_tcp(const char *name, yapsc_domain_t domain)
    : yapsc::client_serial(name, domain) {
	sockfd = -1;
	connfd = -1;

	this->buffer = NULL;
	this->buffer_sz = 0;

	if (!yapsc::tcp_zeroconf_discover(name, address, &port)) {
		EXPIO_LOG_CRIT("yapsc_lp_tcp_zeroconf_discover");
	}
	for (int i = 0; i < 20; i++) {
		connect_server(address, port);
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

client_tcp::~client_tcp() {
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

const char *client_tcp::backend_name() { return "TCP"; }

void client_tcp::bind(void) {
	for (auto &x : local_initiators) {
		yapsc_initiator_socket &init = x.second;
		yapsc_target_socket &target = targets.at(x.first);
		targets.erase(x.first);
		EXPIO_LOG_INFO("Binding: %s", x.first.c_str());
		init.bind(target);
	}
	yapsc::Message p;
	std::string s;
	yapsc::remote_tcp *RW;
	for (auto &x : targets) {
		std::string wrapper = x.first + "_RW";
		RW = new yapsc::remote_tcp(wrapper.c_str());
		yapsc_target_socket &target = x.second;
		RW->isocket.bind(target);

		int port = RW->port_get();
		p.Clear();
		p.set_type(p.TARGET_SOCKET_REGISTRY);
		p.set_socketname(x.first);
		p.set_port(RW->port_get());
		p.set_domain(domain_get());
		if (!p.SerializeToString(&s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		send_string(&s);
	}
	yapsc::proxy_tcp *PW;
	for (auto &x : initiators) {
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
				addr = p.address().c_str();
				break;
			}
			EXPIO_LOG_DBG("Failed to query, sleep(5) to retry");
			sleep(5);
		}
		std::string wrapper = x.first + "_PW";
		PW = new yapsc::proxy_tcp(wrapper.c_str(), addr, port);
		yapsc_initiator_socket &init = x.second;
		init.bind(PW->tsocket);
	}
}

void client_tcp::connect_server(const char *addr, unsigned int port) {
	int fd = -1;
	struct addrinfo *servinfo;

	{
		char sport[7];
		struct addrinfo hints;
		int rv;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;
		hints.ai_family = AF_UNSPEC; /* Allows IPv4 or IPv6 */
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_NUMERICSERV;

		sprintf(sport, "%d", port);

		if ((rv = getaddrinfo(addr, sport, &hints, &servinfo)) != 0) {
			EXPIO_LOG_CRIT("Getaddrinfo: %s\n", gai_strerror(rv));
		}
	}

	{
		struct addrinfo *p;
		for (p = servinfo; p != NULL; p = p->ai_next) {
			fd = socket(p->ai_family, p->ai_socktype,
				    p->ai_protocol);
			if (fd == -1) {
				EXPIO_LOG_DBG("Socket return error");
				continue;
			}

			if (connect(fd, p->ai_addr, p->ai_addrlen) != -1) {
				EXPIO_LOG_INFO("TCP Client connected to %s:%d",
					       addr, port);
				break;
			}

			close(fd);
			fd = -1;
			EXPIO_LOG_DBG("Connect error to %s:%d", addr, port);
		}

		if (p == NULL || fd == -1) {
			EXPIO_LOG_DBG("Failed to connect");
			return;
		}
	}

	freeaddrinfo(servinfo);

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

void client_tcp::send_string(std::string *str) {
	yapsc::send_string_tcp(connfd, str);
}

std::string *client_tcp::recv_string(bool blocking) {
	return yapsc::recv_string_tcp(connfd, blocking);
}

yapsc::client *get_client_tcp(int *argc, char ***argv) {

	const char *domain_env = getenv("YAPSC_DOMAIN");

	if (!domain_env) {
		return NULL;
	}

	char backend[16];
	char name[64];
	int32_t domain;
	sscanf(domain_env, "%[^:]:%[^:]:%" SCNd32 "", backend, name, &domain);

	if (strcasecmp(backend, "tcp")) {
		return NULL;
	}

	if (domain) {
		return new client_tcp(name, domain);
	} else {
		return new yapsc::controller_tcp(name);
	}
}
