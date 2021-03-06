#include "../../build/src/serialize/yapsc_message.pb.h"
#include <expio.h>
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
	char ns[32];
	int port;
	expio_log_init();

	yapsc::remote_tcp *RW = new yapsc::remote_tcp("RW");
	EXPIO_LOG_INFO("Remote Wrapper: %d", RW->port_get());

	yapsc::proxy_tcp *PW =
	    new yapsc::proxy_tcp("PW", "127.0.0.1", RW->port_get());
	EXPIO_LOG_INFO("Proxy Wrapper");

	gen_random(ns, 31);
	std::string msg(ns, 32);

	std::string *msg2 = RW->recv_string();
	if (msg2) {
		EXPIO_LOG_CRIT("Receive before send %s", msg2->c_str());
	}

	for (int i = 0; i < 1000; i++) {
		PW->send_string(&msg);
		for (int i = 0; i < 10; i++) {
			msg2 = RW->recv_string();
			if (msg2) {
				break;
			}
			EXPIO_LOG_DBG("Wait more for message");
			sleep(1);
		}
		if (!msg2) {
			EXPIO_LOG_CRIT("Receive fail");
		}

		EXPIO_LOG_INFO("Message received: %s", msg2->c_str());
		if (msg != (*msg2)) {
			EXPIO_LOG_CRIT("Message differences: %s != %s",
				       msg.c_str(), msg2->c_str());
		}
		delete(msg2);
	}

	sc_stop();

	delete (PW);
	delete (RW);

	google::protobuf::ShutdownProtobufLibrary();
	expio_log_shutdown();
	return 0;
}
