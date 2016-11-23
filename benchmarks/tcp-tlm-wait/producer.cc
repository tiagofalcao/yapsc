#include "producer.h"
#include "main.h"
#include <expio.h>
#include <stdlib.h>
#include <time.h>

typedef producer SC_CURRENT_USER_MODULE;

producer::producer(sc_core::sc_module_name module_name, unsigned int s)
    : sc_module(module_name), peq("peq"), isocket("initiator") {
	isocket.register_nb_transport_bw(this, &producer::nb_transport_bw);
	send = s;
	SC_THREAD(proc);
}
producer::~producer() {}

tlm::tlm_sync_enum
producer::nb_transport_bw(tlm::tlm_generic_payload &payload,
			  tlm::tlm_phase &phase, sc_core::sc_time &delay_time) {

	if (payload.get_response_status() != tlm::TLM_OK_RESPONSE) {
		EXPIO_LOG_CRIT("Wrong response status (%d).",
			       payload.get_response_status());
	}

	if (payload.get_command() != tlm::TLM_WRITE_COMMAND) {
		EXPIO_LOG_CRIT("Command %d is not implemented.",
			       payload.get_command());
	}

	unsigned int addr = payload.get_address();
	unsigned int expected_value = (((addr & 0xF0) >> 4) * 3) - 1;
	unsigned int *ptr = (unsigned int *)payload.get_data_ptr();
	EXPIO_LOG(100, "Payload complete: %u %p %u == %u", addr, ptr, *ptr, expected_value);

	if (expected_value != *ptr) {
		EXPIO_LOG_CRIT("Expected Value: %u != %u.", expected_value,
			       *ptr);
	}
	free(ptr);
	delete (&payload);

	received++;
	if (received == send) {
		if (received == sent) {
			sc_stop();
		} else {
			EXPIO_LOG_CRIT("Receved != Sended");
		}
	}
	transaction_done.notify(sc_core::SC_ZERO_TIME);
	EXPIO_LOG_DBG("Payload received (%d).", received);

	phase = tlm::END_REQ;
	return tlm::TLM_COMPLETED;
}

void producer::proc() {
	srand(time(NULL));

	for (unsigned int i = 0; i < send; i++) {
		unsigned int addr;
		tlm::tlm_generic_payload *payload =
		    new tlm::tlm_generic_payload();

		sent++;

		addr = ((rand() % 0xFF) & 0xFE) + (sent % 2);

		payload->set_address(addr);
		payload->set_data_length(4);

		unsigned int *ptr =
		    (unsigned int *)malloc(sizeof(unsigned int));
		*ptr = ((addr & 0xF0) >> 4) * 3;
		payload->set_data_ptr((unsigned char *)ptr);
		payload->set_write();
		EXPIO_LOG(100, "Payload send: %u %p %u == %u", addr, ptr, *ptr, ((addr & 0xF0) >> 4) * 3);

		sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
		tlm::tlm_phase phase = tlm::BEGIN_REQ;

		tlm::tlm_sync_enum resp =
		    isocket->nb_transport_fw(std::ref(*payload), phase, delay);
		if (resp != tlm::TLM_UPDATED) {
			EXPIO_LOG_CRIT("Response not updated: %d.", resp);
		}
		EXPIO_LOG_DBG("Payload sent (%d).", sent);
		wait(transaction_done);
	}
}
