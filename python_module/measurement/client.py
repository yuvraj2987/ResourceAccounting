#!/usr/bin/python

import socket
import time

def main():
  print "--- start python client ---"
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.connect((host, port))
  #print s.recv(1024)
  s.send("1 2001")
  s.close()

  time.sleep(0.5)
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.connect((host, port))
  s.send("2 2002")
  s.close()

  time.sleep(0.25)
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.connect((host, port))
  s.send("2 2003")
  s.close()

  time.sleep(1.3)
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.connect((host, port))
  s.send("1 2004")
  s.close()


  print "--- client ends ---"

if __name__== '__main__':
  main()
