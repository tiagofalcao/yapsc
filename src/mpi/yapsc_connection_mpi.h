#ifndef _YAPSC_CONNECTION_MPI_H
#define _YAPSC_CONNECTION_MPI_H

#include <mpi.h>
#include <stdint.h>
#include <string>

namespace yapsc {
void send_string_mpi(MPI_Comm comm, int domain, int tag,
		     const std::string *str);
std::string *recv_string_mpi(MPI_Comm comm, int domain, int tag, bool blocking);
}

#endif
