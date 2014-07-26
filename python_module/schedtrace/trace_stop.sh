#!/bin/bash


export TPATH="/sys/kernel/debug/tracing"

if [ ! -e $TPATH ]
then
  mount -t debugfs nodev /sys/kernel/debug
fi

echo 0 > $TPATH/tracing_on
echo nop > $TPATH/current_tracer
echo  > $TPATH/set_event

