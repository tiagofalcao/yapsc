#include "consumer.h"
#include "main.h"
#include <expio.h>

typedef consumer SC_CURRENT_USER_MODULE;

consumer::consumer(sc_core::sc_module_name module_name, unsigned int recv)
    : sc_module(module_name), peq("peq"), tsocket("target") {
	tsocket.register_nb_transport_fw(this, &consumer::nb_transport_fw);

	SC_THREAD(proc);
	this->recv = recv;
}

consumer::~consumer() {}

tlm::tlm_sync_enum consumer::nb_transport_fw(tlm::tlm_generic_payload &payload,
					     tlm::tlm_phase &phase,
					     sc_core::sc_time &delay_time) {

	if (payload.get_command() != tlm::TLM_WRITE_COMMAND) {
		EXPIO_LOG_CRIT("Command %d is not implemented.",
			       payload.get_command());
	}

	unsigned int addr = payload.get_address();
	unsigned int expected_value = ((addr & 0xF0) >> 4) * 3;
	unsigned int *ptr = (unsigned int *)payload.get_data_ptr();
	EXPIO_LOG(3, "Payload answer: %u %p %u == %u", addr, ptr, *ptr, expected_value);

	if (!ptr) {
		EXPIO_LOG_CRIT("Data pointer mandatory to write (%llx).",
			       (long long unsigned int)ptr);
	} else if (expected_value != *ptr) {
		EXPIO_LOG_CRIT("Expected Value: %u != %u.", expected_value,
			       *ptr);
	}
	(*ptr)--;

	delay_time = sc_core::SC_ZERO_TIME;
	payload.set_response_status(tlm::TLM_OK_RESPONSE);

	answered++;

	EXPIO_LOG_DBG("Payload answered (%d).", answered);
	peq.notify(payload, delay_time);
	phase = tlm::END_REQ;
	return tlm::TLM_UPDATED;
}

void consumer::proc() {
	sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

	while (recv > 0) {
		tlm::tlm_generic_payload *payload;

		wait(peq.get_event());
		payload = peq.get_next_transaction();

		tlm::tlm_phase phase = tlm::BEGIN_RESP;

		tlm::tlm_sync_enum resp =
		    tsocket->nb_transport_bw(std::ref(*payload), phase, delay);
		if (resp != tlm::TLM_COMPLETED) {
			EXPIO_LOG_CRIT("Response not completed: %d.", resp);
		}
		recv--;
	}

	wait(100, SC_NS);
	sc_stop();
}
