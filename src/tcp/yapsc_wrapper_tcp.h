#ifndef TLM_WRAPPER_TCP_H_
#define TLM_WRAPPER_TCP_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_serial.h"

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct wrapper_tcp : public yapsc::wrapper_serial {
      public:
	wrapper_tcp(sc_core::sc_module_name module_name);
	virtual ~wrapper_tcp();

	virtual void send_string(const std::string *str);
	virtual std::string *recv_string();
	void send_bytes(int connfd, const char *bytes, uint32_t len, int flags);

      protected:
	int sockfd;
	int connfd;

      private:
	uint32_t buffer_sz = 0;
	char *buffer = NULL;
};
};
#endif
