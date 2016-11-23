#ifndef TLM_WRAPPER_MPI_H_
#define TLM_WRAPPER_MPI_H_

//////////////////////////////////////////////////////////////////////////////

#include <mpi.h>
#include "yapsc_wrapper_serial.h"

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct wrapper_mpi : public yapsc::wrapper_serial {
      public:
	wrapper_mpi(sc_core::sc_module_name module_name, MPI_Comm comm, int my_tag, int domain,
		    int tag);
	virtual ~wrapper_mpi();

	virtual void send_string(const std::string *str);
	virtual std::string *recv_string();

      protected:
	MPI_Comm comm;
	int target_domain;
	int target_tag;
	int my_domain;
	int my_tag;

      private:
	char *send_message;
	MPI_Request send_request;

	char *recv_message;
	unsigned int recv_message_sz;
};
};
#endif
