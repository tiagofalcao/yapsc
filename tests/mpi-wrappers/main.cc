#include "../../build/src/serialize/yapsc_message.pb.h"
#include <expio.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <yapsc_mpi.h>

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
	int ret, my_rank, comm_sz, tag;
	char ns[32];
	expio_log_init();

	ret = MPI_Init(&argc, &argv);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);
	ret = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);
	ret = MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);
	EXPIO_LOG_INFO("MPI Rank:%d World:%d", my_rank, comm_sz);

	tag = 133;

	if (my_rank == 1) {
		yapsc::remote_mpi *RW =
		    new yapsc::remote_mpi("RW", MPI_COMM_WORLD, tag);
		EXPIO_LOG_INFO("Remote Wrapper: %d:%d", my_rank, tag);
		RW->wait_connection();

		std::string *msg2;
		for (int i = 0; i < 1000; i++) {
			while(!(msg2 = RW->recv_string()));
			if (!msg2) {
				EXPIO_LOG_CRIT("Receive fail");
			}
			RW->send_string(msg2);
			delete (msg2);
		}
		delete (RW);
	} else if (my_rank == 0) {
		yapsc::proxy_mpi *PW =
		    new yapsc::proxy_mpi("PW", MPI_COMM_WORLD, tag + 1, 1, tag);
		EXPIO_LOG_INFO("Proxy Wrapper: %d:%d", my_rank, tag + 1);

		srand(time(NULL));
		std::string *msg2;
		for (int i = 0; i < 1000; i++) {
			gen_random(ns, 31);
			std::string msg(ns);
			PW->send_string(&msg);
			msg2 = NULL;
			while(!(msg2 = PW->recv_string()));
			if (!msg2) {
				EXPIO_LOG_CRIT("Receive fail");
			}

			EXPIO_LOG_INFO("Message received: %s", msg2->c_str());
			if (msg != (*msg2)) {
				EXPIO_LOG_CRIT("Message differences: %s != %s",
					       msg.c_str(), msg2->c_str());
			}
			delete (msg2);
		}
		delete (PW);
	}
	sc_stop();

	ret = MPI_Finalize();
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);

	google::protobuf::ShutdownProtobufLibrary();
	expio_log_shutdown();
	return 0;
}
