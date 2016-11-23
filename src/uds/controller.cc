#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_controller_uds.h"
#include "yapsc_connection_uds.h"
#include "yapsc_uds.h"
#include <expio.h>
#include <systemc.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BACKLOG 10

using yapsc::controller_uds;

controller_uds::controller_uds(const char *name)
    : yapsc::controller_serial(name) {
	sockfd = -1;

	if (!start_server()) {
		EXPIO_LOG_CRIT("UDS server error");
	}

	connfd = (int *)malloc(sizeof(int) * domains);
	for (int i = 0; i < domains; i++) {
		connfd[i] = -1;
	}
}

controller_uds::~controller_uds() {
	if (connfd) {
		for (int i = 0; i < domains; i++) {
			if (connfd[i] >= 0) {
				close(connfd[i]);
			}
		}
		free(connfd);
		connfd = NULL;
	}
	if (sockfd >= 0) {
		close(sockfd);
		sockfd = -1;
	}
	{
		char *addr;
		asprintf(&addr, "\0yapsc:%s", name);
		unlink(addr);
		free(addr);
	}
}

const char *controller_uds::backend_name() { return "UDS"; }

void controller_uds::before_start(void) {
	for (int i = 0; i < domains; i++) {
		connection_get();
	}
}

bool controller_uds::connection_get() {
	int c = -1;

	while (c < 0) {
		c = accept(sockfd, NULL, NULL);
		if (c != -1) {
			EXPIO_LOG_INFO("UDS connection at %s", name);
			break;
		}
		EXPIO_LOG_ERR("Connection acceptation error");
	}

	std::string *s = yapsc::recv_string_uds(c, true);
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
	EXPIO_LOG_DBG("Domain %d connected", p->domain());
	connfd[p->domain() - 1] = c;

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
	yapsc::send_string_uds(c, s);
	delete (s);
	return true;
}

bool controller_uds::start_server() {
	int fd = -1;
	struct sockaddr_un addr;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		EXPIO_LOG_CRIT("Socket open");
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 2, "yapsc:%s", name);
	unlink(addr.sun_path);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
	    -1) {
		close(fd);
		EXPIO_LOG_CRIT("Socket bind");
	}

	if (listen(fd, BACKLOG) == -1) {
		EXPIO_LOG_DBG("Backlog");
		return false;
	}

	if (fd <= 0)
		return false;

	this->sockfd = fd;
	EXPIO_LOG_INFO("UDS Server on abstract namespace %s", &addr.sun_path[1]);
	return true;
}

void controller_uds::send_string(std::string *str, yapsc_domain_t domain) {
	yapsc::send_string_uds(connfd[domain - 1], str);
}

std::string *controller_uds::recv_string(yapsc_domain_t domain, bool blocking) {
	return yapsc::recv_string_uds(connfd[domain - 1], blocking);
}
