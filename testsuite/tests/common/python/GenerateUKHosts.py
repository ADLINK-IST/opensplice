#!/usr/bin/env python
import cPickle
import sys
from TestHost import TestHost
    
def hostList():
    """Modify this list to reflect the STAF hosts available in your local network"""
    list = []
    list.append(TestHost('rhel4g.prismtech.com',     ['x86.linux-release'], ['HDE']))
    list.append(TestHost('rhel53-64.prismtech.com',  ['x86_64.linux-release'], ['HDE']))
    list.append(TestHost('rhel54-64b.prismtech.com',  ['x86_64.linux-release'], ['HDE']))
    list.append(TestHost('rhel54-64c.prismtech.com',  ['x86_64.linux-release'], ['HDE']))      
    return list

if __name__ == '__main__':
    if (len(sys.argv) != 2):
        print 'Usage: %s [fileName]' % sys.argv[0]
        sys.exit(1)
    list = hostList()
    output = open(sys.argv[1], 'wb')
    cPickle.dump(list, output)
