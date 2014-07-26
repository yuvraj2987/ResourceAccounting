import sys, os

fin = open(sys.argv[1],'r')

os.system("rm -f sched_plot_files/*")
os.system("rm -f sched_plot_files/selected/*")

processLogFiles = {}

for line in fin:
    
     line = line.strip().split()
     process = line[0]
     process = process.replace('/','-')
     timestamp = line[3][:-1]
     if process not in processLogFiles:
         processLogFiles[process] = open('sched_plot_files/'+process, 'w')

     processLogFiles[process].write(timestamp+'\n')
       
    
