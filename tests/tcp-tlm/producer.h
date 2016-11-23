#ifndef PRODUCER_H
#define PRODUCER_H

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>

#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/peq_with_get.h>

SC_MODULE(producer) {
	tlm_utils::simple_initiator_socket<producer> isocket;
	tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload & payload,
					   tlm::tlm_phase & phase,
					   sc_core::sc_time & delay_time);

	tlm_utils::peq_with_get<tlm::tlm_generic_payload> peq;
	void proc();
	unsigned int send;

	producer(sc_core::sc_module_name module_name, unsigned int send);
	~producer();
};
#endif
