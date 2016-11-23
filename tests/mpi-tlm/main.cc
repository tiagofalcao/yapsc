#include "main.h"
#include "producer.h"
#include "consumer.h"
#include <time.h>
#include <yapsc.h>
#include <yapsc_mpi.h>

unsigned int sent = 0;
unsigned int answered = 0;
unsigned int received = 0;

/***************************************************************************
 *  sc_main
 ***************************************************************************/
int sc_main(int argc, char *argv[]) {
	int REPT = 100000;
	int ret, my_rank, comm_sz, tag;
	expio_log_init();

	ret = MPI_Init(&argc, &argv);
	if (ret) EXPIO_LOG_CRIT("MPI code: %d", ret);
	ret = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	if (ret) EXPIO_LOG_CRIT("MPI code: %d", ret);
	ret = MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	if (ret) EXPIO_LOG_CRIT("MPI code: %d", ret);
	EXPIO_LOG_INFO("MPI Rank:%d World:%d", my_rank, comm_sz);

	if (comm_sz < 2) {
		EXPIO_LOG_CRIT("Required at least 2 processes");
	}

	if (argc >= 2) {
		REPT = atoi(argv[1]);
	} else {
		REPT = 1000;
	}

	tag = 133;
	YAPSC_INIT_VIRTUAL("", my_rank);

	yapsc::proxy_mpi *PW;
	yapsc::remote_mpi *RW;
	consumer *c;
	producer *p;

	if (my_rank == 1) {
		MPI_Recv(&REPT, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		c = new consumer("consumer", REPT);

		RW = new yapsc::remote_mpi("RW", MPI_COMM_WORLD, tag);
		EXPIO_LOG_INFO("Remote Wrapper: %d:%d", my_rank, tag);

		RW->isocket.bind(c->tsocket);
	} else if (my_rank == 0) {
		MPI_Send(&REPT, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		p = new producer("producer", REPT);

		PW = new yapsc::proxy_mpi("PW", MPI_COMM_WORLD, tag+1, 1, tag);
		EXPIO_LOG_INFO("Proxy Wrapper to %d:%d", 0, tag);

		p->isocket.bind(PW->tsocket);
	}

	YAPSC_START();

	if (my_rank == 1) {
		delete(c);
		delete (RW);
	} else if (my_rank == 0) {
		delete(p);
		delete (PW);
	}

	ret = MPI_Finalize();
	if (ret) EXPIO_LOG_CRIT("MPI code: %d", ret);

	google::protobuf::ShutdownProtobufLibrary();
	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
