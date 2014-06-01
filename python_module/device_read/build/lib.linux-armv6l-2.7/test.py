
import sys
import time
import deviceModule

fd = deviceModule.open()

print "Returned fd from open ", fd
ndevices = 0

while ndevices == 0:
  ndevices = deviceModule.get_device(fd)
  if ndevices < 0:
    print "Error while reading event devices"
    sys.exit(-1)
#While ends
print "Devices found:%d"%(ndevices)
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
    print "%-20s\t%-20s\t%-20hd\t%-20s" % (deviceName, openTime, pid, latency)
    print "---------------------------------------------------"
    time.sleep(15)
  #End of for
#End of while

print "-------------------------"
