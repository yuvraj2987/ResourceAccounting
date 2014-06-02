import threading

#User modules
import timeModule
import deviceModule
import time

def main():
    try:
        print "----- Event Log Read starts -----"
        start_time = time.time()
        logfile = "log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(start_time))+"_eventdevice.log"
        fileObj = open(logfile, "w+")
        fileObj.write("%.6f# # #\n" % start_time)
        fd = deviceModule.open()
        ndevices =0
        while ndevices == 0:
            ndevices = deviceModule.get_device(fd)
            if ndevices < 0:
                print "Error while reading event_log device module"
                sys.exit(-1)
            print "Wait till somone opens gps for reading"
        #End of while
        print "Devices found :", ndevices
        while True:
            print "%-20s\t%-20s\t%-20s\t%-20s" % ("DEVICE_NAME", "OPEN_TIME", "PID", "LATENCY")
            for dev in range(0, ndevices):
                #pName, time_sec, time_usec = deviceModule.set_device_info(fd, dev)
                tuples = deviceModule.set_device_info(fd, dev)
                print "Result from set_device_info:", str(tuples)
                print "Returned value length", len(tuples)
                deviceName = tuples[0]
                openTime = "%ld.%06ld" % (tuples[1], tuples[2])
                #tuples[3] tuples[4] - event generated and event dropped... tty no relation
                pid = tuples[5]
                eventsConsumed = tuples[6]
                latency = "%ld.%06ld" % (tuples[7], tuples[8])
                string  = "%-20s\t%-20s\t%-20hd\t%-20s" % (deviceName, openTime, pid, latency)
                #print "%-20s\t%-20s\t%-20hd\t%-20s" % (deviceName, openTime, pid, latency)
                print string 
                fileObj.write(string)
                print "---------------------------------------------------"
                time.sleep(15)
             #End of for
       #End of while

    except KeyboardInterrupt:
        print "---- Event Log Read Interrupted -----"
    finally:
        fileObj.close()

if __name__ == '__main__':
  main()
