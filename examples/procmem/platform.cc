#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <systemc.h>
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "expio.h"
#include "mips/mips.H"
#include "tlm_memory_nb.h"
#include "yapsc.h"

#define MEM_SIZE 512 * 1024 * 1024

const char *project_name = "platform.noc.at";
const char *project_file = "";
const char *archc_version = "";
const char *archc_options = "";

unsigned int ac_start_addr = 0;
unsigned int ac_heap_ptr = 0;
unsigned int dec_cache_size = 0;

void load_elf(const char *filename, unsigned int ac_word_size,
	      bool ac_mt_endian, sc_memory::tlm_memory_nb *mem) {
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	int fd;

	if (!filename || ((fd = open(filename, 0)) < 0)) {
		EXPIO_LOG_CRIT("Openning application file '%s': %s", filename,
			       strerror(errno));
	}

	// Test if it's an ELF file
	if ((read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) || // read header
	    (strncmp((char *)ehdr.e_ident, ELFMAG, 4) !=
	     0) || // test magic number
	    0) {
		close(fd);
		EXPIO_LOG_CRIT("File '%s' is not an elf: %s", filename,
			       strerror(errno));
	}

	// Set start address
	EXPIO_LOG_INFO("ac_start_addr %d",
		       convert_endian(4, ehdr.e_entry, ac_mt_endian));
	ac_start_addr = convert_endian(4, ehdr.e_entry, ac_mt_endian);
	if (ac_start_addr > mem->size()) {
		close(fd);
		EXPIO_LOG_CRIT("The start address of the application is beyond "
			       "model memory.");
	}

	if (convert_endian(2, ehdr.e_type, ac_mt_endian) == ET_EXEC) {
		EXPIO_LOG_INFO("Reading ELF application file: %s", filename);

		for (unsigned int i = 0;
		     i < convert_endian(2, ehdr.e_phnum, ac_mt_endian); i++) {
			// Get program headers and load segments
			lseek(fd,
			      convert_endian(4, ehdr.e_phoff, ac_mt_endian) +
				  convert_endian(2, ehdr.e_phentsize,
						 ac_mt_endian) *
				      i,
			      SEEK_SET);
			if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
				close(fd);
				EXPIO_LOG_CRIT("reading ELF program header");
			}

			if (convert_endian(4, phdr.p_type, ac_mt_endian) ==
			    PT_LOAD) {
				Elf32_Word j;
				Elf32_Addr p_vaddr = convert_endian(
				    4, phdr.p_vaddr, ac_mt_endian);
				Elf32_Word p_memsz = convert_endian(
				    4, phdr.p_memsz, ac_mt_endian);
				Elf32_Word p_filesz = convert_endian(
				    4, phdr.p_filesz, ac_mt_endian);
				Elf32_Off p_offset = convert_endian(
				    4, phdr.p_offset, ac_mt_endian);
				// Error if segment greater then memory
				if (mem->size() < p_vaddr + p_memsz) {
					close(fd);
					EXPIO_LOG_CRIT("not enough memory in "
						       "ArchC model to load "
						       "application.");
				}

				// Set heap to the end of the segment
				EXPIO_LOG_INFO("ac_heap_ptr %d",
					       p_vaddr + p_memsz);
				if (ac_heap_ptr < p_vaddr + p_memsz) {
					ac_heap_ptr = p_vaddr + p_memsz;
				}
				EXPIO_LOG_INFO("dec_cache_size unset %d",
					       p_vaddr + p_memsz);
				if (!dec_cache_size) {
					dec_cache_size = ac_heap_ptr;
				}

				// Load and correct endian
				// if (first_load)
				if (true) {
					lseek(fd, p_offset, SEEK_SET);
					for (j = 0; j < p_filesz;
					     j += ac_word_size) {
						int tmp;
						ssize_t ret_value = read(
						    fd, &tmp, ac_word_size);
						int d = convert_endian(
						    ac_word_size, tmp,
						    ac_mt_endian);
						mem->write(
						    p_vaddr + j +
							mem->start_address(),
						    4, (unsigned char *)&tmp);
					}

					int d = 0;
					for (j = p_vaddr + p_filesz;
					     j <= p_memsz - p_filesz; j++)
						mem->write(p_vaddr + j, 4,
							   (unsigned char *)&d);
				} // if

			} // if

		} // for
	}	 // if

	close(fd);
}

#ifndef DESC
#define DESC ""
#endif

char *backend_name = NULL;
char *simulation_name = NULL;
expio_stats_t *stats = NULL;
void before_simulation() {
	EXPIO_LOG_DBG("Starting simulation");
	stats = expio_stats_tsv_new("results.tsv", "ProcMem", backend_name,
				    "0.1", simulation_name, DESC, 1);
	expio_stats_begin(stats);
}
void success_simulation() {
	expio_stats_end(stats);
	EXPIO_LOG_DBG("Ending simulation");
	if (!expio_stats_write(stats))
		return;
	expio_stats_delete(stats);
}
void error_simulation() { expio_stats_delete(stats); }

int sc_main(int argc, char *argv[]) {
	backend_name = argv[1];
	simulation_name = argv[2];

	int entry_point = atoi(argv[3]);
	char *binary = argv[4];
	char **binary_argv = &(argv[4]);
	int binary_argc = argc - 4;

	fprintf(stderr, "Binary: %s\n", binary);
	fprintf(stderr, "Args: ");
	for (unsigned int i = 0; i < binary_argc; i++)
		fprintf(stderr, "\"%s\" ", binary_argv[i]);
	fputc('\n', stderr);
	if (!binary)
		exit(1);

	expio_log_init();
	argc = 0;
	argv = NULL;
	YAPSC_INIT(&argc, &argv);

	YAPSC_BEFORE_CALLBACK(before_simulation);
	YAPSC_SUCCESS_CALLBACK(success_simulation);
	YAPSC_ERROR_CALLBACK(success_simulation);

	sc_memory::tlm_memory_nb *mem = NULL;
	YAPSC_MODULE("MEM") {
		mem = new sc_memory::tlm_memory_nb("MEM", 0, MEM_SIZE - 1);

		load_elf(binary, 4, false, mem);
		EXPIO_LOG_INFO(">>> ac_heap_ptr %d", ac_heap_ptr);
		EXPIO_LOG_INFO(">>> dec_cache_size %d", dec_cache_size);

		YAPSC_TARGET_SOCKET("MEM", "socket", mem->socket);
	}

	mips *proc_mips = NULL;
	YAPSC_MODULE("Core") {
		mips *proc_mips = new mips("Core");
		proc_mips->ISA.RB[29] = MEM_SIZE;
		proc_mips->ac_heap_ptr = 524288;
		proc_mips->dec_cache_size = proc_mips->ac_heap_ptr;
		proc_mips->ac_start_addr = entry_point;
		EXPIO_LOG_INFO("proc_mips->ac_start_addr 0x%x",
			       proc_mips->ac_start_addr);
		proc_mips->init();

		void *ptr = yapsc_syscall_sbrk(proc_mips->ac_heap_ptr);
		EXPIO_LOG_INFO("proc_mips first addr %p", ptr);

		YAPSC_INITIATOR_SOCKET(proc_mips->MEM.LOCAL_init_socket, "MEM",
				       "socket");
	}

	YAPSC_START();

	YAPSC_MODULE("Core") { delete (proc_mips); }
	YAPSC_MODULE("MEM") { delete (mem); }
	YAPSC_FINALIZE();
	return 0;
}
