#ifndef YAPSC_CLIENT_UDS_H
#define YAPSC_CLIENT_UDS_H

#include "yapsc_client_serial.h"
#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class client_uds : public yapsc::client_serial {
      public:
	client_uds(const char *name, yapsc_domain_t domain);
	virtual ~client_uds();

	virtual const char *backend_name();

      private:
	void connect_server();

	virtual void bind(void);
	virtual void send_string(std::string *str);
	virtual std::string *recv_string(bool blocking);

	int sockfd;
	int connfd;
};
};

extern "C" {
yapsc::client *get_client_uds(int *argc, char ***argv);
}

#endif
