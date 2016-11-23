#include "main.h"
#include "producer.h"
#include "consumer.h"
#include <time.h>
#include <yapsc.h>
#include <yapsc_uds.h>

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
	char ns[32];
	expio_log_init();

	srand(time(NULL));
	gen_random(ns, 31);

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

	yapsc::proxy_uds *PW;
	yapsc::remote_uds *RW;
	consumer *c;
	producer *p;

	if (pid) {
		c = new consumer("consumer", REPT);

		RW = new yapsc::remote_uds("RW", ns);
		EXPIO_LOG_INFO("Remote Wrapper: %s", ns);

		RW->isocket.bind(c->tsocket);
	} else {
		p = new producer("producer", REPT);

		PW = new yapsc::proxy_uds("PW", ns);
		EXPIO_LOG_INFO("Proxy Wrapper: %s", ns);

		p->isocket.bind(PW->tsocket);
	}

	YAPSC_START();

	if (pid) {
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
