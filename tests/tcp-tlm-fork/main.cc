#include "main.h"
#include "producer.h"
#include "consumer.h"
#include <time.h>
#include <yapsc.h>
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

unsigned int sent = 0;
unsigned int answered = 0;
unsigned int received = 0;

/***************************************************************************
 *  sc_main
 ***************************************************************************/
int sc_main(int argc, char *argv[]) {
	int REPT = 100000;
	char name[32];
	expio_log_init();

	srand(time(NULL));
	gen_random(name, 31);


	if (argc >= 2) {
		REPT = atoi(argv[1]);
	} else {
		REPT = 1000;
	}

	pid_t pid = fork();
	if (pid < 0) {
		EXPIO_LOG_CRIT("Fork Error");
	}
	pid = !!pid;
	YAPSC_INIT_VIRTUAL("", pid);

	yapsc::proxy_tcp *PW;
	yapsc::remote_tcp *RW;
	consumer *c;
	producer *p;

	if (pid) {
		c = new consumer("consumer", 1000);

		RW = new yapsc::remote_tcp("RW");
		int port = RW->port_get();
		EXPIO_LOG_INFO("Remote Wrapper: %d", port);

		if (!yapsc::tcp_zeroconf_publish_init(name, port)) {
			EXPIO_LOG_CRIT("yapsc_controller_tcp_zeroconf_setup");
		}

		RW->isocket.bind(c->tsocket);
	} else {
		p = new producer("producer", 1000);

		int port = 0;
		char address[64];
		if (!yapsc::tcp_zeroconf_discover(name, address, &port)) {
			EXPIO_LOG_CRIT("yapsc_lp_tcp_zeroconf_discover");
		}

		EXPIO_LOG_INFO("Proxy Wrapper: %s:%d", address, port);
		PW = new yapsc::proxy_tcp("PW", address, port);

		p->isocket.bind(PW->tsocket);
	}

	YAPSC_START();

	if (pid) {
		yapsc::tcp_zeroconf_publish_shutdown();
		delete(c);
		delete (RW);
	} else {
		delete(p);
		delete (PW);
	}

	google::protobuf::ShutdownProtobufLibrary();
	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
