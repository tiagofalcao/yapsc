#include "yapsc_client.h"
#include <expio.h>
#include <systemc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using yapsc::client;

client::client(const char *name, yapsc_domain_t domain) {
	this->domain = domain;
	this->name = strdup(name);
}

client::~client() {
	free(name);
	name = NULL;
}

yapsc_domain_t client::domain_get() { return domain; }

const char *client::name_get() { return name; }

bool client::can_continue() {
	return true;
}

size_t client::syscall_read(int fildes, void *buf, size_t nbytes) {
	return read(fildes, buf, nbytes);
}

size_t client::syscall_write(int fildes, const void *buf, size_t nbytes) {
	return write(fildes, buf, nbytes);
}

int client::syscall_open(const char *pathname, int oflags, mode_t mode) {
	return open(pathname, oflags, mode);
}

int client::syscall_close(int fildes) {
	return close(fildes);
}

int client::syscall_creat(const char *pathname, mode_t mode) {
	return creat(pathname, mode);
}

off_t client::syscall_lseek(int fildes, off_t offset, int whence) {
	return lseek(fildes, offset, whence);
}

int client::syscall_isatty(int fildes) {
	return isatty(fildes);
}

void *client::syscall_sbrk(intptr_t increment) {
	return sbrk(increment);
}

void client::syscall_exit(int status) {
	sc_stop();
}

char *client::syscall_getcwd(char *buf, size_t nbytes) {
	return getcwd(buf, nbytes);
}
