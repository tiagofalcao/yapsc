#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_client.h"
#include "yapsc_payload_extension.h"
#include "yapsc_proxy_mpi.h"

#include <expio.h>

#include <mpi.h>

using yapsc::proxy_mpi;
typedef yapsc::proxy_mpi SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

proxy_mpi::proxy_mpi(sc_core::sc_module_name module_name, MPI_Comm comm, int my_tag,
		     int domain, int tag)
    : yapsc::wrapper_mpi(module_name, comm, my_tag, domain, tag), tsocket("target") {
	tsocket.register_nb_transport_fw(this, &proxy_mpi::nb_transport_fw);

	EXPIO_LOG_DBG("MPI Client connecting to %d:%d", target_domain, target_tag);
	{
		yapsc::Message p;
		p.set_type(p.TARGET_SOCKET_REGISTRY);
		p.set_port(my_tag);
		p.set_domain(my_domain);
		std::string *s = new std::string();
		if (!p.SerializeToString(s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		send_string(s);
		delete (s);

		EXPIO_LOG_INFO("MPI Client connected to %d:%d", target_domain, target_tag);
	}
}

/**
 * Default destructor.
 */
proxy_mpi::~proxy_mpi() {}

tlm::tlm_sync_enum proxy_mpi::nb_transport_fw(tlm::tlm_generic_payload &payload,
					      tlm::tlm_phase &phase,
					      sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Forward to MPI");
	send_payload(&payload);
	phase = tlm::END_REQ;
	return tlm::TLM_UPDATED;
}

void proxy_mpi::proc_in() {
	while (true) {
		wait(1, sc_core::SC_NS);

		if (YAPSC_CLIENT && !YAPSC_CLIENT->can_continue()) {
			sc_core::sc_stop();
		}

		tlm::tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm::tlm_phase phase = tlm::BEGIN_RESP;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm_sync_enum resp =
		    tsocket->nb_transport_bw(std::ref(*payload), phase, delay);
		if (resp != tlm::TLM_COMPLETED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
