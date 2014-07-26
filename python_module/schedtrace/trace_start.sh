#!/bin/bash


export TPATH="/sys/kernel/debug/tracing"

if [ ! -e $TPATH ]
then
  mount -t debugfs nodev /sys/kernel/debug
fi

echo sched_switch > $TPATH/set_event
echo 1 > $TPATH/tracing_on

