#ifndef TLM_WRAPPER_UDS_H_
#define TLM_WRAPPER_UDS_H_

//////////////////////////////////////////////////////////////////////////////

#include "yapsc_wrapper_serial.h"

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct wrapper_uds : public yapsc::wrapper_serial {
      public:
	wrapper_uds(sc_core::sc_module_name module_name, const char *ns);
	virtual ~wrapper_uds();

	virtual void send_string(const std::string *str);
	virtual std::string *recv_string();

      protected:
	int sockfd;
	int connfd;
	char *ns;

      private:
	uint32_t buffer_sz = 0;
	char *buffer = NULL;
};
};
#endif
