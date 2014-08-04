import sys

fin = open(sys.argv[1],'r')
fout = open('socketInfoFormat.txt','w')

startTime = float(fin.readline().split()[0])

for line in fin:
    line = line.strip().split()
    dt = float(line[0])
    pid = line[3]
    fout.write( str(startTime+dt) + '\t' + pid + '\n' )
    
fin.close()
fout.close()
