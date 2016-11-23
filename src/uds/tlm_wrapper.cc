#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_wrapper_uds.h"
#include "yapsc_connection_uds.h"

#include <expio.h>

#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

using yapsc::wrapper_uds;
typedef yapsc::wrapper_uds SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

wrapper_uds::wrapper_uds(sc_core::sc_module_name module_name, const char *ns) : yapsc::wrapper_serial(module_name) {
	sockfd = -1;
	connfd = -1;
	this->ns = strdup(ns);
}

/**
 * Default destructor.
 */
wrapper_uds::~wrapper_uds() {
	if (connfd >= 0) {
		close(connfd);
		connfd = -1;
	}
	if (sockfd >= 0) {
		close(sockfd);
		sockfd = -1;
	}
	if (buffer) {
		free(buffer);
		buffer_sz = 0;
		buffer = NULL;
	}
	free(ns);
}

void wrapper_uds::send_string(const std::string *str) {
	yapsc::send_string_uds(connfd, str);
}

std::string *wrapper_uds::recv_string() {
	return yapsc::recv_string_uds(connfd, false);
}
