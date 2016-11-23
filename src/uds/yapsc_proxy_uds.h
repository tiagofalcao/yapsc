#ifndef TLM_PROXY_UDS_H_
#define TLM_PROXY_UDS_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_uds.h"
#include <tlm_utils/simple_target_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct proxy_uds : public yapsc::wrapper_uds {
      public:
	proxy_uds(sc_core::sc_module_name module_name, const char *ns);
	virtual ~proxy_uds();

	tlm_utils::simple_target_socket<proxy_uds> tsocket;
	tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &payload,
					   tlm::tlm_phase &phase,
					   sc_core::sc_time &delay_time);

	virtual void proc_in(void);

      private:
	void connect_server(void);
};
};
#endif
