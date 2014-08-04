import os
import fnmatch

# creates a text file for each cgps process. This
# file contins data about when cgps asked for a gps
# location and then time when it received the response
# from the gpsd. The is a certain format to the output
# because it is used in gnuplot to create a rectangle 
# showing the timer period of when the cgps was waiting
# for a response.

count = 0

for root, dirs, files in os.walk("."):
    for items in fnmatch.filter(files, "*cgps.log"):

        fin = open(items,'r')
        startTime = 0
        endTime = 0
        dt = 0
        baseTime = float(fin.readline().split()[0])

        for line in fin:
            line = line.strip().split()
            dt = float(line[0])
            n = int(line[1])
            if (n==0):
                startTime = baseTime + dt
            if (n==2):
                endTime = baseTime + dt
                break

        if (startTime==0 or endTime==0):
            continue

        fout = open("cgpsFixDelayData"+str(count)+".txt", "w")
        #fout.write(str(startTime) + " 100000\n")
        #fout.write(str(startTime) + " 150000\n")
        #fout.write(str(endTime) + " 150000\n")
        #fout.write(str(endTime) + " 100000\n")
        fout.write(str(startTime) + " 0\n")
        fout.write(str(startTime) + " 1\n")
        fout.write(str(endTime) + " 1\n")
        fout.write(str(endTime) + " 0\n")
        count += 1
        
