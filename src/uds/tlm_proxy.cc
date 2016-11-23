#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_proxy_uds.h"

#include <expio.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using yapsc::proxy_uds;
typedef yapsc::proxy_uds SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

proxy_uds::proxy_uds(sc_core::sc_module_name module_name, const char *ns)
    : yapsc::wrapper_uds(module_name, ns), tsocket("target") {
	tsocket.register_nb_transport_fw(this, &proxy_uds::nb_transport_fw);

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
}

/**
 * Default destructor.
 */
proxy_uds::~proxy_uds() {}

void proxy_uds::connect_server() {
	int fd = -1;
	struct sockaddr_un addr;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		EXPIO_LOG_CRIT("Socket open");
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(&addr.sun_path[1], ns, sizeof(addr.sun_path) - 2);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) !=
	    -1) {
		EXPIO_LOG_INFO("UDS Client connected to %s", ns);
		connfd = fd;
		return;
	}

	close(fd);
	fd = -1;
	EXPIO_LOG_DBG("Connect error to %s", ns);
}

tlm_sync_enum proxy_uds::nb_transport_fw(tlm_generic_payload &payload,
					 tlm_phase &phase,
					 sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Forward to UDS");
	send_payload(&payload);
	phase = END_REQ;
	return TLM_UPDATED;
}

void proxy_uds::proc_in() {
	while (true) {
		wait(1, sc_core::SC_NS);

		tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm_phase phase = BEGIN_RESP;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm_sync_enum resp =
		    tsocket->nb_transport_bw(std::ref(*payload), phase, delay);
		if (resp != TLM_COMPLETED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
