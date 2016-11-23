#ifndef TLM_REMOTE_TCP_H_
#define TLM_REMOTE_TCP_H_

#include <thread>

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_tcp.h"
#include <tlm_utils/simple_initiator_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct remote_tcp : public yapsc::wrapper_tcp {
      public:
	remote_tcp(sc_core::sc_module_name module_name);
	remote_tcp(sc_core::sc_module_name module_name, int port);
	virtual ~remote_tcp();

	tlm_utils::simple_initiator_socket<remote_tcp> isocket;
	tlm::tlm_sync_enum
	nb_transport_bw(tlm::tlm_generic_payload &payload,
			tlm::tlm_phase &phase, sc_core::sc_time &delay_time);


	virtual void proc_in(void);

	int port_get(void) const;

      private:
	std::thread *conn_thread;

	int port;
	bool start_server(int port);
	void wait_connection(void);
};
};
#endif
