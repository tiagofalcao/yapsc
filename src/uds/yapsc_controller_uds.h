#ifndef YAPSC_CONTROLLER_UDS_H
#define YAPSC_CONTROLLER_UDS_H

#include "yapsc_controller_serial.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class controller_uds : public yapsc::controller_serial {
      public:
	controller_uds(const char *name);
	virtual ~controller_uds();

	virtual const char *backend_name();

      private:
	virtual void before_start(void);
	virtual void send_string(std::string *str, yapsc_domain_t domain);
	virtual std::string *recv_string(yapsc_domain_t domain, bool blocking);

	bool start_server();
	bool connection_get();

	int sockfd;
	int *connfd;
};
};

#endif
