#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_remote_uds.h"

#include <expio.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
#define BACKLOG 10

using yapsc::remote_uds;
typedef yapsc::remote_uds SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

remote_uds::remote_uds(sc_core::sc_module_name module_name, const char *ns)
    : yapsc::wrapper_uds(module_name, ns), isocket("initiator") {
	isocket.register_nb_transport_bw(this, &remote_uds::nb_transport_bw);

	if (!start_server()) {
		EXPIO_LOG_CRIT("UDS server error");
	}
}

/**
 * Default destructor.
 */
remote_uds::~remote_uds() {
	if (conn_thread) {
		if (conn_thread->joinable())
			conn_thread->join();
		delete (conn_thread);
		conn_thread = NULL;
	}
	unlink(ns);
}

bool remote_uds::start_server() {
	int fd = -1;
	struct sockaddr_un addr;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		EXPIO_LOG_CRIT("Socket open");
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(&addr.sun_path[1], ns, sizeof(addr.sun_path) - 2);

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
	EXPIO_LOG_INFO("UDS Server on abstract namespace %s", ns);
	this->conn_thread = new std::thread(&remote_uds::wait_connection, this);
	return true;
}

void remote_uds::wait_connection(void) {
	EXPIO_LOG_INFO("UDS Server %s waiting connection", ns);

	while (connfd <= 0) {
		connfd = accept(sockfd, NULL, NULL);
		if (connfd != -1) {
			EXPIO_LOG_INFO("UDS connection at %s", ns);
			break;
		}
		EXPIO_LOG_ERR("Connection acceptation error");
	}
}

tlm_sync_enum remote_uds::nb_transport_bw(tlm_generic_payload &payload,
					  tlm_phase &phase,
					  sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Backward to UDS");
	send_payload(&payload);
	phase = END_REQ;
	return TLM_COMPLETED;
}

void remote_uds::proc_in() {
	//if (conn_thread) {
	//	if (conn_thread->joinable())
	//		conn_thread->join();
	//	delete (conn_thread);
	//	conn_thread = NULL;
	//}
	while (true) {
		wait(1, sc_core::SC_NS);

		tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm_phase phase = BEGIN_REQ;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm_sync_enum resp =
		    isocket->nb_transport_fw(*payload, phase, delay);
		if (resp != TLM_UPDATED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
