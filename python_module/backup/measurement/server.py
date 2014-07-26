#!/usr/bin/python

import socket

def main():
  print "--- starting server ---"
  s = socket.socket(socket.AF_UNIX)
  host = "127.0.0.1"
  port = 40000
  s.bind((host, port))

  s.setblocking(1)
  s.listen(5)
 
  while True:
    c, addr = s.accept()
    print "Got connection from:", addr
#    c.send("Thank you for connecting to the server")
#    print "sent, now receiving..."
    mystr = c.recv(1024)
    print "got this:"
    print mystr
    print "reception done, closing"
    c.close()

if __name__== '__main__':
  main()
