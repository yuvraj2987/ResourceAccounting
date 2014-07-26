import sys, os
from optparse import OptionParser
from time import sleep

fwrite = open("out.txt", "w")

#os.system("/home/pi/ResourceAccounting/python_module/schedtrace/schedtrace.sh")

while (1):

  fread = open("/sys/kernel/debug/tracing/trace", "r")
  
  prevName = ""
  prevTime = ""
  tmp = ""

  for l in fread:
    if not l.startswith('#'):
      l = l.strip().split()
      name = l[0]
      timestamp = l[3][:-1]
      if name.startswith('cgps'):        
        if not prevName.startswith('cgps'):
          # switch in
          tmp += "i " + prevTime + "\n"
      elif prevName.startswith('cgps'):
          # switch out
          tmp += "o " + prevTime + "\n"
      prevName = name
      prevTime = timestamp
  fwrite.write(tmp)
  fwrite.flush()
  sleep(0.1)
