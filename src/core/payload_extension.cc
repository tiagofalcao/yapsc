#include "yapsc_payload_extension.h"

using yapsc::yapsc_payload;

yapsc_payload::yapsc_payload(int32_t domain, uint64_t payload) {
	this->domain = domain;
	this->ptr = payload;
}

yapsc_payload::~yapsc_payload() {}

int32_t yapsc_payload::getDomain() { return this->domain; }

uint64_t yapsc_payload::getPTR() { return this->ptr; }

tlm_extension_base *yapsc_payload::clone() const {
	return new yapsc_payload(domain, ptr);
}

void yapsc_payload::copy_from(tlm_extension_base const &ext) {
	this->domain = static_cast<yapsc_payload const &>(ext).domain;
	this->ptr = static_cast<yapsc_payload const &>(ext).ptr;
}
