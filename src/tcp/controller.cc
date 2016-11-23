#include "../../build/src/serialize/yapsc_message.pb.h"
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

#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
#define BACKLOG 10

using yapsc::controller_tcp;

controller_tcp::controller_tcp(const char *name)
    : yapsc::controller_serial(name) {
	sockfd = -1;
	port = 0;

	for (int p = 2048; (p < 65000) && (!start_server(p)); p++) {
		EXPIO_LOG_DBG("Skipping server on port %d", p);
	}

	if (!port) {
		EXPIO_LOG_CRIT("TCP port bind error from 2048 to 65000.");
	}

	if (!yapsc::tcp_zeroconf_publish_init(name, port)) {
		EXPIO_LOG_CRIT("yapsc_controller_tcp_zeroconf_setup");
	}

	connfd = (int *)malloc(sizeof(int) * domains);
	domain_address =
	    (std::string **)malloc(sizeof(std::string *) * domains);
	for (int i = 0; i < domains; i++) {
		connfd[i] = -1;
	}
}

controller_tcp::~controller_tcp() {
	yapsc::tcp_zeroconf_publish_shutdown();
	if (connfd) {
		for (int i = 0; i < domains; i++) {
			if (connfd[i] >= 0) {
				close(connfd[i]);
			}
			if (domain_address[i]) {
				delete (domain_address[i]);
			}
		}
		free(connfd);
		connfd = NULL;
	}
	if (sockfd >= 0) {
		close(sockfd);
		sockfd = -1;
	}
}

const char *controller_tcp::backend_name() { return "TCP"; }

const std::string *controller_tcp::domain_address_get(yapsc_domain_t domain) {
	return domain_address[domain - 1];
}

void controller_tcp::before_start(void) {
	for (int i = 0; i < domains; i++) {
		connection_get();
	}
}

bool controller_tcp::connection_get() {
	int c = -1;
	char addrStr[ADDRSTRLEN];

	while (c < 0) {
		struct sockaddr_storage claddr;
		socklen_t addrlen;

		addrlen = sizeof(struct sockaddr_storage);
		EXPIO_LOG_DBG("Waiting connection");
		c = accept(sockfd, (struct sockaddr *)&claddr, &addrlen);
		if (c == -1) {
			EXPIO_LOG_WARN("Connection acceptation error");
			continue;
		}

		switch (claddr.ss_family) {
		case AF_INET:
			inet_ntop(AF_INET,
				  &(((struct sockaddr_in *)&claddr)->sin_addr),
				  addrStr, ADDRSTRLEN);
			break;

		case AF_INET6:
			inet_ntop(
			    AF_INET6,
			    &(((struct sockaddr_in6 *)&claddr)->sin6_addr),
			    addrStr, ADDRSTRLEN);
			break;
		}
		EXPIO_LOG_INFO("TCP connection from %s on %d", addrStr, port);
	}

	std::string *s = yapsc::recv_string_tcp(c, true);
	if (!s) {
		EXPIO_LOG_CRIT("Empty message from client");
	}
	yapsc::Message *p = new yapsc::Message();
	if (!p->ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);

	if (p->type() != p->DOMAIN_REGISTRY) {
		EXPIO_LOG_CRIT("Invalid message in domain registry stage");
	} else if (p->domain() > domains || p->domain() <= 0) {
		EXPIO_LOG_CRIT("Invalid domain %d in domain registry stage",
			       p->domain());
	}
	EXPIO_LOG_DBG("Domain %d connected from %s", p->domain(), addrStr);
	connfd[p->domain() - 1] = c;
	domain_address[p->domain() - 1] = new std::string(addrStr);

	for (auto &x : modules) {
		if (x.second == p->domain()) {
			p->add_names(x.first);
		}
	}

	s = new std::string();
	if (!p->SerializeToString(s)) {
		EXPIO_LOG_CRIT("Serialize error");
	}
	EXPIO_LOG_INFO("Domain %d registered", p->domain());
	yapsc::send_string_tcp(c, s);
	delete (s);
	return true;
}

bool controller_tcp::start_server(int p) {
	int fd = 0;
	struct addrinfo *servinfo;

	{
		char out_port[7];
		struct addrinfo hints;
		int rv;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_UNSPEC; /* Allows IPv4 or IPv6 */
		hints.ai_flags =
		    AI_PASSIVE | AI_NUMERICSERV; /* Wildcard IP address;
						    service name is
						    numeric */

		sprintf(out_port, "%d", p);
		if ((rv = getaddrinfo(NULL, out_port, &hints, &servinfo)) !=
		    0) {
			EXPIO_LOG_CRIT("Getaddrinfo: %s\n", gai_strerror(rv));
			exit(1);
		}
	}

	{
		struct addrinfo *p;
		int yes = 1;

		for (p = servinfo; p != NULL; p = p->ai_next) {
			fd = socket(p->ai_family, p->ai_socktype,
				    p->ai_protocol);
			if (fd == -1) {
				continue;
			}

			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				       sizeof(int)) == -1) {
				EXPIO_LOG_CRIT("SO_REUSEADDR setsockopt");
			}

			if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &yes,
				       sizeof(int)) == -1) {
				EXPIO_LOG_WARN("TCP_NODELAY setsockopt");
			}

			if (bind(fd, p->ai_addr, p->ai_addrlen) == -1) {
				close(fd);
				continue;
			}

			break;
		}

		if (p == NULL) {
			EXPIO_LOG_DBG("Failed to bind socket to port");
			return false;
		}
	}

	if (listen(fd, BACKLOG) == -1) {
		EXPIO_LOG_DBG("Backlog");
		return false;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (fd <= 0)
		return false;

	this->sockfd = fd;
	this->port = p;
	EXPIO_LOG_INFO("TCP Server on port %d", this->port);
	return true;
}

void controller_tcp::send_string(std::string *str, yapsc_domain_t domain) {
	yapsc::send_string_tcp(connfd[domain - 1], str);
}

std::string *controller_tcp::recv_string(yapsc_domain_t domain, bool blocking) {
	return yapsc::recv_string_tcp(connfd[domain - 1], blocking);
}
