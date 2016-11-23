#ifndef YAPSC_CLIENT_VIRTUAL_H
#define YAPSC_CLIENT_VIRTUAL_H

#include "yapsc_client.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class client_virtual : public yapsc::client {
      public:
	client_virtual(const char *name, yapsc_domain_t domain);
	virtual ~client_virtual();

	virtual const char *backend_name();
	virtual bool module_domain(const std::string module_name);
	virtual void initiator_socket(yapsc_initiator_socket &init,
				      const std::string module_name,
				      const std::string socket_name);
	virtual void target_socket(const std::string module_name,
				   const std::string socket_name,
				   yapsc_target_socket &target);
	virtual void start(void);

      private:
	yapsc_initiator_map initiators;
	yapsc_target_map targets;
};
};

#endif
