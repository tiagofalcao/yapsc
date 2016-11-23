#include "../../build/src/serialize/yapsc_message.pb.h"
#include "../core/yapsc_core.h"
#include "yapsc_payload_extension.h"
#include "yapsc_wrapper_tcp.h"
#include "yapsc_connection_tcp.h"

#include <expio.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>

using yapsc::wrapper_tcp;
typedef yapsc::wrapper_tcp SC_CURRENT_USER_MODULE;

/**
 * Default constructor.
 */

wrapper_tcp::wrapper_tcp(sc_core::sc_module_name module_name)
    : yapsc::wrapper_serial(module_name) {
	sockfd = -1;
	connfd = -1;
}

/**
 * Default destructor.
 */
wrapper_tcp::~wrapper_tcp() {
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
}

void wrapper_tcp::send_bytes(int connfd, const char *bytes, uint32_t len, int flags) {
	int sent = 0;
	while (sent != len) {
		int s = send(connfd, &(bytes[sent]), len - sent, flags);
		if (sent < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				EXPIO_LOG_CRIT("Send error: %d %s", errno,
						strerror(errno));
			} else {
				EXPIO_LOG_DBG("Send error: %d %s", errno,
						strerror(errno));
			}
			continue;
		} else if (send == 0) {
			EXPIO_LOG_CRIT("Disconnected");
		}
		sent += s;
		if (sent != len) {
			EXPIO_LOG_DBG("Sent few bytes that required %d != %u",
				      sent, len);
		}
	}
}

void wrapper_tcp::send_string(const std::string *str) {
	yapsc::send_string_tcp(connfd, str);
}

std::string *wrapper_tcp::recv_string() {
	return yapsc::recv_string_tcp(connfd, false);
}
