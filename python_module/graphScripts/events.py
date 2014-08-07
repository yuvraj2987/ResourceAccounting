import sys
import math
import os

# creates a txt file for both cgps and gpsd showing
# the % cpu used during a time period (1 second).
fin = open(sys.argv[1],'r')
fout = open("events.txt","w")

time=0.0
totalCount = 0
prev = 0
name=""
slot=0

eTimes = {}

for wholeline in fin:
    prev=totalCount
    try:
        line = wholeline.strip().split()
        name = line[0]
        time = float(line[1])
        totalCount = int(line[5])
        slot = math.floor(time)
    
    except:
        break

    if slot == 0:
	continue

    try:
        eTimes[slot] += totalCount-prev
    except:
        eTimes[slot] = totalCount-prev

sorted(totalTimes, key=totalTimes.get)

for key in eTimes:
    try:
        avg = eTimes[key]
    except:
        avg = 0.0

    fout.write(str(key) + "\t" + str(avg) + "\n")

fin.close()
fout.close()

