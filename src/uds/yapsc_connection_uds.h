#ifndef _YAPSC_CONNECTION_UDS_H
#define _YAPSC_CONNECTION_UDS_H

#include <stdint.h>
#include <string>

namespace yapsc {
void send_string_uds(int connfd, const std::string *str);
std::string *recv_string_uds(int connfd, bool blocking);
}

#endif
