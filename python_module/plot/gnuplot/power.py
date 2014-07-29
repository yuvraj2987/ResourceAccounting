import sys

fin = open(sys.argv[1],'r')
fout = open('powerInfoFormat.txt','w')

startTime = float(fin.readline().split()[0])

for line in fin:
    line = line.strip().split()
    dt = float(line[0])
    fout.write( str(startTime+dt) + '\t' + line[1] + '\n' )
    
fin.close()
fout.close()

# generate average power

fin = open('powerInfoFormat.txt','r')
fout = open('powerAvgInfoFormat.txt','w')

time = []
power = []
for line in fin:
    line = line.strip().split()
    time += [line[0]]
    power += [float(line[1])]

poweravg = []
for x in range(len(power)):
    avg = power[x]
    count = 1
    for i in range(1,100):
        try:
            avg += power[x-i]
            count += 1
        except:
            pass
        try:
            avg += power[x+i]
            count += 1
        except:
            pass
    avg = avg/count
    poweravg += [avg]

for x in range(len(power)):
    fout.write( time[x] + '\t' + str(poweravg[x]) + '\n' )
