#!/usr/bin/python

import socket
import timeModule
import os

def getProcessName(port):
  cmd1 = "lsof -i :"+port
  result = os.popen(cmd1)
  resultStr = result.read()
  #print "---- lsof result -----"
  #print resultStr
  #print "-------------------"
  #print "split lines"
  #Ignore first line
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
    cols = lines[1].split()
    processName, pid = cols[:2]
    #print "process:%s\t pid:%s"%(processName, pid)
    #print "-----------------"
  #end of if and else
  print "process:%s\t pid:%s"%(processName, pid)
  return processName, pid
  
def main():
  tupTs = timeModule.time();
  ts = "%ld.%06ld" % (tupTs[0], tupTs[1])
  print "--- starting server at ---", ts
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  #host = socket.gethostname()
  host = "127.0.0.1"
  print "host: ", host
  port = 40000
  s.bind((host, port))
  s.setblocking(1)
  s.listen(5)

  while True:
    try:
      c, addr = s.accept()
      print "Got connection from:", addr
#     c.send("Thank you for connecting to the server")
#     print "sent, now receiving..."
      receivedStr = c.recv(1024)
      print "got this:"
      print receivedStr
      print "reception done, closing"
      recordType, port, time = receivedStr.split("|")
      process, pid = getProcessName(port)
      gpsdList = [str(recordType), process, pid, str(time)]
      print "------------- Gpsd Data String ---------"
      print gpsdList
    #c.send("Server string")
    except: 
      c.close()

if __name__== '__main__':
  main()
