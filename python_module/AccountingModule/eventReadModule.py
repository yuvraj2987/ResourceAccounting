import threading

#User modules
import timeModule
import deviceModule
import time
class EventLogThread(threading.Thread):
  def __init__(self, start_time):
    threading.Thread.__init__(self)
    self.start_time = start_time
    self._stop = threading.Event()
  
  def stop(self):
    self._stop.set()

  def _isStopped(self):
    return self._stop.isSet()

  def run(self):
    print "----- Event Log Read starts -----"
    logfile = "log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(self.start_time))+"_eventdevice.log"
    fileObj = open(logfile, "w+")
    fileObj.write("%.6f# # #\n" % self.start_time)
    deviceFd = deviceModule.open()
    print "DeviceFd is: ", deviceFd
    fileObj.write("Device fd is %d\n"%(deviceFd))
    if(deviceFd is None):
      #print "Failed to Open the device"
      fileObj.write("Error: Failed to open /dev/event_log_dev device\n")
      fileObj.close()
    else:
      #print "Device Fd is valid"
      numDevices = 0
      while ((numDevices == 0) and (not self._isStopped())):
        numDevices = deviceModule.get_device(deviceFd)
        #print "Number of devices found:", numDevices
        fileObj.write("Number of devices found: %d\n"%(numDevices))
        if(numDevices < 0):
          #print "Error: numDevics < 0"
          fileObj.write("Error: while reading event devices\n")
          fileObj.close()
          break
      #End of devices found while
      #print "Out of first while"
      
      if not fileObj.closed:
        #print "%-20s\t%-20s\t%-20s\t%-20s" % ("DEVICE_NAME", "OPEN_TIME", "PID", "LATENCY")
        fileObj.write("%-20s\t%-20s\t%-20s\t%-20s\n" % ("DEVICE_NAME", "OPEN_TIME", "PID", "LATENCY"))
        while not self._isStopped():
          for dev in range(0, numDevices):
            tuple = deviceModule.set_device_info(deviceFd, dev)
            deviceName = tuple[0]
            openTime = "%ld.%06ld" % (tuple[1], tuple[2])
            pid = tuple[5]
            eventsConsumed = tuple[6]
            latency = "%ld.%06ld" % (tuple[7], tuple[8])
            #print "%-20s\t%-20s\t%-20hd\t%-20s" % (deviceName, openTime, pid, latency)
            fileObj.write("%-20s\t%-20s\t%-20hd\t%-20s\n" % (deviceName, openTime, pid, latency))
            fileObj.write("--------------------------------------------------------------------\n");
            time.sleep(5)
          #end of for loop
        #end of while loop
        #print "--- EventDeviceLog Thread interrupted ----"
        fileObj.close()
      # end of file.closed if
    #run method ends

def main():
  try:
    print "---- Test Event Log Read Thread ----"
    startTime = time.time()
    thread1 = EventLogThread(startTime)
    thread1.start()
    while True:
      print "Sleep for few seconds"
      time.sleep(15)
    #end of while true
  except KeyboardInterrupt:
   print "Interrupt event log read thread"
   thread1.stop()
   thread1.join()
   print "--- Test complete ---"


if __name__ == '__main__':
  main()
