#include "main.h"
#include "consumer.h"
#include "producer.h"
#include <expio.h>
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

	srand(time(NULL));

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
	YAPSC_INIT_VIRTUAL("", my_rank);

	if (comm_sz != 2) {
		EXPIO_LOG_CRIT("Required at least 2 processes");
	}

	bool local = false;
	char pname[MPI_MAX_PROCESSOR_NAME];
	int pname_len;
	ret = MPI_Get_processor_name(pname, &pname_len);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);

	if (argc >= 2) {
		REPT = atoi(argv[1]);
	} else {
		REPT = 1000 + rand() % 1000000;
		expio_log_level_global_set((EXPIO_Log_Level)0);
	}
	tag = 133;
	EXPIO_LOG_DBG("MPI tag: %d", tag);

	yapsc::proxy_mpi *PW;
	yapsc::remote_mpi *RW;
	consumer *c;
	producer *p;

	if (my_rank == 0) {
		EXPIO_LOG_INFO("MPI Node: %s %d", pname, my_rank);

		char rname[MPI_MAX_PROCESSOR_NAME];
		MPI_Recv(&rname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 1, 0,
			 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		local = (strcmp(rname, pname) == 0);
		EXPIO_LOG_INFO("MPI Remote Node: %s (%d)", rname, local);

		EXPIO_LOG_DBG("REPT %d: %d", my_rank, REPT);
		MPI_Send(&REPT, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

		EXPIO_LOG_DBG("Barrier %d", my_rank);
		ret = MPI_Barrier(MPI_COMM_WORLD);
		if (ret)
			EXPIO_LOG_CRIT("MPI code: %d", ret);

		p = new producer("producer", REPT);

		PW =
		    new yapsc::proxy_mpi("PW", MPI_COMM_WORLD, tag + 1, 1, tag);
		EXPIO_LOG_INFO("Proxy Wrapper to %d:%d", 0, tag);

		p->isocket.bind(PW->tsocket);
	} else {
		EXPIO_LOG_INFO("MPI Node: %s %d", pname, my_rank);
		MPI_Send(pname, pname_len + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

		EXPIO_LOG_DBG("Getting REPT %d", my_rank);
		MPI_Recv(&REPT, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
			 MPI_STATUS_IGNORE);
		EXPIO_LOG_DBG("REPT %d: %d", my_rank, REPT);

		EXPIO_LOG_DBG("Barrier %d", my_rank);
		ret = MPI_Barrier(MPI_COMM_WORLD);
		if (ret)
			EXPIO_LOG_CRIT("MPI code: %d", ret);

		c = new consumer("consumer", REPT);

		RW = new yapsc::remote_mpi("RW", MPI_COMM_WORLD, tag);
		EXPIO_LOG_INFO("Remote Wrapper: %d:%d", my_rank, tag);

		RW->isocket.bind(c->tsocket);

	}

	expio_stats_t *stats = NULL;
	if (my_rank == 0) {
		EXPIO_LOG_DBG("Local simulation: %d", local);
		stats = expio_stats_tsv_new("benchmarks.tsv", "Latency",
					    local ? "mpi" : "mpi-remote",
					    VERSION, "wait", "", REPT);
		expio_stats_begin(stats);
	}
	sc_start();
	if (my_rank == 0) {
		expio_stats_end(stats);
		if (!expio_stats_write(stats))
			return 1;
		expio_stats_delete(stats);
	}

	if (my_rank == 1) {
		delete (c);
		delete (RW);
	} else if (my_rank == 0) {
		delete (p);
		delete (PW);
	}

	ret = MPI_Finalize();
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);

	google::protobuf::ShutdownProtobufLibrary();
	YAPSC_FINALIZE();
	expio_log_shutdown();
	return 0;
}
