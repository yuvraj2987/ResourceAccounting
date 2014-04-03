import smbus
import time
import threading


#globalvariables and methods
myFile = "default.log"
fileLock = threading.Lock()
bus = smbus.SMBus(1)
bufferSize=100

## Device config
DEVICE_ADDRESS= 0x40
REG_CONFIG = 0x00
REG_SHUNT = 0x01
REG_BUS = 0x02
REG_POWER = 0x03
REG_CURRENT = 0x04
REG_CALIB = 0x05

def save_records(records,filename):
	with file(filename, "a+") as fd:
		for myRecord in range(0,len(records)/2):
			myStr="%.6f %.1f" % (records[2*myRecord], records[2*myRecord+1])
			fd.write(myStr+"\n")
#####
class writerThread (threading.Thread):
	def __init__(self, array, offset, iter, filename):
		threading.Thread.__init__(self)
		self.writeArray = array
		self.offsetTime = offset
		self.iteration = iter
		self.filename=filename

	def run(self):
		print "--- writer thread number "+str(self.iteration)+" starts ---"
		fileLock.acquire()
		save_records(self.writeArray, self.filename)
		#fd = open(self.filename, 'a+')
		#s = "--- writing finished at "+str(time.time() - self.offsetTime)+"\n"
		#fd.write(s)
		#fd.close()
		fileLock.release()
		print "---- writing thread finished file writing at "+str(time.time() - self.offsetTime)

#####
def main():
	print "--- start of program ---"
	bus.write_word_data(DEVICE_ADDRESS, 0x00, 0x9F08)
	my_array = []
	myIteration=1;
	startTime = time.time()
	myFile = "log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(startTime))+".log"
	with file(myFile, "a+") as fd:
		fd.write("# %.6f\n" % startTime)
	#my_array.append(startTime)
	#my_array.append(0)
	
	print "log file set to "+myFile+"\n"
	print "--- sensing starts ----"
	
	while True:
		time.sleep(0.01)
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
		
		if len(my_array) == 2*bufferSize:
			dup_array = my_array
			thread = writerThread(dup_array, startTime, myIteration, myFile)
			thread.start()
			myIteration = myIteration + 1
			my_array = []
		#end if
	#end while

	print "--- end of program ---"

#####

if __name__ == "__main__":
	main()

#####