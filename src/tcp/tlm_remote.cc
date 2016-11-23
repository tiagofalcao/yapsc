#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_remote_tcp.h"
#include "yapsc_client.h"

#include <expio.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/tcp.h>
#include <thread>

#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
#define BACKLOG 10

using yapsc::remote_tcp;
typedef yapsc::remote_tcp SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

remote_tcp::remote_tcp(sc_core::sc_module_name module_name, int p)
    : yapsc::wrapper_tcp(module_name), isocket("initiator") {
	isocket.register_nb_transport_bw(this, &remote_tcp::nb_transport_bw);

	if (p) {
		if (!start_server(p)) {
			EXPIO_LOG_ERR("TCP port bind error");
		}
	} else {

		for (int p = 2048; (p < 65000) && (!start_server(p)); p++) {
			EXPIO_LOG_DBG("Skipping server on port %d", p);
		}
		if (!port) {
			EXPIO_LOG_CRIT(
			    "TCP port bind error from 2048 to 65000.");
		}
	}
}

remote_tcp::remote_tcp(sc_core::sc_module_name module_name)
    : yapsc::remote_tcp(module_name, 0) {}

/**
 * Default destructor.
 */
remote_tcp::~remote_tcp() {
	if (conn_thread) {
		if (conn_thread->joinable())
			conn_thread->join();
		delete (conn_thread);
		conn_thread = NULL;
	}
}

bool remote_tcp::start_server(int p) {
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
		    AI_PASSIVE |
		    AI_NUMERICSERV; /* Wildcard IP address; service name is
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
	this->conn_thread = new std::thread(&remote_tcp::wait_connection, this);
	return true;
}

int remote_tcp::port_get(void) const { return this->port; }

void remote_tcp::wait_connection(void) {
	EXPIO_LOG_INFO("TCP Server on port %d waiting connection", this->port);

	while (connfd <= 0) {
		struct sockaddr_storage claddr;
		socklen_t addrlen;
		char addrStr[ADDRSTRLEN];

		addrlen = sizeof(struct sockaddr_storage);
		connfd = accept(sockfd, (struct sockaddr *)&claddr, &addrlen);
		if (connfd == -1) {
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
}

tlm::tlm_sync_enum remote_tcp::nb_transport_bw(tlm_generic_payload &payload,
					  tlm::tlm_phase &phase,
					  sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Backward to TCP");
	send_payload(&payload);
	phase = END_REQ;
	return TLM_COMPLETED;
}

void remote_tcp::proc_in() {
	//if (conn_thread) {
	//	if (conn_thread->joinable())
	//		conn_thread->join();
	//	delete (conn_thread);
	//	conn_thread = NULL;
	//}
	while (true) {
		wait(1, sc_core::SC_NS);

		if (YAPSC_CLIENT && !YAPSC_CLIENT->can_continue()) {
			sc_core::sc_stop();
		}

		tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm::tlm_phase phase = BEGIN_REQ;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm::tlm_sync_enum resp =
		    isocket->nb_transport_fw(*payload, phase, delay);
		if (resp != TLM_UPDATED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
