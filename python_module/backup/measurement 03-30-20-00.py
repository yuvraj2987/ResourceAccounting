
import smbus
import time
import threading


#globalvariables and methods
filename = "measurement.log"
fileLock = threading.Lock()
bus = smbus.SMBus(1)
startTime = 0
## Device config
DEVICE_ADDRESS= 0x40
REG_CONFIG = 0x00
REG_SHUNT = 0x01
REG_BUS = 0x02
REG_POWER = 0x03
REG_CURRENT = 0x04
REG_CALIB = 0x05

def save_records(records):
  with file(filename, "a+") as fd:
    for val in records:
      fd.write(str(val)+"\n")

class writerThread (threading.Thread):
  def __init__(self, array):
    threading.Thread.__init__(self)
    self.writeArray = array

  def run(self):
   print "--- writer thread starts ---"
   fileLock.acquire()
   save_records(self.writeArray)
   fd = open(filename, 'a+')
   s = "--- writing finished at "+str(time.time() - startTime)+"\n"
   fd.write(s)
   fd.close()
   fileLock.release()
   print "---- writer thread ends ----"


def main():
  print "--- current sensor configuration ---"
  bus.write_word_data(DEVICE_ADDRESS, 0x00, 0x9F08)
  
  print "--- i2c testing ----"
  with file(filename, "a+") as fh:
    fh.write("------ record starts ---- \n")

  my_array = []
  startTime = time.time()

  while True:
    rtTime = time.time() - startTime
    tmpShunt = bus.read_word_data(DEVICE_ADDRESS, REG_SHUNT)
    tmpBus = bus.read_word_data(DEVICE_ADDRESS, REG_BUS)
    voltage = ((tmpBus & 0xFF00) >>8) | ((tmpBus & 0x00FF) << 8)
    if voltage > 65000:
      voltage = 0

    rtVoltage = (voltage >> 3) * 4
    shunt = ((tmpShunt & 0xFF00) >>8) | ((tmpShunt & 0x00FF) << 8)
    if shunt > 65000:
      shunt = 0

    rtCurrent = (shunt+0.0)/10

    rtPower = rtCurrent * rtVoltage
    my_array.append(rtTime)
    my_array.append(rtPower)

    if len(my_array) == 2000:
      dup_array = my_array
      thread = writerThread(dup_array)
      thread.start()
      my_array = []
    #end if
  #end while

  print "--- end of program ---"

if __name__ == "__main__":
  main()
