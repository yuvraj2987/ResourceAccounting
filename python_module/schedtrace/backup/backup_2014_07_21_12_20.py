import sys, os
from optparse import OptionParser
from time import sleep

#parser = OptionParser()
#parser.add_option("-f", "--file", dest="filename",
#                      help="Log to FILE", metavar="FILE")
#parser.add_option("-q", "--quiet",
#                      action="store_false", dest="verbose", default=True,
#                                        help="don't print status messages to stdout")
#
#(options, args) = parser.parse_args()

#fwrite = open(options.filename, "w")

os.system("/proc/uptime | awk '{print $1}'")

fwrite = open("out.txt", "w")

os.system("echo 0 >/sys/kernel/debug/tracing/tracing_enabled")

os.system("echo 1 >/sys/kernel/debug/tracing/tracing_enabled")

while (1):
  fread = open("/sys/kernel/debug/tracing/trace", "r")
  sys.stdout.write('-')
  sys.stdout.flush()
  c = 0
  l = ""
  for line in fread:
    c += 1
    l += line
    if c==100000:
      fwrite.write(l)
      c = 0
      l = ""
      sys.stdout.write('.')
      sys.stdout.flush()
  if c !=0:
    fwrite.write(l)

  sleep(0.1)
