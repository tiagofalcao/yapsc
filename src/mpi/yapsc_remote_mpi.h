#ifndef TLM_REMOTE_MPI_H_
#define TLM_REMOTE_MPI_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_mpi.h"
#include <tlm_utils/simple_initiator_socket.h>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct remote_mpi : public yapsc::wrapper_mpi {
      public:
	remote_mpi(sc_core::sc_module_name module_name, MPI_Comm comm, int tag);
	virtual ~remote_mpi();

	tlm_utils::simple_initiator_socket<remote_mpi> isocket;
	tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &payload,
					   tlm::tlm_phase &phase,
					   sc_core::sc_time &delay_time);

	virtual void proc_in(void);
	void wait_connection(void);

      private:
};
};
#endif
