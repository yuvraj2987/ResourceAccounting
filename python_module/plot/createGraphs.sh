#!/bin/bash

# power usage
python power.py *power.log

# cgps sched switch
python schedAvg.py *scheduler.log

# cgps fix delay
python cgpsFixDelay.py

# create graph & open
cp ra.plt /tmp/ra.plt
for f in `ls cgpsFixDelayData*`
do
    echo \"$f\" notitle w lines lw 5 ,\\ >> /tmp/ra.plt
done
cat /tmp/ra.plt | sed '$s/..$//' > /tmp/ra2.plt
gnuplot < /tmp/ra2.plt
rm /tmp/ra*.plt
gnome-open ra.pdf
