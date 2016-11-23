#!/bin/bash
CUR=$PWD
ROOT=$(realpath $(dirname $BASH_SOURCE))
echo "Running ${ROOT}"
pushd $ROOT &>>/dev/null

if [ $# -lt 3 ]; then
	echo "Missing arguments: $# < 4"
	echo "Usage: ./run.sh <protocol> <binary> <domains> [name]"
	exit 1
fi

NAME=`tr -dc A-Za-z0-9 < /dev/urandom | head -c24`
if [ $# -gt 3 ]; then
	NAME="$4"
fi
echo "$1:${NAME} $$"

trap 'kill $(jobs -p)' SIGINT SIGTERM
for DOMAIN in $(seq 1 $3); do
	export YAPSC_DOMAIN="$1:${NAME}:${DOMAIN}"
	${CUR}/$2 &>/tmp/yapsc_${NAME}.${DOMAIN}.log &
	echo "${YAPSC_DOMAIN} $! /tmp/yapsc_${NAME}.${DOMAIN}.log"
done

export YAPSC_DOMAIN="$1:${NAME}:0"
${CUR}/$2 &>/tmp/yapsc_${NAME}.0.log &
CTR=$!
echo "${YAPSC_DOMAIN} $! /tmp/yapsc_${NAME}.0.log"
wait $CTR
RET=$?

JOBS=$(jobs -p)
if [ -n "$JOBS" ]; then
	for job in $JOBS; do
		kill $job >>/dev/null
		if [ $? -eq 0 ];then
			RET = 1
			echo "Job ${job} killed"
		fi
	done
fi
popd &>>/dev/null
exit ${RET}
