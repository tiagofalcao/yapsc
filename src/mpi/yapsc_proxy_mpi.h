#ifndef TLM_PROXY_MPI_H_
#define TLM_PROXY_MPI_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_mpi.h"
#include <tlm_utils/simple_target_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct proxy_mpi : public yapsc::wrapper_mpi {
      public:
	proxy_mpi(sc_core::sc_module_name module_name, MPI_Comm comm,
		  int my_tag, int domain, int tag);
	virtual ~proxy_mpi();

	tlm_utils::simple_target_socket<proxy_mpi> tsocket;
	tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &payload,
					   tlm::tlm_phase &phase,
					   sc_core::sc_time &delay_time);

	virtual void proc_in(void);
};
};
#endif
