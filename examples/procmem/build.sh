#!/bin/bash

ROOT=$(realpath $(dirname $BASH_SOURCE))
pushd ${ROOT} &>>/dev/null

CC="mpic++"

CFLAGS="-O0 -std=c++11 -fPIC -g -Wno-deprecated"
INCLUDES=""
LIBPATH=""
LIBS=""
LINKFLAGS=""

for pkg in systemc archc expio protobuf yapsc sc-memory; do
CFLAGS="${CFLAGS} $(pkg-config --cflags-only-other ${pkg})"
INCLUDES="${INCLUDES} $(pkg-config --cflags-only-I ${pkg})"
LIBPATH="${LIBPATH} $(pkg-config --libs-only-L ${pkg})"
LIBS="${LIBS} $(pkg-config --libs-only-l ${pkg})"
LINKFLAGS="${LINKFLAGS} $(pkg-config --libs-only-other ${pkg})"
done

ACFILES="mips_arch.o mips_arch_ref.o mips.o mips_syscall.o mips_intr_handlers.o"

for proc in wocache 2wcache; do
	pushd mips &>>/dev/null
	acsim mips_${proc}.ac

	for obj in ${ACFILES} ; do
		echo -n "${obj} ... "
		${CC} ${CFLAGS} ${INCLUDES} -o ${obj} -c ${obj%.o}.cpp || exit
		echo " ok "
	done
	echo -n "mips_${proc}_yapsc.a ... "
	ar r mips_${proc}_yapsc.a mips_arch.o mips_arch_ref.o mips.o mips_syscall.o mips_intr_handlers.o || exit
	echo " ok "
	popd &>>/dev/null

	echo -n "platform_${proc}_yapsc.o ... "
	${CC} ${CFLAGS} -DDESC=\"${proc}\" ${INCLUDES} -o platform_${proc}_yapsc.o -c platform.cc || exit
	echo " ok "

	echo -n "platform_${proc}_yapsc.x ... "
	${CC} ${LINKFLAGS} ${LIBPATH} -o platform_${proc}_yapsc.x platform_${proc}_yapsc.o mips/mips_${proc}_yapsc.a ${LIBS} || exit
	echo " ok "

	pushd mips &>>/dev/null
	acsim mips_${proc}.ac

	for obj in ${ACFILES} ; do
		echo -n "${obj} ... "
		${CC} ${CFLAGS} -DYAPSC_DISABLED ${INCLUDES} -o ${obj} -c ${obj%.o}.cpp || exit
		echo " ok "
	done
	echo -n "mips_${proc}.a ... "
	ar r mips_${proc}.a mips_arch.o mips_arch_ref.o mips.o mips_syscall.o mips_intr_handlers.o || exit
	echo " ok "
	popd &>>/dev/null

	echo -n "platform_${proc}.o ... "
	${CC} ${CFLAGS} -DYAPSC_DISABLED -DDESC=\"${proc}\" ${INCLUDES} -o platform_${proc}.o -c platform.cc || exit
	echo " ok "

	echo -n "platform_${proc}.x ... "
	${CC} ${LINKFLAGS} ${LIBPATH} -o platform_${proc}.x platform_${proc}.o mips/mips_${proc}.a ${LIBS} || exit
	echo " ok "

done

popd &>>/dev/null
