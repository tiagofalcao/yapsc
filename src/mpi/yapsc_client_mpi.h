#ifndef YAPSC_CLIENT_MPI_H
#define YAPSC_CLIENT_MPI_H

#include "yapsc_client_serial.h"
#include "yapsc_core.h"
#include <stdint.h>
#include <mpi.h>

namespace yapsc {

class client_mpi : public yapsc::client_serial {
      public:
	client_mpi(const char *name, MPI_Comm comm, yapsc_domain_t domain);
	virtual ~client_mpi();

	virtual const char *backend_name();

      private:
	virtual void bind(void);
	virtual void send_string(std::string *str);
	virtual std::string *recv_string(bool blocking);

	MPI_Comm comm;
};
};

extern "C" {
yapsc::client *get_client_mpi(int *argc, char ***argv);
}

#endif
