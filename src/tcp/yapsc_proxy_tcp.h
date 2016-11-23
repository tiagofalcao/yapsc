#ifndef TLM_PROXY_TCP_H_
#define TLM_PROXY_TCP_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_tcp.h"
#include <tlm_utils/simple_target_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct proxy_tcp : public yapsc::wrapper_tcp {
      public:
	proxy_tcp(sc_core::sc_module_name module_name, const char *addr,
		  unsigned int port);
	virtual ~proxy_tcp();

	tlm_utils::simple_target_socket<proxy_tcp> tsocket;
	tlm::tlm_sync_enum
	nb_transport_fw(tlm::tlm_generic_payload &payload,
			tlm::tlm_phase &phase, sc_core::sc_time &delay_time);

	virtual void proc_in(void);

      private:
	void connect_server(const char *addr, unsigned int port);
};
};
#endif
