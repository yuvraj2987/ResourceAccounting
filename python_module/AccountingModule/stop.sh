#!/bin/bash

echo "Stop accounting experiment"

echo "Stop gpsd"
(sudo killall gpsd)&

echo "Stop python codes"
(sudo killall python)&
