#include "yapsc_controller.h"
#include <expio.h>
#include <systemc.h>

using yapsc::controller;

controller::controller(const char *name) : yapsc::client(name, 0), modules() {
	domains = 0;
	{
		FILE *arq = fopen("partition.yapsc", "r");
		if (!arq) {
			EXPIO_LOG_CRIT("Missing partition.yapsc");
		}
		char m[32];
		int d;
		while (fscanf(arq, "%s %d\n", m, &d) == 2) {
			if (d > domains) {
				domains = d;
			}
			EXPIO_LOG_DBG("Loading module %s on domain %d", m, d);
			modules.insert(
			    yapsc_module_domain_pair(std::string(m), d));
		}
		fclose(arq);
	}
	EXPIO_LOG_INFO("Expected amount of domains: %d", domains);
}

controller::~controller() {
}

bool controller::module_domain(const std::string module_name) { return false; }

void controller::initiator_socket(yapsc_initiator_socket &init,
				  const std::string module_name,
				  const std::string socket_name) {
	EXPIO_LOG_ERR("Adding initiator socket on controller");
}

void controller::target_socket(const std::string module_name,
			       const std::string socket_name,
			       yapsc_target_socket &target) {
	EXPIO_LOG_ERR("Adding target socket on controller");
}
