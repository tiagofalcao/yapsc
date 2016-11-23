#ifndef YAPSC_CLIENT_H
#define YAPSC_CLIENT_H

#include "yapsc_core.h"
#include <stdint.h>

namespace yapsc {

class client {
      public:
	client(const char *name, yapsc_domain_t domain);
	virtual ~client();

	yapsc_domain_t domain_get();
	const char *name_get();

	virtual const char *backend_name() = 0;
	virtual bool module_domain(const std::string module_name) = 0;
	virtual void initiator_socket(yapsc_initiator_socket &init,
				      const std::string module_name,
				      const std::string socket_name) = 0;
	virtual void target_socket(const std::string module_name,
				   const std::string socket_name,
				   yapsc_target_socket &target) = 0;
	virtual void start(void) = 0;
	virtual bool can_continue();

	virtual size_t syscall_read(int fildes, void *buf, size_t nbytes);
	virtual size_t syscall_write(int fildes, const void *buf,
				     size_t nbytes);
	virtual int syscall_open(const char *pathname, int oflags, mode_t mode);
	virtual int syscall_close(int fildes);
	virtual int syscall_creat(const char *pathname, mode_t mode);
	virtual off_t syscall_lseek(int fildes, off_t offset, int whence);
	virtual int syscall_isatty(int fildes);
	virtual void *syscall_sbrk(intptr_t increment);
	virtual void syscall_exit(int status);
	virtual char *syscall_getcwd(char *buf, size_t nbytes);

	yapsc_before_simulation_cb before_simulation = NULL;
	yapsc_success_simulation_cb success_simulation = NULL;
	yapsc_error_simulation_cb error_simulation = NULL;

      protected:
	yapsc_domain_t domain;
	char *name;
};
};
extern yapsc::client *YAPSC_CLIENT;

#endif
