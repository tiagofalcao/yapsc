#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_wrapper_serial.h"

#include <expio.h>

using yapsc::wrapper_serial;
typedef yapsc::wrapper_serial SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

wrapper_serial::wrapper_serial(sc_core::sc_module_name module_name) : sc_module(module_name) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	SC_THREAD(proc_in);
}

/**
 * Default destructor.
 */
wrapper_serial::~wrapper_serial() {
}

void wrapper_serial::send_payload(tlm::tlm_generic_payload *payload) {
	std::string *str = encode_payload(payload);
	send_string(str);
	delete (str);
}

tlm::tlm_generic_payload *wrapper_serial::recv_payload() {
	std::string *str = recv_string();
	if (!str) {
		return NULL;
	}
	tlm_generic_payload *payload = decode_payload(str);
	delete (str);
	return payload;
}

std::string *wrapper_serial::encode_payload(tlm::tlm_generic_payload *payload) {
	yapsc::Payload *p = new yapsc::Payload();

	yapsc::yapsc_payload *yp = NULL;
	payload->get_extension(yp);
	if (yp) {
		p->set_domain(yp->getDomain());
		p->set_pointer(yp->getPTR());
	} else {
		p->set_domain((int32_t)yapsc_domain());
		p->set_pointer((uint64_t)payload);
	}

	uint64_t str_len = 0;
	std::string *str = NULL;
	yapsc::Payload_Memory *pm;

	p->set_command((int32_t)payload->get_command());
	p->set_responsestatus((int32_t)payload->get_response_status());
	p->set_address((uint64_t)payload->get_address());

	pm = p->mutable_data();
	str_len = (uint64_t)payload->get_data_length();
	str = new std::string((char *)payload->get_data_ptr(), str_len);
	pm->set_data(*str);
	pm->set_length(str_len);
	delete str;

	pm = p->mutable_byteenable();
	str_len = (uint64_t)payload->get_byte_enable_length();
	str = new std::string((char *)payload->get_byte_enable_ptr(), str_len);
	pm->set_data(*str);
	pm->set_length(str_len);
	delete str;

	p->set_streamingwidth((uint64_t)payload->get_streaming_width());
	p->set_dmiallowed((bool)payload->is_dmi_allowed());
	p->set_gpoption((int32_t)payload->get_gp_option());

	str = new std::string();
	if (p->SerializeToString(str)) {
		EXPIO_LOG_DBG("Encoded Payload with %d: %p %p %" PRIu64 "",
			      str->length(), payload, payload->get_data_ptr(),
			      payload->get_data_length());
	} else {
		EXPIO_LOG_CRIT("Payload encode error");
		delete (p);
		return NULL;
	}

	if (yp != NULL) {
		unsigned char *ptr;
		ptr = payload->get_data_ptr();
		if (ptr) {
			free(ptr);
		}
		ptr = payload->get_byte_enable_ptr();
		if (ptr) {
			free(ptr);
		}
		delete (payload);
	}
	delete (p);

	return str;
}

tlm::tlm_generic_payload *wrapper_serial::decode_payload(std::string *str) {
	yapsc::Payload *p = new yapsc::Payload();
	bool local;

	if (p->ParseFromString(*str)) {
		local = p->domain() == (int32_t)yapsc_domain();
	} else {
		EXPIO_LOG_CRIT("Payload decode error with %d", str->length());
		delete (p);
		return NULL;
	}

	tlm_generic_payload *payload;
	if (local) {
		payload = (tlm::tlm_generic_payload *)p->pointer();
	} else {
		payload = new tlm_generic_payload();
		yapsc::yapsc_payload *yp =
		    new yapsc::yapsc_payload(p->domain(), p->pointer());
		payload->set_extension(yp);
		payload->set_data_ptr(NULL);
		payload->set_byte_enable_ptr(NULL);
	}

	if (local) {
		EXPIO_LOG_DBG("Decoded Local Payload with %d: %p %p %" PRIu64
			      "",
			      str->length(), payload, payload->get_data_ptr(),
			      payload->get_data_length());
	} else {
		EXPIO_LOG_DBG("Decoded Transitional Payload with %d: %p",
			      str->length(), payload);
	}

	yapsc::Payload_Memory *pm;
	unsigned char *ptr = NULL;

	payload->set_command((tlm::tlm_command)p->command());
	payload->set_response_status(
	    (tlm::tlm_response_status)p->responsestatus());
	payload->set_address(p->address());

	pm = p->mutable_data();
	payload->set_data_length(pm->length());
	if (local) {
		ptr = payload->get_data_ptr();
	} else {
		ptr = (unsigned char *)malloc(pm->length());
		payload->set_data_ptr(ptr);
	}

	if (ptr) {
		memcpy(ptr, (pm->data()).c_str(), pm->length());
	}

	pm = p->mutable_byteenable();
	payload->set_byte_enable_length(pm->length());
	if (local) {
		ptr = payload->get_data_ptr();
	} else {
		ptr = (unsigned char *)malloc(pm->length());
		payload->set_byte_enable_ptr(ptr);
	}
	if (ptr) {
		memcpy(ptr, (pm->data()).c_str(), pm->length());
	}

	payload->set_streaming_width(p->streamingwidth());
	payload->set_dmi_allowed(p->dmiallowed());
	payload->set_gp_option((tlm::tlm_gp_option)p->gpoption());

	delete (p);
	return payload;
}

void wrapper_serial::proc_in() {}
