#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_controller_mpi.h"
#include "yapsc_mpi.h"
#include "yapsc_connection_mpi.h"
#include <expio.h>
#include <systemc.h>

using yapsc::controller_mpi;

controller_mpi::controller_mpi(const char *name, MPI_Comm comm)
    : yapsc::controller_serial(name) {

	int world_size;
	MPI_Comm_size(comm, &world_size);
	world_size--;
	if (world_size < domains) {
		EXPIO_LOG_CRIT("Required at least %d processes", world_size);
	} else if (world_size > domains) {
		domains = world_size;
		EXPIO_LOG_DBG("Required at least %d processes", world_size);
	}

	this->comm = comm;
}

controller_mpi::~controller_mpi() {
	MPI_Barrier(comm);
	EXPIO_LOG_DBG("Controller Finalize");
	MPI_Finalize();
}

const char *controller_mpi::backend_name() { return "MPI"; }

void controller_mpi::before_start(void) {
	std::string *s;
	yapsc::Message p;
	for (int i = 1; i <= domains; i++) {
		p.Clear();

		EXPIO_LOG_DBG("Waiting domain %d", i);
		s = recv_string(i, true);
		if (!p.ParseFromString(*s)) {
			EXPIO_LOG_CRIT("Parser error");
		}
		delete (s);

		if (p.type() != p.DOMAIN_REGISTRY) {
			EXPIO_LOG_CRIT("Invalid message in domain registry stage");
		} else if (p.domain() > domains || p.domain() <= 0) {
			EXPIO_LOG_CRIT("Invalid domain %d in domain registry stage",
					p.domain());
		}
		EXPIO_LOG_DBG("Domain %d connected", p.domain());

		for (auto &x : modules) {
			if (x.second == p.domain()) {
				p.add_names(x.first);
			}
		}

		s = new std::string();
		if (!p.SerializeToString(s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		EXPIO_LOG_INFO("Domain %d registered", p.domain());
		send_string(s, p.domain());
		delete (s);
	}
}

void controller_mpi::send_string(std::string *str, yapsc_domain_t domain) {
	yapsc::send_string_mpi(comm, domain, 0, str);
}

std::string *controller_mpi::recv_string(yapsc_domain_t domain, bool blocking) {
	return yapsc::recv_string_mpi(comm, domain, 0, blocking);
}
