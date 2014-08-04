import sys
import math
import os

# creates a txt file for both cgps and gpsd showing
# the % cpu used during a time period (1 second).
fin = open(sys.argv[1],'r')
fout = open("cgpsSchedAvg2.txt","w")
fout2 = open("gpsdSchedAvg2.txt","w")

time=0.0
totalCount = 0
cgpsCount = 0
gpsdCount = 0
name=""
slot=0

cgpsTimes = {}
gpsdTimes = {}
totalTimes = {}

for wholeline in fin:
    prevtime=time
    try:
        line = wholeline.strip().split()
        name = line[0]
        time = float(line[3][:-1])
        slot = math.floor(prevtime)
    
    except:
        break

    if slot == 0:
	continue

    if name.startswith("cgps"):
        try:
            cgpsTimes[slot] += time - prevtime
        except:
            cgpsTimes[slot] = time - prevtime

        if cgpsTimes[slot] > 1:
            cgpsTimes[slot+1]=cgpsTimes[slot]-1
            cgpsTimes[slot]=1
            totalTimes[slot]=1
            totalTimes[slot+1]=time-prevtime -1
            

    if name.startswith("gpsd"):
        try:
            gpsdTimes[slot] += time - prevtime
        except:
            gpsdTimes[slot] = time - prevtime

        if gpsdTimes[slot] > 1:
            gpsdTimes[slot]=1

    try:
        totalTimes[slot] += time - prevtime
    except:
        totalTimes[slot] = time - prevtime
        
    if totalTimes[slot] > 1:
        totalTimes[slot]=1

sorted(totalTimes, key=totalTimes.get)

for key in totalTimes:
    try:
        avg = cgpsTimes[key]
    except:
        avg = 0.0
    try:
        avg2 = gpsdTimes[key]/totalTimes[key]
    except:
        avg2 = 0.0

    fout.write(str(key) + "\t" + str(avg) + "\n")
    fout2.write(str(key) + "\t" + str(avg2) + "\n")

fin.close()
fout.close()
fout2.close()

os.system("sort cgpsSchedAvg2.txt > cgpsSchedAvg.txt && rm cgpsSchedAvg2.txt")
os.system("sort gpsdSchedAvg2.txt > gpsdSchedAvg.txt && rm gpsdSchedAvg2.txt")
