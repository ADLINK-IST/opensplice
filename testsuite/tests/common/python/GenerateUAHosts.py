#!/usr/bin/env python
import cPickle
import sys
from TestHost import TestHost
    
def hostList():
    """Modify this list to reflect the STAF hosts available in your local network"""
    list = []
    list.append(TestHost('ws175', ['x86.win32'], ['HDE'], 6503))
    list.append(TestHost('glasgow', ['x86.linux'], ['HDE'], 6504))
    list.append(TestHost('vm-dds-rhel53x86', ['x86.linux'], ['HDE'], 6503))
    list.append(TestHost('vm-dds-rhel53x64', ['x86_64.linux'], ['HDE'], 6503))
    return list

if __name__ == '__main__':
    if (len(sys.argv) != 2):
        print 'Usage: %s [fileName]' % sys.argv[0]
        sys.exit(1)
    list = hostList()
    output = open(sys.argv[1], 'wb')
    cPickle.dump(list, output)
