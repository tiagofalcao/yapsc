#include "yapsc_client_sc.h"
#include <expio.h>
#include <systemc.h>

using yapsc::client_sc;

client_sc::client_sc(const char *name) : yapsc::client(name, -1) {}
client_sc::~client_sc() {}

const char *client_sc::backend_name() { return "SystemC"; }

bool client_sc::module_domain(const std::string module_name) { return true; }

void client_sc::initiator_socket(yapsc_initiator_socket &init,
				 const std::string module_name,
				 const std::string socket_name) {
	std::string name = module_name + "_" + socket_name;
	initiators.insert(yapsc_initiator_pair(name, init));
}

void client_sc::target_socket(const std::string module_name,
			      const std::string socket_name,
			      yapsc_target_socket &target) {
	std::string name = module_name + "_" + socket_name;
	targets.insert(yapsc_target_pair(name, target));
}

void client_sc::start(void) {
	for (auto &x : initiators) {
		yapsc_initiator_socket &init = x.second;
		yapsc_target_socket &target = targets.at(x.first);
		EXPIO_LOG_INFO("Binding: %s", x.first.c_str());
		init.bind(target);
	}
	try {
		EXPIO_LOG_INFO("Starting kernal");
		if (before_simulation)
			before_simulation();
		sc_start();
		error_simulation = NULL;
		if (success_simulation)
			success_simulation();
		EXPIO_LOG_INFO("Exited kernal");
	} catch (std::exception &e) {
		EXPIO_LOG_WARN("Caught exception %s", e.what());
	} catch (...) {
		EXPIO_LOG_ERR("Caught exception during simulation.");
	}

	if (not sc_end_of_simulation_invoked()) {
		EXPIO_LOG_ERR("Simulation stopped without explicit sc_stop()");
		sc_stop();
	}
	if (error_simulation)
		error_simulation();
	EXPIO_LOG_INFO("The end.");
}

yapsc::client *get_client_sc(int *argc, char ***argv) {
	return new client_sc("");
}
