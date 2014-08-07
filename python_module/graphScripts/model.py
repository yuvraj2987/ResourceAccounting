import os
import fnmatch

# creates a text file for entire cgps processes. This
# file contins data about when cgps are running
# and depending on this times we model the energy consumption
# This is a certain format to the output
# because it is used in gnuplot to create a plot. 

count = 0

slotsStart=[]
slotsEnd=[]

for root, dirs, files in os.walk("."):
    for items in fnmatch.filter(reversed(files), "*cgps.log"):
	
        fin = open(items,'r')
	fin.readline() #skip header

        startTime = 0
        endTime = 0
        dtime = 0
	n=0
        for line in fin:
            line = line.strip().split()
            dtime = float(line[0])
            n = int(line[1])
            if (n==0):
                startTime = dtime
            if (n==2):
                endTime = dtime
	    if (n==3):
		endTime = dtime
                

#	print "file: "+items+" \t\tstart time: "+str(startTime) + "   \t 	end time: "+str(endTime)

	if (startTime==0 or endTime==0 or endTime<=startTime):
            continue
	
	merged=0
	for i in range (count):
	    if startTime<slotsEnd[i] and endTime>slotsStart[i]:
#		print "merging with ", str(i)
		merged=1
		if startTime<slotsStart[i]:
		    slotsStart[i]=startTime
		if endTime>slotsEnd[i]:
		    slotsEnd[i]=endTime
		
	if merged == 0: #add a separate slot
	    slotsStart.append(startTime)
	    slotsEnd.append(endTime)
#	    print "new slot added: ",slotsStart[len(slotsStart)-1],"-",slotsEnd[len(slotsEnd)-1]
#	    print ""
	    count += 1

timeref=open("cgpsFixDelayData.txt","r")
line=timeref.readline()
timepoint=float(line.strip().split()[0])
line=timeref.readline()
finishingtime=float(line.strip().split()[0])

fout = open("powerModel.txt", "w")
fenergy = open("energy_model.txt","w")
timeStep=0.5
energy=0
while (timepoint<finishingtime):
    value = 120000
    for t in range (count):
	if timepoint>slotsStart[t] and timepoint<slotsEnd[t]:
	    value=170000
    fout.write(str(timepoint)+"\t"+str(value)+"\n")
    energy+=value*timeStep
    fenergy.write(str(timepoint)+"\t"+str(energy)+"\n")
    timepoint+=timeStep

fout.close()
fenergy.close()
