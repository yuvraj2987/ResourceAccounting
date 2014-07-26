#!/usr/bin/python

import socket

def main():
  print "--- starting server ---"
  s = socket.socket()
  host = socket.gethostname()
  port = 12345
  s.bind((host, port))

  s.listen(5)
  while True:
    c, addr = s.accept()
    print "Got connection from:", addr
    c.send("Thank you for connecting to the server")
    c.close()

if __name__== '__main__':
  main()
