#ifndef YAPSC_CONTROLLER_SERIAL_H
#define YAPSC_CONTROLLER_SERIAL_H

#include "yapsc_controller.h"
#include "yapsc_core.h"
#include <stdint.h>

typedef std::pair<yapsc_domain_t, int> yapsc_domain_port_pair;
typedef std::map<std::string, yapsc_domain_port_pair> yapsc_serial_target_map;
typedef std::pair<std::string, yapsc_domain_port_pair> yapsc_serial_target_pair;

namespace yapsc {

class controller_serial : public yapsc::controller {
      public:
	controller_serial(const char *name);
	virtual ~controller_serial();

	virtual void start(void);

      protected:
	virtual void before_start(void);
	virtual void send_string(std::string *str, yapsc_domain_t domain) = 0;
	virtual std::string *recv_string(yapsc_domain_t domain, bool blocking) = 0;
	virtual const std::string *domain_address_get(yapsc_domain_t domain);

	virtual void action_end(yapsc_domain_t domain, int status);
	virtual bool action_socket_registry(yapsc::Message *p,
					    yapsc_domain_t domain);
	virtual bool action_socket_query(yapsc::Message *p,
					 yapsc_domain_t domain);
	virtual bool action_barrier(yapsc::Message *p, yapsc_domain_t domain);
	virtual bool action_syscall(yapsc::Message *p, yapsc_domain_t domain);

	int ending_status;
	int ending;
	bool *ending_notify;

	yapsc_serial_target_map targets;
};
};

#endif
