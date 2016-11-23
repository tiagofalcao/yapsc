#ifndef _YAPSC_CONNECTION_TCP_H
#define _YAPSC_CONNECTION_TCP_H

#include <stdint.h>
#include <string>

namespace yapsc {
void send_string_tcp(int connfd, const std::string *str);
std::string *recv_string_tcp(int connfd, bool blocking);
}

#endif
