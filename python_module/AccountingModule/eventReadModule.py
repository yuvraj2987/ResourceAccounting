import threading, time

#Custom modules
import timeModule, deviceModule

class EventLogThread(threading.Thread):
  def __init__(self, start_time):
    threading.Thread.__init__(self)
    self.start_time = start_time
    self._stop = threading.Event()

  def stop(self):
    self._stop.set()

  def stopped(self):
    return self._stop.isSet()

  def run(self):
    print "---- Event Log Read Starts ----"
    logfile = "log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(self.start_time))+"_eventdevice.log"
    print "Logfile is:", logfile
    fileObj = open(logfile, "w+")
    fileObj.write("%.6f# # #\n" % self.start_time)
    deviceFd = deviceModule.open()
    print "Device Fd:", deviceFd
    
    numDevices = 0
    while (numDevices == 0):
      numDevices = deviceModule.get_device(deviceFd)
      print "Number of devices found:", numDevices
      if numDevices < 0:
          print "Error: eventLogDev thread"
          print "Number of devices found is less than 0"
          fileObj.close()
          sys.exit(-1)
    #while ends
    print "EventLogThread: number of devices found:", numDevices
    fileObj.write("%-20s\t%-20s\t%-20s\t%-20s\n" % ("DEVICE_NAME", "OPEN_TIME", "PID", "LATENCY"))

    while not self.stopped():
      for dev in range(0, numDevices):
        tuples = deviceModule.set_device_info(deviceFd, dev)
        deviceName = str(tuples[0])
        openTime = "%ld.%06ld" % (tuples[1], tuples[2])
        pid      = tuples[5]
        eventsConsumed = tuples[6]
        latency = "%ld.%06ld" % (tuples[7], tuples[8])
        fileObj.write("%-20s\t%-20s\t%-20hd\t%-20s\n" % (deviceName, openTime, pid, latency))
        fileObj.write("--------------------------------------------------------------------\n");
        time.sleep(5)
        #time.sleep(5)
      #end of for
    #end of while
    print "Close file"
    fileObj.close()
    print "EventLogThread exit"
  #End of thread run

def main():
  try:
    start_time = time.time()
    th = EventLogThread(start_time)
    print "Start the thread"
    th.start()
    while True:
      time.sleep(15)
  except KeyboardInterrupt:
    print "Interrupt event log read thread"
    th.stop()
    th.join()



if __name__ == "__main__":
  main()
