#include "main.h"
#include "consumer.h"
#include "producer.h"
#include <expio.h>
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
	char name[32];
	expio_log_init();

	srand(time(NULL));
	if (argc >= 2) {
		strncpy(name, argv[1], 31);
	} else {
		gen_random(name, 31);
	}

	if (argc >= 4) {
		REPT = atoi(argv[3]);
	} else {
		REPT = 1000 + rand() % 1000000;
		expio_log_level_global_set((EXPIO_Log_Level)0);
	}

	pid_t pid;
	if (argc >= 3) {
		pid = atoi(argv[2]);
	} else {
		pid = fork();
		if (pid < 0) {
			EXPIO_LOG_CRIT("Fork Error");
		}
	}
	pid = !!pid;
	YAPSC_INIT_VIRTUAL("", pid);

	yapsc::proxy_uds *PW;
	yapsc::remote_uds *RW;
	consumer *c;
	producer *p;

	if (pid) {
		c = new consumer("consumer", REPT);

		RW = new yapsc::remote_uds("RW", name);
		EXPIO_LOG_INFO("Remote Wrapper: %s", name);

		RW->isocket.bind(c->tsocket);
	} else {
		p = new producer("producer", REPT);

		EXPIO_LOG_INFO("Proxy Wrapper: %s", name);
		PW = new yapsc::proxy_uds("PW", name);

		p->isocket.bind(PW->tsocket);
	}

	expio_stats_t *stats = NULL;
	if (!pid) {
		stats = expio_stats_tsv_new("benchmarks.tsv", "Latency", "uds",
					    VERSION, "wait", "", REPT);
		expio_stats_begin(stats);
	}
	sc_start();
	if (!pid) {
		expio_stats_end(stats);
		if (!expio_stats_write(stats))
			return 1;
		expio_stats_delete(stats);
	}

	if (pid) {
		delete (c);
		delete (RW);
	} else {
		delete (p);
		delete (PW);
	}

	google::protobuf::ShutdownProtobufLibrary();
	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
