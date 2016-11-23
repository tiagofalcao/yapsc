#include "yapsc_connection_mpi.h"
#include <expio.h>

void yapsc::send_string_mpi(MPI_Comm comm, int domain, int tag,
			    const std::string *str) {
	int ret;
	uint32_t len = str->length();
	if (len == 0) {
		EXPIO_LOG_ERR("Can not send a empty message");
		return;
	}

	EXPIO_LOG_DBG("Sending message %p with %d to %d:%d", str->c_str(), len, domain, tag);
	ret = MPI_Send(str->c_str(), len, MPI_CHAR, domain, tag, comm);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);
	EXPIO_LOG_DBG("Sent message with %d to %d:%d", len, domain, tag);
}

std::string *yapsc::recv_string_mpi(MPI_Comm comm, int domain, int tag,
				    bool blocking) {
	int ret, len;
	MPI_Status status;

	if (blocking) {
		ret = MPI_Probe(domain, tag, comm, &status);
	} else {
		ret = MPI_Iprobe(domain, tag, comm, &len, &status);
		if (!len) {
			return NULL;
		}
	}

	ret = MPI_Get_count(&status, MPI_CHAR, &len);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);

	char *buf = (char *)malloc(len * sizeof(char));

	EXPIO_LOG_DBG("Receiving message with %d from %d:%d", len,
		      status.MPI_SOURCE, status.MPI_TAG);
	ret = MPI_Recv(buf, len, MPI_CHAR, domain, tag, comm, &status);
	if (ret)
		EXPIO_LOG_CRIT("MPI code: %d", ret);

	EXPIO_LOG_DBG("Received message with %d from %d:%d", len,
		      status.MPI_SOURCE, status.MPI_TAG);
	std::string *str = new std::string(buf, len);
	free(buf);
	return str;
}
