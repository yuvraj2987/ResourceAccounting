#!/bin/bash


export TPATH="/sys/kernel/debug/tracing"

if [ ! -e $TPATH ]
then
  mount -t debugfs nodev /sys/kernel/debug
fi

cat $TPATH/per_cpu/cpu0/stats

