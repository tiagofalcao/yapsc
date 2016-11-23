#include <expio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if __has_include(<mpi.h>) && !defined(MPI_DISABLED)
#include <mpi.h>
#endif

#include "../mpi/yapsc_client_mpi.h"
#include "../tcp/yapsc_client_tcp.h"
#include "../uds/yapsc_client_uds.h"
#include "yapsc_client.h"
#include "yapsc_client_sc.h"
#include "yapsc_client_virtual.h"
#include "yapsc_core.h"

yapsc::client *YAPSC_CLIENT = NULL;

yapsc_domain_t yapsc_domain() { return YAPSC_CLIENT->domain_get(); }

const char *yapsc_name() { return YAPSC_CLIENT->name_get(); }

void YAPSC_INIT_VIRTUAL(const char *name, yapsc_domain_t domain) {
	YAPSC_CLIENT = new yapsc::client_virtual(name, domain);
}

void YAPSC_INIT(int *argc, char ***argv) {

	YAPSC_CLIENT = get_client_tcp(argc, argv);
	if (YAPSC_CLIENT)
		return;

	YAPSC_CLIENT = get_client_uds(argc, argv);
	if (YAPSC_CLIENT)
		return;

	YAPSC_CLIENT = get_client_mpi(argc, argv);
	if (YAPSC_CLIENT)
		return;

	YAPSC_CLIENT = get_client_sc(argc, argv);
	if (YAPSC_CLIENT)
		return;
}

void YAPSC_FINALIZE() {
	EXPIO_LOG_INFO("YAPSC_FINALIZE %d", yapsc_domain());
	delete (YAPSC_CLIENT);
	YAPSC_CLIENT = NULL;
}

bool YAPSC_MODULE_CUR_DOMAIN(const std::string module_name) {
	return YAPSC_CLIENT->module_domain(module_name);
}

void YAPSC_INITIATOR_SOCKET(yapsc_initiator_socket &init,
			    const std::string module_name,
			    const std::string socket_name) {
	EXPIO_LOG_INFO("Init Socket to %s.%s", module_name.c_str(),
		       socket_name.c_str());
	YAPSC_CLIENT->initiator_socket(init, module_name, socket_name);
}

void YAPSC_TARGET_SOCKET(const std::string module_name,
			 const std::string socket_name,
			 yapsc_target_socket &target) {
	EXPIO_LOG_INFO("Target Socket: %s.%s", module_name.c_str(),
		       socket_name.c_str());
	YAPSC_CLIENT->target_socket(module_name, socket_name, target);
}

void YAPSC_START() { YAPSC_CLIENT->start(); }

void YAPSC_BEFORE_CALLBACK(yapsc_before_simulation_cb CB) {
	YAPSC_CLIENT->before_simulation = CB;
}
void YAPSC_SUCCESS_CALLBACK(yapsc_success_simulation_cb CB) {
	YAPSC_CLIENT->success_simulation = CB;
}
void YAPSC_ERROR_CALLBACK(yapsc_error_simulation_cb CB) {
	YAPSC_CLIENT->error_simulation = CB;
}

size_t yapsc_syscall_read(int fildes, void *buf, size_t nbytes) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_read(fildes, buf, nbytes);
	return read(fildes, buf, nbytes);
}

size_t yapsc_syscall_write(int fildes, const void *buf, size_t nbytes) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_write(fildes, buf, nbytes);
	return write(fildes, buf, nbytes);
}

int yapsc_syscall_open(const char *pathname, int oflags, mode_t mode) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_open(pathname, oflags, mode);
	return open(pathname, oflags, mode);
}

int yapsc_syscall_close(int fildes) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_close(fildes);
	return close(fildes);
}

int yapsc_syscall_creat(const char *pathname, mode_t mode) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_creat(pathname, mode);
	return creat(pathname, mode);
}

off_t yapsc_syscall_lseek(int fildes, off_t offset, int whence) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_lseek(fildes, offset, whence);
	return lseek(fildes, offset, whence);
}

int yapsc_syscall_isatty(int fildes) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_isatty(fildes);
	return isatty(fildes);
}

void *yapsc_syscall_sbrk(intptr_t increment) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_sbrk(increment);
	return sbrk(increment);
}

void yapsc_syscall_exit(int status) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_exit(status);
	return exit(status);
}

char *yapsc_syscall_getcwd(char *buf, size_t nbytes) {
	if (YAPSC_CLIENT)
		return YAPSC_CLIENT->syscall_getcwd(buf, nbytes);
	return getcwd(buf, nbytes);
}
