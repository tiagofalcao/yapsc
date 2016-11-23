#ifndef TLM_WRAPPER_SERIAL_H_
#define TLM_WRAPPER_SERIAL_H_

//////////////////////////////////////////////////////////////////////////////

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#include <tlm>

//////////////////////////////////////////////////////////////////////////////
namespace yapsc {

struct wrapper_serial : public ::sc_core::sc_module {
      public:
	wrapper_serial(sc_core::sc_module_name module_name);
	virtual ~wrapper_serial();

	virtual void proc_in();

	virtual void send_payload(tlm::tlm_generic_payload *payload);
	virtual tlm::tlm_generic_payload *recv_payload();

	virtual void send_string(const std::string *str) = 0;
	virtual std::string *recv_string() = 0;

	virtual std::string *encode_payload(tlm::tlm_generic_payload *payload);
	virtual tlm::tlm_generic_payload *decode_payload(std::string *str);
};
};
#endif
