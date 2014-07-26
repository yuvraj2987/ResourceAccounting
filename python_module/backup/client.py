#!/usr/bin/python

import socket

def main():
  print "--- start python client ---"
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.connect((host, port))
  print s.recv(1024)
  s.close()
  print "--- client ends ---"

if __name__== '__main__':
  main()
