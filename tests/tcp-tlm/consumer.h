#ifndef CONSUMER_H
#define CONSUMER_H

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>

#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_get.h>

SC_MODULE(consumer) {
	tlm_utils::simple_target_socket<consumer> tsocket;
	tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload & payload,
					   tlm::tlm_phase & phase,
					   sc_core::sc_time & delay_time);

	tlm_utils::peq_with_get<tlm::tlm_generic_payload> peq;
	void proc();
	unsigned int recv;

	consumer(sc_core::sc_module_name module_name, unsigned int recv);
	~consumer();
};
#endif
