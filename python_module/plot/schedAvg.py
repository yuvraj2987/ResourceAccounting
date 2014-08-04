import sys
import math
import os

# creates a txt file for both cgps and gpsd showing
# the % cpu used during a time period (1 second).

fin = open(sys.argv[1],'r')
fout = open("cgpsSchedAvg2.txt","w")
fout2 = open("gpsdSchedAvg2.txt","w")

totalCount = 0
cgpsCount = 0
gpsdCount = 0

cgpsTimes = {}
gpsdTimes = {}
totalTimes = {}

for line in fin:

    line = line.strip().split()
    name = line[0]
    time = math.floor(float(line[3][:-1]))
    if name.startswith("cgps"):
        try:
            cgpsTimes[time] += 1.0
        except:
            cgpsTimes[time] = 1.0
    if name.startswith("gpsd"):
        try:
            gpsdTimes[time] += 1.0
        except:
            gpsdTimes[time] = 1.0
    try:
        totalTimes[time] += 1.0
    except:
        totalTimes[time] = 1.0

sorted(totalTimes, key=totalTimes.get)
for key in totalTimes:
    try:
        avg = cgpsTimes[key]/totalTimes[key]
    except:
        avg = 0.0
    try:
        avg2 = gpsdTimes[key]/totalTimes[key]
    except:
        avg2 = 0.0
    #fout.write(str(key) + "\t" + str((avg*1000000) + 100000) + "\n")
    fout.write(str(key) + "\t" + str(avg) + "\n")
    fout2.write(str(key) + "\t" + str(avg2) + "\n")
    #print str(int(key)) + ":\t" + str(int(cgpsTimes[key])) + " / " + str(int(totalTimes[key])) + "\t= " + str(avg)

fin.close()
fout.close()
fout2.close()

os.system("sort cgpsSchedAvg2.txt > cgpsSchedAvg.txt && rm cgpsSchedAvg2.txt")
os.system("sort gpsdSchedAvg2.txt > gpsdSchedAvg.txt && rm gpsdSchedAvg2.txt")
