#!/bin/bash

for i in `seq 1 10`;
do
    /home/pi/ResourceAccounting/gpsd-3.0/cgps &
done
