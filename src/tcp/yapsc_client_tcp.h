#ifndef YAPSC_CLIENT_TCP_H
#define YAPSC_CLIENT_TCP_H

#include "yapsc_client_serial.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class client_tcp : public yapsc::client_serial {
      public:
	client_tcp(const char *name, yapsc_domain_t domain);
	virtual ~client_tcp();

	virtual const char *backend_name();

      private:
	void connect_server(const char *addr, unsigned int port);

	virtual void bind(void);
	virtual void send_string(std::string *str);
	virtual std::string *recv_string(bool blocking);
	uint32_t buffer_sz = 0;
	char *buffer = NULL;

	int port = 0;
	char address[64];

	int sockfd;
	int connfd;
};
};

extern "C" {
yapsc::client *get_client_tcp(int *argc, char ***argv);
}

#endif
