#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_client.h"
#include "yapsc_connection_mpi.h"
#include "yapsc_remote_mpi.h"

#include <expio.h>

using yapsc::remote_mpi;
typedef yapsc::remote_mpi SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

remote_mpi::remote_mpi(sc_core::sc_module_name module_name, MPI_Comm comm,
		       int tag)
    : yapsc::wrapper_mpi(module_name, comm, tag, MPI_ANY_SOURCE, MPI_ANY_TAG),
      isocket("initiator") {
	isocket.register_nb_transport_bw(this, &remote_mpi::nb_transport_bw);
}

/**
 * Default destructor.
 */
remote_mpi::~remote_mpi() {}

tlm::tlm_sync_enum
remote_mpi::nb_transport_bw(tlm::tlm_generic_payload &payload,
			    tlm::tlm_phase &phase,
			    sc_core::sc_time &delay_time) {
	EXPIO_LOG_DBG("Payload Backward to MPI");
	send_payload(&payload);
	phase = tlm::END_REQ;
	return tlm::TLM_COMPLETED;
}

void remote_mpi::wait_connection(void) {
	yapsc::Message p;
	std::string *s = NULL;

	EXPIO_LOG_DBG("MPI Server %d:%d waiting connection from %d:%d",
		      my_domain, my_tag, target_domain, target_tag);

	s = yapsc::recv_string_mpi(comm, target_domain, my_tag, true);
	if (!p.ParseFromString(*s)) {
		EXPIO_LOG_CRIT("Parser error");
	}
	delete (s);

	target_domain = p.domain();
	target_tag = p.port();

	EXPIO_LOG_INFO("MPI Server connection from %d:%d", target_domain,
		       target_tag);
}

void remote_mpi::proc_in() {
	this->wait_connection();
	while (true) {
		wait(1, sc_core::SC_NS);

		if (YAPSC_CLIENT && !YAPSC_CLIENT->can_continue()) {
			sc_core::sc_stop();
		}

		tlm::tlm_generic_payload *payload = recv_payload();
		if (!payload) {
			continue;
		}

		tlm::tlm_phase phase = tlm::BEGIN_REQ;
		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

		tlm::tlm_sync_enum resp =
		    isocket->nb_transport_fw(*payload, phase, delay);
		if (resp != tlm::TLM_UPDATED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
	}
}
