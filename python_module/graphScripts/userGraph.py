import os
import fnmatch
import sys

prev=0
first=0

fin = open(sys.argv[1],'r')

dt = 0
fin.readline() #skip header
fout = open("userInOut.txt", "w")

for line in fin:
    line = line.strip().split()
    dt = float(line[0])
    n = int(line[1])
    if (first==0):
        first=1
        fout.write(str(dt)+" 0\n")

    if (n==1 or n==0):
       if (n!=prev):
           prev=n
           if (n==0):
               fout.write(str(dt)+" 1\n")
               fout.write(str(dt)+" 0\n")
           if (n==1):
               fout.write(str(dt)+" 0\n")
               fout.write(str(dt)+" 1\n")

fref = open("cgpsFixDelayData.txt","r")
for line in fref:
    line=line.strip().split()
    dt = float(line[0])

if (prev==1):
    fout.write(str(dt)+" 1\n")
else:
    fout.write(str(dt)+" 0\n")
