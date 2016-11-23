#ifndef YAPSC_CLIENT_SC_H
#define YAPSC_CLIENT_SC_H

#include "yapsc_client.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class client_sc : public yapsc::client {
      public:
	client_sc(const char *name);
	virtual ~client_sc();

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

extern "C" {
yapsc::client *get_client_sc(int *argc, char ***argv);
}

#endif
