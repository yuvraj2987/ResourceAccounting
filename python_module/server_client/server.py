#!/usr/bin/python

import socket
import timeModule
def main():
  tupTs = timeModule.time();
  ts = "%ld.%06ld" % (tupTs[0], tupTs[1])
  print "--- starting server at ---", ts
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  #host = socket.gethostname()
  host = "localhost"
  print "host: ", host
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
    #c.send("Server string")
    c.close()

if __name__== '__main__':
  main()
