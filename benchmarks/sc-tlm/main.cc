#include "main.h"
#include "consumer.h"
#include "producer.h"
#include <expio.h>
#include <time.h>

unsigned int sent = 0;
unsigned int answered = 0;
unsigned int received = 0;

/***************************************************************************
 *  sc_main
 ***************************************************************************/
int sc_main(int argc, char *argv[]) {
	int REPT = 100000;
	expio_log_init();

	srand(time(NULL));

	if (argc >= 2) {
		REPT = atoi(argv[1]);
	} else {
		REPT = 1000 + rand() % 1000000;
		expio_log_level_global_set((EXPIO_Log_Level)0);
	}
	consumer *c;
	producer *p;

	{ c = new consumer("consumer", REPT); }
	{
		p = new producer("producer", REPT);

		p->isocket.bind(c->tsocket);
	}

	expio_stats_t *stats = NULL;
	{
		stats =
		    expio_stats_tsv_new("benchmarks.tsv", "Latency", "systemc",
					VERSION, "no-wait", "", REPT);
		expio_stats_begin(stats);
	}
	sc_start();
	{
		expio_stats_end(stats);
		if (!expio_stats_write(stats))
			return 1;
		expio_stats_delete(stats);
	}

	{ delete (c); }
	{ delete (p); }

	expio_log_shutdown();
	return 0;
}
