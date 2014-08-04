import sys, os
from optparse import OptionParser
import time

start_time = time.time()
filename="../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(start_time))+"_scheduler.log"
flog=open (filename, "w")

filename="../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(start_time))+"_scheduler_errorrs.log"
#errors = open(filename,'w')
 
last_time = 0
num_err=0 
my_str=""


print "mounting ftrace"
os.system("./trace_mount.sh")

print "stop tracing"
os.system("./trace_stop.sh")

print "flushing ftrace ring buffer"
os.system("./trace_flush.sh")

print "starting scheduler switch tracing"
os.system("./trace_start.sh")

print "recording and logging every 3 seconds..."


while (1):
    os.system("cat /sys/kernel/debug/tracing/trace > /tmp/run_tmp.tmp")
    fread = open("/tmp/run_tmp.tmp", "r")
    for line in fread:
        if not line.startswith('#'):
            splt=line.strip().split()
            timestamp = splt[3][:-1]
            try:
                float(timestamp)
                if timestamp>last_time:
                    flog.write(line)
                    last_time=timestamp
            except ValueError:
                my_str=line.replace("rs:main Q:Reg", "rs:main_Q:Reg")
                splt=my_str.strip().split()
                timestamp = splt[3][:-1]
                try:
                    float (timestamp)
                    if timestamp>last_time:
                        flog.write(my_str)
                        last_time=timestamp
                        print "on line modified to fit the pattern\n\t"
                        print line
                        print my_str
                except:
                    if (num_err==0):
                        errors = open(filename,'w')
                    num_err=num_err+1
                    errors.write(line)
                    print "line skipped. a total of "+str(num_err) +" lines not processed, and logged in "+filename+" (some might be duplicates):\n\t"
                    print line

    flog.flush()
    time.sleep(3.0)


