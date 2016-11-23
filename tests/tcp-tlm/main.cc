#include "main.h"
#include "producer.h"
#include "consumer.h"
#include <yapsc.h>
#include <yapsc_tcp.h>

unsigned int sent = 0;
unsigned int answered = 0;
unsigned int received = 0;

/***************************************************************************
 *  sc_main
 ***************************************************************************/
int sc_main(int argc, char *argv[]) {
	int REPT = 100000;
	int port = 0;
	expio_log_init();

	if (argc >= 2) {
		REPT = atoi(argv[1]);
	} else {
		REPT = 1000;
	}
	YAPSC_INIT_VIRTUAL("", 0);

	yapsc::proxy_tcp *PW;
	yapsc::remote_tcp *RW;
	consumer *c;
	producer *p;

	{
		c = new consumer("consumer", REPT);

		RW = new yapsc::remote_tcp("RW");
		port = RW->port_get();
		EXPIO_LOG_INFO("Remote Wrapper: %d", port);

		RW->isocket.bind(c->tsocket);
	}
	{
		p = new producer("producer", REPT);

		EXPIO_LOG_INFO("Proxy Wrapper: %d", port);
		PW = new yapsc::proxy_tcp("PW", "127.0.0.1", port);

		p->isocket.bind(PW->tsocket);
	}

	YAPSC_START();

	{
		delete(c);
		delete (RW);
	}
	{
		delete(p);
		delete (PW);
	}

	google::protobuf::ShutdownProtobufLibrary();
	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
