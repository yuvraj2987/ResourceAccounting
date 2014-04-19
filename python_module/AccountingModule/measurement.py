import smbus
import time
import threading
import socket


#Custome Modules
import event_log_read
import timeModule


#globalvariables and methods
myFile_power = "default_power.log"
myFile_socket = "default_socket.log"
fileLock_power = threading.Lock()
fileLock_socket = threading.Lock()
bus = smbus.SMBus(1)
lineBufferSize=100

## Device config
DEVICE_ADDRESS= 0x40
REG_CONFIG = 0x00
REG_SHUNT = 0x01
REG_BUS = 0x02
REG_POWER = 0x03
REG_CURRENT = 0x04
REG_CALIB = 0x05
##### Global variables
def get_time():
  ts = timeModule.time()
  ret = ts[0] + (ts[1]+0.0)/1000000000
  return ret

def get_time(ts):
  ret = ts[0] + (ts[1]+0.0)/1000000000
  return ret

def saveRecords_power(records,fileName):
	with file(fileName, "a+") as fd:
		for myRecord in range(0,len(records)/2):
			myStr="%.6f %.1f" % (records[2*myRecord], records[2*myRecord+1])
			fd.write(myStr+"\n")
#####
def saveRecords_socket(records,fileName):
	with file(fileName, "a+") as fd:
		for myRecord in range(0,len(records)/4):
			myStr= (records[4*myRecord]," ", records[4*myRecord+1]," ",  records[4*myRecord+2], " ", records[4*myRecord+3])
			fd.write(myStr+"\n")
#####
#####
class writerThread_power (threading.Thread):
	def __init__(self, array, offset, tmpiteration, fileName):
		threading.Thread.__init__(self)
		self.writeArray = array
		self.offsetTime = offset
		self.iteration = tmpiteration
		self.fileName=fileName

	def run(self):
		if self.iteration%10==1:
			print "--- writerThread_power number "+str(self.iteration)+" starts ---"
		fileLock_power.acquire()
		saveRecords_power(self.writeArray, self.fileName)
		fileLock_power.release()
		if self.iteration%10==1:
			print "--- writingThread_power finished file writing at "+str(get_time() - self.offsetTime)

#####
class writerThread_socket (threading.Thread):
	def __init__(self, array, offset, tmpiteration, fileName):
		threading.Thread.__init__(self)
		self.writeArray = array
		self.offsetTime = offset
		self.iteration = tmpiteration
		self.fileName=fileName

	def run(self):
		print "--- writerThread_socket number "+str(self.iteration)+" starts ---"
		fileLock_socket.acquire()
		saveRecords_socket(self.writeArray, self.fileName)
		fileLock_socket.release()
		print "--- writingThread_socket finished file writing at "+str(get_time() - self.offsetTime)

#####
#####
class measureThread (threading.Thread):
	def __init__(self, offset, fileName):
		threading.Thread.__init__(self)
		self._stop = threading.Event()
		self.offsetTime = offset
		self.fileName=fileName

	def stop(self):
		self._stop.set()

	def stopped(self):
		return self._stop.isSet()

	def run(self):
		print "--- measureThread starts ---"
		
		with file(self.fileName, "a+") as fd:
			fd.write("%.6f #\n" % self.offsetTime)
		myArray_power = []
		myIteration_power=1;

		while True:
			time.sleep(0.01)
			rtTime = get_time() - self.offsetTime
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
			myArray_power.append(rtTime)
			myArray_power.append(rtPower)
		
			if len(myArray_power) == 2*lineBufferSize:
				dupArray = myArray_power
				thread_power = writerThread_power(dupArray, self.offsetTime, myIteration_power, self.fileName)
				myIteration_power = myIteration_power + 1
				myArray_power = []
				thread_power.start()
			
			if self.stopped() == True:
				print "--- measureThread is interrupted ----"
				break

			#end if
		#end while
		print "--- measureThread is writing the rest of the data ----"
		dupArray = myArray_power
		thread_power = writerThread_power(dupArray, self.offsetTime, myIteration_power, self.fileName)
		myIteration_power = myIteration_power + 1
		myArray_power = []
		thread_power.start()

		print "--- measureThread says bye bye ---"
#####

def main():
	print "--- main starts ---"
	startTime = get_time()
	myFile_power = "log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(startTime))+"_power.log"
	myFile_socket = "log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(startTime))+"_socket.log"
	bus.write_word_data(DEVICE_ADDRESS, 0x00, 0x9F08)
	print "power log file set to "+myFile_power
	print "socket log file set to "+myFile_socket

	with file(myFile_socket, "a+") as fd:
		fd.write("%.6f # # #\n" % startTime)


	print "--- running measureThread ----"
	
	thread_measure = measureThread(startTime, myFile_power)
  thread_events  = event_log_read.EventLogThread(startTime)
	thread_measure.start()
  thread_events.start()

	print "--- starting server ---"

	try:
		mySocket = socket.socket()
		#host = socket.gethostname()
		host = "127.0.0.1"
		port = 40000 #12345
		mySocket.bind((host, port))
		mySocket.setblocking(1)
		mySocket.listen(5)
		myArray_socket
		myIteration_socket=1;
	
		print "--- listening on server starts ---"

		while True:
			try:	
				myConnection, myAddress = mySocket.accept()
				#nowTime = get_time()
				socketStr = myConnection.recv(1024)
				print "--- got something form the socket: " + socketStr + " ---"
				myConnection.close()
				#recordType, recordID, recordName = socketStr.split("|")
        recordType, port, time = receivedStr.split("|")
        process, pid = getProcesInfo(port)
        nowTime = get_time(time) 
				strTime = "%.6f" % (nowTime - startTime)
				#myArray_socket.append(strTime)
				#myArray_socket.append(str(recordType))
				#myArray_socket.append(str(recordID))
				#myArray_socket.append(str(recordName))
        myArray_socket = [strTime, str(recordType), process, pid]
				dummyArray = myArray_socket
				thread_socket = writerThread_socket (dummyArray, startTime, myIteration_socket, myFile_socket)
				myIteration_socket = myIteration_socket + 1
				thread_socket.start()
			except:
				thread_measure.stop()
				break

	except:
		print "--- something wrong with the socket ---"
		thread_measure.stop()
	


	print "--- end of program ---"

def test():
  print "Testting timestamp"
  print "Cur time from timeModule", timeModule.time()
  print "get_time: ", get_time()
#####

if __name__ == "__main__":
	main()
  #test()

#####
