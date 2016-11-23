#include <expio.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <yapsc_tcp.h>

void gen_random(char *s, const int len) {
	static const char alphanum[] = "0123456789"
				       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				       "abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len] = 0;
}

int sc_main(int argc, char *argv[]) {
	char name[32];
	int port;
	expio_log_init();

	srand(time(NULL));

	char address[64];
	int port2;

	gen_random(name, 31);
	port = rand() % 65536;

	EXPIO_LOG_INFO("Publishing...");
	if (!yapsc::tcp_zeroconf_publish_init(name, port)) {
		EXPIO_LOG_CRIT("yapsc_controller_tcp_zeroconf_setup");
	}

	EXPIO_LOG_INFO("Discovering...");
	if (!yapsc::tcp_zeroconf_discover(name, address, &port2)) {
		EXPIO_LOG_CRIT("yapsc_lp_tcp_zeroconf_discover");
	} else if (port == port2) {
		EXPIO_LOG_INFO("%s %s %d", name, address, port);
	} else {
		EXPIO_LOG_CRIT("yapsc_lp_tcp_zeroconf_discover %d != %d", port,
			       port2);
	}

	yapsc::tcp_zeroconf_publish_shutdown();

	expio_log_shutdown();
	return 0;
}
