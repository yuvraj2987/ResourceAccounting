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
	file_open=0
        baseTime = 0
	fin.readline() #skip header
        for line in fin:
            line = line.strip().split()
            dt = float(line[0])
            n = int(line[1])
            if (n==0):
                startTime = baseTime + dt
            if (n==2):
                endTime = baseTime + dt
                if (startTime==0 or endTime==0):
                    break
	        if file_open==0:
		    file_open=1
		    fout = open("cgpsFixDelayData"+str(count)+".txt", "w+")
		    print str(items)+" written to file " + str(count)
                fout.write(str(startTime) + " "+str(count)+"\n")
                fout.write(str(startTime) + " "+str(count+1)+"\n")
                fout.write(str(endTime) + " "+str(count+1)+"\n")
                fout.write(str(endTime) + " "+str(count)+"\n")
                startTime= baseTime + dt

        count += 1
        n=0
        fout.close()
