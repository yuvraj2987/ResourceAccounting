import smbus
import threading
import socket
import time
import sys, os
import traceback
#Custome Modules
import eventReadModule
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
def convert_time(tmStr):
	tm_list = tmStr.split(".")
	#print "split time", str(tm_list)

	tm = [0.0, 0.0]
	tm[0]  = float(tm_list[0])
	tm[1]  = float(tm_list[1])

	ret = tm[0] + (tm[1]+0.0)/1000000000
	return ret


# gets current time of CLOCK_MONOTONIC
# returns time in [0] seconds and [1] nanoseconds
# this def returns the seconds from CLOCK_MONOTONIC
# ts[1] is divided by large number to convert it to seconds
# 1000000000 ns = 1 second
def get_time():
  ts=timeModule.time()
  ret = ts[0]+(ts[1]+0.0)/1000000000
  return ret

def saveRecords_power(records,fileName):
	with file(fileName, "a+") as fd:
		for myRecord in range(0,len(records)/2):
			myStr="%.6f %.1f" % (records[2*myRecord], records[2*myRecord+1])
			fd.write(myStr+"\n")
#####
def saveRecords_socket(records,fileName):
	#print "Start saveRecords"
	myStr = records[0]+" "+records[1]+" "+records[2]+" "+records[3]
	#print myStr
	with file(fileName, "a+") as fd:
		fd.write(myStr+"\n")
	
#####
def getProcessName(port):
	#print "getProcessName starts for port:", port	
	cmd1 = "lsof -i :"+port
  	result = os.popen(cmd1)
  	resultStr = result.read()
	lines = resultStr.splitlines()
	processName = ""
	pid = ""
	if len(lines) > 2:
   		#print "lines are greater than 2"    
	   for line in lines[1:]:
		  #print line.strip()
		  #print "Type of line:",type(line)
		  cols = line.split()
		  #print "Type of cols:", type(cols)
		  #print cols[:2]
		  tname, tid= cols[:2]
		  if tname not in ['gpsd', 'bash']:
		    processName = tname
		    pid = tid 
		    break
		  else:
		    processName = ""
		    pid = ""
	else:
		#print "lines are less than 2"
		#added by kyle
		try:
			cols = lines[1].split()
			processName, pid = cols[:2]
		except:
			processName = ""
			pid = ""
	#print "process:%s\t pid:%s"%(processName, pid)
  	return processName, pid
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
			fd.write("#time #power(uW)")
#			fd.write("%.6f #\n" % self.offsetTime)
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
	print "--- main starts  ---"
#	reference_time=get_time()
	reference_time=0
	start_time = time.time()
	myFile_power = "../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(start_time))+"_power.log"
	myFile_socket = "../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S",time.localtime(start_time))+"_socket.log"
	bus.write_word_data(DEVICE_ADDRESS, 0x00, 0x9F08)
	print "power log file set to "+myFile_power
	print "socket log file set to "+myFile_socket

	with file(myFile_socket, "a+") as fd:
		fd.write("#time #0-reqest from apps/1-respond from gpsd to the app #app name  #app PID")
#		fd.write("%.6f # # #\n" % reference_time)
        

	print "--- running measureThread ----"
	
	thread_measure = measureThread(reference_time, myFile_power)
	#thread_events  = eventReadModule.EventLogThread(reference_time)
	thread_measure.start()
	#thread_events.start()

	print "--- starting server ---"

	try:
		
		mySocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		host = "127.0.0.1"
		port = 40000
		mySocket.bind((host, port))
		mySocket.setblocking(1)
		mySocket.listen(5)
		print "Server started"
		
		while True:
			try:
				conn, addr = mySocket.accept()
				print "Got connection from:", addr
				receiveStr = conn.recv(1024)
				print "Received following message\n", receiveStr
				recordType, port, tmStr = receiveStr.split("|")
				process, pid = getProcessName(port)
				print "Got process name: ", process, " | ", pid
				nowTime = convert_time(tmStr) 
				print "Time from gpsd:", str(nowTime)
				strTime = "%.6f" % (nowTime - reference_time)
				print "Got start time: ", str(strTime)
			
				myArray_socket = [str(strTime), str(recordType), str(process), str(pid)]
				dummyArray = myArray_socket
				print "Array:",dummyArray				
				print "Start writing "
				saveRecords_socket(dummyArray, myFile_socket)
				print "Record saved in file"
				
			except:
				print "Exception in socket communication"
				print "Close the socket"
				print traceback.format_exc()
				mySocket.close()
				raise
	        
	except KeyboardInterrupt:
		#print "Exception message:"+ str(sys.exc_info())
		print "Keyboard Interrupt occoured"
		thread_measure.stop()
		#thread_events.stop()
		print "Wait for threads to exit"
		thread_measure.join()
		#thread_events.join()
	except:
		print "Unexpected error:"+ str(sys.exc_info())
		thread_measure.stop()
		#thread_events.stop()
		print "Wait for threads to exit"
		thread_measure.join()
		#thread_events.join()

	print "--- end of program ---"

def test():
  startTime = get_time()
  print startTime
  print time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(startTime))
  print "--------"
  print "Testting timestamp"
  print "Cur time from timeModule", timeModule.time()
  print "get_time: ", get_time()
  print "time.time()", time.time()
  print "original : ", time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(time.time()))
#####

if __name__ == "__main__":
  main()
  #test()

#####
