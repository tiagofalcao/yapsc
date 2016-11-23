#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_proxy_tcp.h"
#include "yapsc_client.h"

#include <expio.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/tcp.h>
#include <pthread.h>

using yapsc::proxy_tcp;
typedef yapsc::proxy_tcp SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

proxy_tcp::proxy_tcp(sc_core::sc_module_name module_name, const char *addr,
		     unsigned int port)
    : yapsc::wrapper_tcp(module_name), tsocket("target") {
	tsocket.register_nb_transport_fw(this, &proxy_tcp::nb_transport_fw);

	for (int i = 0; i < 20; i++) {
		connect_server(addr, port);
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
}

/**
 * Default destructor.
 */
proxy_tcp::~proxy_tcp() {}

void proxy_tcp::connect_server(const char *addr, unsigned int port) {
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
	return;
}

tlm::tlm_sync_enum proxy_tcp::nb_transport_fw(tlm::tlm_generic_payload &payload,
					 tlm::tlm_phase &phase,
					 sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Forward to TCP");
	send_payload(&payload);
	phase = END_REQ;
	return TLM_UPDATED;
}

void proxy_tcp::proc_in() {
	while (true) {
		wait(1, sc_core::SC_NS);

		if (YAPSC_CLIENT && !YAPSC_CLIENT->can_continue()) {
			sc_core::sc_stop();
		}

		tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm::tlm_phase phase = BEGIN_RESP;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm::tlm_sync_enum resp =
		    tsocket->nb_transport_bw(std::ref(*payload), phase, delay);
		if (resp != TLM_COMPLETED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
