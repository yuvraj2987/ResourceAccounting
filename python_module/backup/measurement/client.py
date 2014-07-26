#!/usr/bin/python

import socket
import time

def main():
  print "--- start python client ---"
  s = socket.socket()
#  host = socket.gethostname()
  host = "127.0.0.1"
  port = 40000 #12345
  s.connect((host, port))
  #print s.recv(1024)
  s.send("1|2001|process_one")
  s.close()

  time.sleep(0.5)
  s = socket.socket()
  port = 40000 #12345
  s.connect((host, port))
  s.send("2|2002|process_two")
  s.close()

  time.sleep(0.25)
  s = socket.socket()
  port = 40000 #12345
  s.connect((host, port))
  s.send("2|2003|process_three")
  s.close()

  time.sleep(1.3)
  s = socket.socket()
  port = 40000 #12345
  s.connect((host, port))
  s.send("1|2004|process_four")
  s.close()


  print "--- client ends ---"

if __name__== '__main__':
  main()
