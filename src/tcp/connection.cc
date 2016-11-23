#include "yapsc_connection_tcp.h"
#include <expio.h>

#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>

void yapsc::send_string_tcp(int connfd, const std::string *str) {
	uint32_t len = str->length();
	if (len == 0) {
		EXPIO_LOG_ERR("Can not send a empty message");
		return;
	}
	{
		uint32_t L;
		L = htole32(len);
		uint32_t T = 0;
		const char *s = (const char *) &L;
		while (T != sizeof(L)) {
			int sent = send(connfd, &(s[T]), sizeof(L)-T, MSG_MORE);
			if (sent < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					continue;
				}
				EXPIO_LOG_ERR("Send error: %d %s", errno,
					      strerror(errno));
			} else if (send == 0) {
				EXPIO_LOG_ERR("Disconnected");
			}
			T += sent;
		}
	}
	{
		uint32_t T = 0;
		const char *s = str->c_str();
		while (T != len) {
			int sent = send(connfd, &(s[T]), len-T, MSG_DONTWAIT);
			if (sent < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					continue;
				}
				EXPIO_LOG_ERR("Send error: %d %s", errno,
					      strerror(errno));
			} else if (send == 0) {
				EXPIO_LOG_ERR("Disconnected");
			}
			T += sent;
		}
	}
}

std::string *yapsc::recv_string_tcp(int connfd, bool blocking) {
	uint32_t len = 0;
	int flags = 0;
	if (!blocking) {
		flags = MSG_DONTWAIT;
	}
	{
		int r = recv(connfd, &len, sizeof(len), flags | MSG_WAITALL);
		len = le32toh(len);
		if (!len) {
			return NULL;
		} else if (r != sizeof(len)) {
			EXPIO_LOG_ERR("Received wrong amount of bytes");
		}
		EXPIO_LOG_INFO("Received message of %" PRIu32 " bytes length",
			       len);
	}
	{
		char *buf = (char *)malloc(len * sizeof(char));
		int r = recv(connfd, buf, len, MSG_WAITALL);
		if (((uint32_t)r) != len) {
			EXPIO_LOG_ERR("Received wrong amount of bytes");
		}
		std::string *str = new std::string(buf, len);
		free(buf);
		return str;
	}
}
