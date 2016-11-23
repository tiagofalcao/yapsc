#ifndef YAPSC_CONTROLLER_TCP_H
#define YAPSC_CONTROLLER_TCP_H

#include "yapsc_controller_serial.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class controller_tcp : public yapsc::controller_serial {
      public:
	controller_tcp(const char *name);
	virtual ~controller_tcp();

	virtual const char *backend_name();

      private:
	int port = 0;

	virtual void before_start(void);
	virtual void send_string(std::string *str, yapsc_domain_t domain);
	virtual std::string *recv_string(yapsc_domain_t domain, bool blocking);
	virtual const std::string *domain_address_get(yapsc_domain_t domain);

	bool start_server(int p);
	bool connection_get();

	int sockfd;
	int *connfd;
	std::string **domain_address;

	uint32_t buffer_sz = 0;
	char *buffer = NULL;
};
};

#endif
