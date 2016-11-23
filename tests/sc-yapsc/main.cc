#include "main.h"
#include "producer.h"
#include "consumer.h"
#include <yapsc.h>


unsigned int sent = 0;
unsigned int answered = 0;
unsigned int received = 0;

/***************************************************************************
 *  sc_main
 ***************************************************************************/
int sc_main(int argc, char *argv[]) {
	expio_log_init();
	YAPSC_INIT(&argc, &argv);

	consumer *c;
	producer *p;

	YAPSC_MODULE("consumer"){
		c = new consumer("consumer", 100);
		YAPSC_TARGET_SOCKET("consumer", "socket", c->tsocket);
	}

	YAPSC_MODULE("producer"){
		p = new producer("producer", 100);
		YAPSC_INITIATOR_SOCKET(p->isocket, "consumer", "socket");
	}

	YAPSC_START();

	YAPSC_MODULE("consumer"){
		delete(c);
	}

	YAPSC_MODULE("producer"){
		delete(p);
	}

	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
