#ifndef YAPSC_CONTROLLER_MPI_H
#define YAPSC_CONTROLLER_MPI_H

#include "yapsc_controller_serial.h"
#include "yapsc_core.h"
#include <stdint.h>
#include <mpi.h>

namespace yapsc {

class controller_mpi : public yapsc::controller_serial {
      public:
	controller_mpi(const char *name, MPI_Comm comm);
	virtual ~controller_mpi();

	virtual const char *backend_name();

      private:
	virtual void before_start(void);
	virtual void send_string(std::string *str, yapsc_domain_t domain);
	virtual std::string *recv_string(yapsc_domain_t domain, bool blocking);

	MPI_Comm comm;
};
};

#endif
