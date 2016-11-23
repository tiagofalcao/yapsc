#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_connection_mpi.h"
#include "yapsc_wrapper_mpi.h"

#include <expio.h>

using yapsc::wrapper_mpi;
typedef yapsc::wrapper_mpi SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

wrapper_mpi::wrapper_mpi(sc_core::sc_module_name module_name, MPI_Comm comm, int my_tag,
			 int domain, int tag)
    : yapsc::wrapper_serial(module_name) {

	this->comm = comm;
	this->target_domain = domain;
	this->target_tag = tag;
	this->my_tag = my_tag;
	MPI_Comm_rank(comm, &this->my_domain);
}

/**
 * Default destructor.
 */
wrapper_mpi::~wrapper_mpi() {
}

void wrapper_mpi::send_string(const std::string *str) {
	yapsc::send_string_mpi(comm, target_domain, target_tag, str);
}

std::string *wrapper_mpi::recv_string() {
	return yapsc::recv_string_mpi(comm, target_domain, my_tag, false);
}
