#ifndef YAPSC_CLIENT_SERIAL_H
#define YAPSC_CLIENT_SERIAL_H

#include "yapsc_core.h"
#include "yapsc_client.h"
#include <stdint.h>

namespace yapsc {

class client_serial : public yapsc::client {
      public:
	client_serial(const char *name, yapsc_domain_t domain);
	virtual ~client_serial();

	virtual const char *backend_name() = 0;
	virtual bool module_domain(const std::string module_name);
	virtual void initiator_socket(yapsc_initiator_socket &init,
				      const std::string module_name,
				      const std::string socket_name);
	virtual void target_socket(const std::string module_name,
				   const std::string socket_name,
				   yapsc_target_socket &target);
	virtual bool can_continue();
	virtual void start(void);

      protected:
	yapsc_module_domain_map modules;
	yapsc_initiator_map local_initiators;
	yapsc_initiator_map initiators;
	yapsc_target_map targets;
	virtual void get_modules(void);

	virtual void bind(void) = 0;
	virtual void send_string(std::string *str) = 0;
	virtual std::string *recv_string(bool blocking) = 0;

	uint32_t buffer_sz = 0;
	char *buffer = NULL;

	bool ended;
	virtual void action_barrier();
	virtual void action_end(int status);

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
};
};

#endif
