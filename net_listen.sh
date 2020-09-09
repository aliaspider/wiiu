#!/bin/bash

if [ -z "$PC_DEVELOPMENT_TCP_PORT" ]; then
  PC_DEVELOPMENT_TCP_PORT=4405
fi

log_file=$1
if [ -z $1 ] ; then
  log_file=log.txt
fi

exit_listen_loop=0

trap 'exit_listen_loop=1' SIGINT

rm $log_file

while [ $exit_listen_loop -eq 0 ]; do
  echo ========= `date` ========= | tee -a $log_file
  netcat -p $PC_DEVELOPMENT_TCP_PORT -l | tee -a $log_file
done
