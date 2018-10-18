#!/usr/bin/python

import os

print "Content-Type: text/plain\n\n"
for key in os.environ.keys():
    print "%30s %s \n" % (key,os.environ[key])
