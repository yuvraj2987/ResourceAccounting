
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
  for dev in range(0, ndevices):
    pName, time_sec, time_usec = deviceModule.set_device_info(fd, dev)
    if pName is None:
      break;
    #End of if
    time_start = "%ld.%ld"%(time_sec, time_usec)
    print "Process Name:%s Time: %s"%(pName, time_start)
    print "---------------------------------------------------"
    time.sleep(15)
  #End of for
#End of while

print "-------------------------"
