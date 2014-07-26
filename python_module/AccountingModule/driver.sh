#!/bin/bash

echo "--- Accounting Test starts ----"

echo "Start measurement and server thread"

(sudo python measurement.py)&
sleep 5;
echo "Start gpsd"
(sudo /home/pi/gpsd-3.0/gpsd /dev/ttyUSB0 -N -F /var/run/gpsd.sock)&
sleep 5;

echo "Start cgps application"
(/home/pi/gpsd-3.0/cgps -s)&
sleep 5
#echo "Start device Read"
#(sudo python eventReadModule.py)
