#ifndef _YAPSC_H
#define _YAPSC_H

#include <map>
#include <tlm>

typedef tlm::tlm_initiator_socket<32, tlm::tlm_base_protocol_types>
    yapsc_initiator_socket;
typedef std::map<std::string, yapsc_initiator_socket &> yapsc_initiator_map;
typedef std::pair<std::string, yapsc_initiator_socket &> yapsc_initiator_pair;

typedef tlm::tlm_target_socket<32, tlm::tlm_base_protocol_types>
    yapsc_target_socket;
typedef std::map<std::string, yapsc_target_socket &> yapsc_target_map;
typedef std::pair<std::string, yapsc_target_socket &> yapsc_target_pair;

typedef void (*yapsc_before_simulation_cb)();
typedef void (*yapsc_success_simulation_cb)();
typedef void (*yapsc_error_simulation_cb)();

#if __has_include("yapsc_core.h") && !defined(YAPSC_DISABLED)
#include "yapsc_core.h"
#else

#define YAPSC_MSGID "yapsc"

#define YAPSC_INIT(ARGC, ARGV)                                                 \
	yapsc_before_simulation_cb YAPSC_BEFORE_CB = NULL;                     \
	yapsc_before_simulation_cb YAPSC_SUCCESS_CB = NULL;                    \
	yapsc_before_simulation_cb YAPSC_ERROR_CB = NULL;                      \
	yapsc_initiator_map YAPSC_INITIATORS;                                  \
	yapsc_target_map YAPSC_TARGETS;                                        \
	sc_report_handler::set_actions("/IEEE_Std_1666/deprecated",            \
				       SC_DO_NOTHING);

#define YAPSC_FINALIZE()

#define YAPSC_MODULE_CUR_DOMAIN(MODULE_NAME) true

#define YAPSC_MODULE(MODULE_NAME)                                              \
	SC_REPORT_INFO(YAPSC_MSGID, "Module domain: " MODULE_NAME);            \
	if (YAPSC_MODULE_CUR_DOMAIN(MODULE_NAME))

#define YAPSC_PAYLOAD_EXTENSION(EXT, ENCODE, DECODE)                           \
	SC_REPORT_INFO(YAPSC_MSGID,                                            \
		       "Payload Extension registered: " + typeid(EXT).name());

#define YAPSC_INITIATOR_SOCKET(INIT, TARGET_MODULE, TARGET_NAME)               \
	SC_REPORT_INFO(YAPSC_MSGID,                                            \
		       ("Init Socket to " TARGET_MODULE "." TARGET_NAME));     \
	YAPSC_INITIATORS.insert(                                               \
	    yapsc_initiator_pair(TARGET_MODULE "." TARGET_NAME, INIT));

#define YAPSC_TARGET_SOCKET(TARGET_MODULE, TARGET_NAME, TARGET)                \
	SC_REPORT_INFO(YAPSC_MSGID,                                            \
		       ("Target Socket: " TARGET_MODULE "." TARGET_NAME));     \
	YAPSC_TARGETS.insert(                                                  \
	    yapsc_target_pair(TARGET_MODULE "." TARGET_NAME, TARGET));

#define YAPSC_START()                                                          \
	for (auto &x : YAPSC_INITIATORS) {                                     \
		yapsc_initiator_socket &init = x.second;                       \
		yapsc_target_socket &target = YAPSC_TARGETS.at(x.first);       \
		init.bind(target);                                             \
	}                                                                      \
	try {                                                                  \
		SC_REPORT_INFO(YAPSC_MSGID, "Starting kernal");                \
		if (YAPSC_BEFORE_CB)                                           \
			YAPSC_BEFORE_CB();                                     \
		sc_start();                                                    \
		YAPSC_ERROR_CB = NULL;                                         \
		if (YAPSC_BEFORE_CB)                                           \
			YAPSC_SUCCESS_CB();                                    \
		SC_REPORT_INFO(YAPSC_MSGID, "Exited kernal");                  \
	} catch (std::exception & e) {                                         \
		SC_REPORT_WARNING(YAPSC_MSGID, e.what());                      \
	} catch (...) {                                                        \
		SC_REPORT_ERROR(YAPSC_MSGID,                                   \
				"Caught exception during simulation.");        \
	}                                                                      \
                                                                               \
	if (not sc_end_of_simulation_invoked()) {                              \
		SC_REPORT_ERROR(                                               \
		    YAPSC_MSGID,                                               \
		    "ERROR: Simulation stopped without explicit sc_stop()");   \
		sc_stop();                                                     \
	}                                                                      \
	if (YAPSC_ERROR_CB)                                                    \
		YAPSC_ERROR_CB();                                              \
	SC_REPORT_INFO(YAPSC_MSGID, "The end.");

#define YAPSC_BEFORE_CALLBACK(CB) YAPSC_BEFORE_CB = CB
#define YAPSC_SUCCESS_CALLBACK(CB) YAPSC_SUCCESS_CB = CB
#define YAPSC_ERROR_CALLBACK(CB) YAPSC_ERROR_CB = CB

#define yapsc_syscall_read(fildes, buf, nbytes) ::read(fildes, buf, nbytes)
#define yapsc_syscall_write(fildes, buf, nbytes) ::write(fildes, buf, nbytes)
#define yapsc_syscall_open(pathname, oflags, mode)                             \
	::open(pathname, oflags, mode)
#define yapsc_syscall_close(fildes) ::close(fildes)
#define yapsc_syscall_creat(pathname, mode) ::creat(pathname, mode)
#define yapsc_syscall_lseek(fildes, offset, whence)                            \
	::lseek(fildes, offset, whence)
#define yapsc_syscall_isatty(fildes) ::isatty(fildes)
#define yapsc_syscall_sbrk(increment)                                          \
	({                                                                     \
		static unsigned long int sbrk_addr = 0;                        \
		void *old = (void *)sbrk_addr;                                 \
		sbrk_addr += increment;                                        \
		old;                                                           \
	})
#define yapsc_syscall_exit(status) ::sc_stop()
#define yapsc_syscall_getcwd(buf, nbytes) ::getcwd(buf, nbytes)

#endif

#endif
