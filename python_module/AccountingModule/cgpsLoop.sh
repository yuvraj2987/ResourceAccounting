#!/bin/bash

for i in `seq 1 15`;
do
    /home/pi/gpsd-3.0/cgps -s &
done
