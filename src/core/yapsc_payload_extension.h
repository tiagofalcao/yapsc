#ifndef YAPSC_PAYLOAD_EXT_H
#define YAPSC_PAYLOAD_EXT_H

#include <stdint.h>
#include <tlm.h>
using namespace tlm;

namespace yapsc {

class yapsc_payload : public tlm_extension<yapsc_payload> {
      public:
	yapsc_payload(int32_t domain, uint64_t payload);
	~yapsc_payload();

	int32_t getDomain();
	uint64_t getPTR();

	tlm_extension_base *clone() const;
	void copy_from(tlm_extension_base const &ext);

      private:
	int32_t domain;
	uint64_t ptr;
};
};

#endif
