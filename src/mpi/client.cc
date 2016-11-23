#include "../../build/src/serialize/yapsc_message.pb.h"
#include "yapsc_client_mpi.h"
#include "yapsc_connection_mpi.h"
#include "yapsc_controller_mpi.h"
#include "yapsc_mpi.h"
#include <expio.h>
#include <systemc.h>

using yapsc::client_mpi;

client_mpi::client_mpi(const char *name, MPI_Comm comm, yapsc_domain_t domain)
    : yapsc::client_serial(name, domain) {
	this->comm = comm;
	{
		yapsc::Message p;
		p.set_type(p.DOMAIN_REGISTRY);
		p.set_domain(domain_get());

		std::string *s = new std::string();
		if (!p.SerializeToString(s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		EXPIO_LOG_DBG("Registering %d to controller", p.domain());
		send_string(s);
		delete (s);
	}
	get_modules();
}

client_mpi::~client_mpi() {
	EXPIO_LOG_DBG("MPI_Finalize");
	MPI_Barrier(comm);
	MPI_Finalize();
}

const char *client_mpi::backend_name() { return "MPI"; }

void client_mpi::bind(void) {
	for (auto &x : local_initiators) {
		yapsc_initiator_socket &init = x.second;
		yapsc_target_socket &target = targets.at(x.first);
		targets.erase(x.first);
		EXPIO_LOG_INFO("Binding: %s", x.first.c_str());
		init.bind(target);
	}
	yapsc::Message p;
	std::string s;
	yapsc::remote_mpi *RW;
	int tag = 10000 * domain;
	for (auto &x : targets) {
		std::string wrapper = x.first + "_RW";
		RW = new yapsc::remote_mpi(wrapper.c_str(), comm, tag);
		yapsc_target_socket &target = x.second;
		RW->isocket.bind(target);

		EXPIO_LOG_DBG("Socket %s at %d:%d", x.first.c_str(),
			      domain_get(), tag);
		p.Clear();
		p.set_type(p.TARGET_SOCKET_REGISTRY);
		p.set_socketname(x.first);
		p.set_port(tag);
		p.set_domain(domain_get());
		if (!p.SerializeToString(&s)) {
			EXPIO_LOG_CRIT("Serialize error");
		}
		send_string(&s);
		tag++;
	}
	action_barrier();
	yapsc::proxy_mpi *PW;
	for (auto &x : initiators) {
		int target_tag = 0;
		int target_domain = 0;
		const char *addr;
		while (!target_tag) {
			p.Clear();
			p.set_type(p.TARGET_SOCKET_QUERY);
			p.set_socketname(x.first);
			if (!p.SerializeToString(&s)) {
				EXPIO_LOG_CRIT("Serialize error");
			}
			send_string(&s);
			std::string *str = recv_string(true);
			if (!p.ParseFromString(*str)) {
				EXPIO_LOG_CRIT("Parser error");
			}
			delete (str);
			target_tag = p.port();
			if (target_tag) {
				target_domain = p.domain();
				break;
			}
			EXPIO_LOG_DBG("Failed to query, sleep(5) to retry");
			sleep(5);
		}
		std::string wrapper = x.first + "_PW";
		EXPIO_LOG_DBG("Got socket %s at %d:%d", x.first.c_str(),
			      target_domain, target_tag);
		PW = new yapsc::proxy_mpi(wrapper.c_str(), comm, tag,
					  target_domain, target_tag);
		yapsc_initiator_socket &init = x.second;
		init.bind(PW->tsocket);
		tag++;
	}
}

void client_mpi::send_string(std::string *str) {
	yapsc::send_string_mpi(comm, 0, 0, str);
}

std::string *client_mpi::recv_string(bool blocking) {
	return yapsc::recv_string_mpi(comm, 0, 0, blocking);
}

yapsc::client *get_client_mpi(int *argc, char ***argv) {
#ifndef MPI_VERSION
	return NULL;
#else
	MPI_Init(argc, argv);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	if (world_size < 2) {
		MPI_Finalize();
		return NULL;
	}

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	char name[] = "YAPSC_MPI";

	if (world_rank) {
		return new client_mpi(name, MPI_COMM_WORLD, world_rank);
	} else {
		return new yapsc::controller_mpi(name, MPI_COMM_WORLD);
	}
#endif
}
