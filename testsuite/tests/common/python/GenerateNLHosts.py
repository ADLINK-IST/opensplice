#!/usr/bin/env python
import cPickle
import sys
from TestHost import TestHost

def hostList():
    """Modify this list to reflect the STAF hosts available in your local network"""
    list = []
    list.append(TestHost('patrick.office.ptnl',     ['x86_64.linux-release'], ['RTS']))
    list.append(TestHost('x64-ub10-01.office.ptnl', ['x86_64.linux-release'], ['RTS']))
    list.append(TestHost('x64-ub10-02.office.ptnl', ['x86_64.linux-release'], ['RTS']))
    list.append(TestHost('perf4.perfnet.ptnl',      ['x86_64.linux-release'], ['RTS']))
    return list

if __name__ == '__main__':
    if (len(sys.argv) != 2):
        print 'Usage: %s [fileName]' % sys.argv[0]
        sys.exit(1)
    list = hostList()
    output = open(sys.argv[1], 'wb')
    cPickle.dump(list, output)
