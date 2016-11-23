#ifndef YAPSC_CONTROLLER_H
#define YAPSC_CONTROLLER_H

#include "yapsc_client.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class controller : public yapsc::client {
      public:
	controller(const char *name);
	virtual ~controller();

	virtual bool module_domain(const std::string module_name);
	virtual void initiator_socket(yapsc_initiator_socket &init,
				      const std::string module_name,
				      const std::string socket_name);
	virtual void target_socket(const std::string module_name,
				   const std::string socket_name,
				   yapsc_target_socket &target);
	virtual void start(void) = 0;

      protected:
	yapsc_module_domain_map modules;
	uint32_t domains;
};
};

#endif
