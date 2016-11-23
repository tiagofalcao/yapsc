#ifndef YAPSC_TCP_H
#define YAPSC_TCP_H

#include <stdint.h>

namespace yapsc {
bool tcp_zeroconf_publish_init(const char *name, int port);
void tcp_zeroconf_publish_shutdown(void);
bool tcp_zeroconf_discover(const char *name, char *address, int *port);
};

#include "yapsc_proxy_tcp.h"
#include "yapsc_remote_tcp.h"
#endif
