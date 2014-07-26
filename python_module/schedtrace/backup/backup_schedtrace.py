import sys, os
from optparse import OptionParser
from time import sleep

parser = OptionParser()
parser.add_option("-f", "--file", dest="filename",
                      help="Log to FILE", metavar="FILE")
parser.add_option("-q", "--quiet",
                      action="store_false", dest="verbose", default=True,
                                        help="don't print status messages to stdout")

(options, args) = parser.parse_args()

fwrite = open(options.filename, "w")

while 1:
  fread = open("/sys/kernel/debug/tracing/trace", "r")
  for line in fread:
    print fwrite.write(line)

  sleep(0.1)


