/**
 * @file      mips_syscall.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:52 -0300
 * 
 * @brief     The ArchC MIPS-I functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "mips_syscall.H"
#include <expio.h>
#include <yapsc.h>

// 'using namespace' statement to allow access to all
// mips-specific datatypes
using namespace mips_parms;
unsigned procNumber = 0;

void mips_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i++, addr++) {
    buf[i] = DATA_PORT->read_byte(addr);
  }
}

void mips_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i++, addr++) {
    DATA_PORT->write_byte(addr, buf[i]);
    EXPIO_LOG_DBG("DATA_PORT[%d]=%d", addr, buf[i]);

  }
}

void mips_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i+=4, addr+=4) {
    DATA_PORT->write(addr, *(unsigned int *) &buf[i]);
    EXPIO_LOG_DBG("DATA_PORT_no[%d]=%d", addr, buf[i]);
  }
}

int mips_syscall::get_int(int argn)
{
  return RB[4+argn];
}

void mips_syscall::set_int(int argn, int val)
{
  RB[2+argn] = val;
}

void mips_syscall::return_from_syscall()
{
  ac_pc = RB[31];
  npc = ac_pc + 4;
}

void mips_syscall::set_prog_args(int argc, char **argv)
{


  int i, j, base;

  unsigned int ac_argv[30];
  char ac_argstr[512];

  base = AC_RAM_END - 512 - procNumber * 64 * 1024;
  for (i=0, j=0; i<argc; i++) {
    int len = strlen(argv[i]) + 1;
    ac_argv[i] = base + j;
    memcpy(&ac_argstr[j], argv[i], len);
    j += len;
  }

  RB[4] = base;
  set_buffer(0, (unsigned char*) ac_argstr, 512);   //$25 = $29(sp) - 4 (set_buffer adds 4)
  

  RB[4] = base - 120;
  set_buffer_noinvert(0, (unsigned char*) ac_argv, 120);

  //RB[4] = AC_RAM_END-512-128;

  //Set %o0 to the argument count
  RB[4] = argc;

  //Set %o1 to the string pointers
  RB[5] = base - 120;

  procNumber ++;
}

void mips_syscall::open()
{
  DEBUG_SYSCALL("open");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int flags = get_int(1); correct_flags(&flags);
  int mode = get_int(2);
  int ret;
  ret = yapsc_syscall_open((char*)pathname, flags, mode);
  EXPIO_LOG_DBG("open\t%s %d %d = %d",pathname, flags, mode, ret);
  set_int(0, ret);
  return_from_syscall();
}

void mips_syscall::creat()
{
  DEBUG_SYSCALL("creat");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int mode = get_int(1);
  int ret;
    ret = yapsc_syscall_creat((char*)pathname, mode);
  EXPIO_LOG_DBG("creat\t%s %d = %d",pathname, mode, ret);
  set_int(0, ret);
  return_from_syscall();
}

void mips_syscall::close()
{
  DEBUG_SYSCALL("close");
  int fd = get_int(0);
  int ret;
  // Silently ignore attempts to close standard streams (newlib may try to do so when exiting)
  if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
    ret = 0;
  else {
	  ret = yapsc_syscall_close(fd);
  }
  EXPIO_LOG_DBG("close\t%d = %d", fd, ret);
  set_int(0, ret);
  return_from_syscall();
}

void mips_syscall::read()
{
  DEBUG_SYSCALL("read");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  int ret;
  ret = yapsc_syscall_read(fd, buf, count);
  EXPIO_LOG_DBG("read\t%d %u = %d %s", fd, count, ret, buf);
  set_buffer(1, buf, ret);
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

void mips_syscall::write()
{
  DEBUG_SYSCALL("write");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  get_buffer(1, buf, count);
  int ret;
  ret = yapsc_syscall_write(fd, buf, count);
  EXPIO_LOG_DBG("write\t%d %u %s = %d", fd, count, buf, ret);
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

void mips_syscall::lseek()
{
  DEBUG_SYSCALL("lseek");
  int fd = get_int(0);
  int offset = get_int(1);
  int whence = get_int(2);
  int ret;
  ret = yapsc_syscall_lseek(fd, offset, whence);
  EXPIO_LOG_DBG("lseek\t%d %d %d = %d", fd, offset, whence, ret);
  set_int(0, ret);
  return_from_syscall();
}


void mips_syscall::isatty()
{
  DEBUG_SYSCALL("isatty");
  int desc = get_int(0);
  int ret;
  ret = yapsc_syscall_isatty(desc);
  set_int(0, ret);
  EXPIO_LOG_DBG("isatty\t%d = %d", desc, ret);
  return_from_syscall();
}

void mips_syscall::sbrk()
{
  DEBUG_SYSCALL("sbrk");
  unsigned int increment = get_int(0);
  void *ret;

  ret = yapsc_syscall_sbrk(increment);

  set_int(0, *((int*)(&ret))); //FIXME
  EXPIO_LOG_DBG("sbrk.\t + %u = %d", increment, ret);
  return_from_syscall();
}


void mips_syscall::_exit()
{
  DEBUG_SYSCALL("_exit");
  int ac_exit_status = get_int(0);

  EXPIO_LOG_DBG("exit\t%d", ac_exit_status);
  yapsc_syscall_exit(ac_exit_status);

#ifdef USE_GDB
  if (get_gdbstub()) (get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */

  stop(ac_exit_status);
}

void mips_syscall::getcwd()
{
  DEBUG_SYSCALL("getcwd");
  unsigned size = get_int(1);
  char *buf = (char*) malloc(size);
  char *ret;
  ret = yapsc_syscall_getcwd(buf, size);
  set_buffer(0, (unsigned char*) buf, size); //FIXME
  set_int(0, *((int*)(&ret))); //FIXME
  EXPIO_LOG_DBG("getcwd\t%d = %l %s", size, ret, buf);
  return_from_syscall();
  free(buf);
}

void mips_syscall::getpagesize()
{
  DEBUG_SYSCALL("getpagesize");

  //FIXME: page size changes depending the architecture
  int ret = sysconf(_SC_PAGE_SIZE);
  EXPIO_LOG_DBG("getpagesize\t= %d", ret);

  set_int(0, ret);
  return_from_syscall();
}



