#ifndef _YAPSC_CORE_H
#define _YAPSC_CORE_H

#include "yapsc.h"
#include <stdint.h>

typedef int32_t yapsc_domain_t;
typedef std::map<std::string, yapsc_domain_t> yapsc_module_domain_map;
typedef std::pair<std::string, yapsc_domain_t> yapsc_module_domain_pair;

extern "C" {

yapsc_domain_t yapsc_domain();
const char *yapsc_name();

void YAPSC_INIT_VIRTUAL(const char *name, yapsc_domain_t domain);
void YAPSC_INIT(int *argc, char ***argv);
void YAPSC_FINALIZE();
bool YAPSC_MODULE_CUR_DOMAIN(const std::string module_name);
void YAPSC_INITIATOR_SOCKET(yapsc_initiator_socket &init,
			    const std::string module_name,
			    const std::string socket_name);
void YAPSC_TARGET_SOCKET(const std::string module_name,
			 const std::string socket_name,
			 yapsc_target_socket &target);
void YAPSC_START();

void YAPSC_SYSCALL_EXIT(int status);

void YAPSC_BEFORE_CALLBACK(yapsc_before_simulation_cb CB);
void YAPSC_SUCCESS_CALLBACK(yapsc_success_simulation_cb CB);
void YAPSC_ERROR_CALLBACK(yapsc_error_simulation_cb CB);

size_t yapsc_syscall_read(int fildes, void *buf, size_t nbytes);
size_t yapsc_syscall_write(int fildes, const void *buf, size_t nbytes);
int yapsc_syscall_open(const char *pathname, int oflags, mode_t mode);
int yapsc_syscall_close(int fildes);
int yapsc_syscall_creat(const char *pathname, mode_t mode);
off_t yapsc_syscall_lseek(int fildes, off_t offset, int whence);
int yapsc_syscall_isatty(int fildes);
void *yapsc_syscall_sbrk(intptr_t increment);
void yapsc_syscall_exit(int status);
char *yapsc_syscall_getcwd(char *buf, size_t nbytes);

}

#define YAPSC_MODULE(MODULE_NAME) if (YAPSC_MODULE_CUR_DOMAIN(MODULE_NAME))

#endif
