#include "yapsc_client_virtual.h"
#include <expio.h>
#include <systemc.h>

using yapsc::client_virtual;

client_virtual::client_virtual(const char *name, yapsc_domain_t domain)
    : yapsc::client(name, domain) {}

client_virtual::~client_virtual() {}

const char *client_virtual::backend_name() { return "Virtual"; }

bool client_virtual::module_domain(const std::string module_name) {
	return false;
}

void client_virtual::initiator_socket(yapsc_initiator_socket &init,
				      const std::string module_name,
				      const std::string socket_name) {}

void client_virtual::target_socket(const std::string module_name,
				   const std::string socket_name,
				   yapsc_target_socket &target) {}

void client_virtual::start(void) {
	try {
		EXPIO_LOG_INFO("Starting kernal");
		sc_start();
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
	EXPIO_LOG_INFO("The end.");
}
