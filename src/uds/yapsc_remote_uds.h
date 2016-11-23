#ifndef TLM_REMOTE_UDS_H_
#define TLM_REMOTE_UDS_H_

#include <thread>

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_uds.h"
#include <tlm_utils/simple_initiator_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct remote_uds : public yapsc::wrapper_uds {
      public:
	remote_uds(sc_core::sc_module_name module_name, const char *ns);
	virtual ~remote_uds();

	tlm_utils::simple_initiator_socket<remote_uds> isocket;
	tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &payload,
					   tlm::tlm_phase &phase,
					   sc_core::sc_time &delay_time);

	virtual void proc_in(void);

      private:
	std::thread *conn_thread;

	bool start_server(void);
	void wait_connection(void);
};
};
#endif
