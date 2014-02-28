#!/usr/bin/env python
import cPickle
import sys
from TestHost import TestHost
    
def hostList():
    """Modify this list to reflect the STAF hosts available in your local network"""
    list = []
    list.append(TestHost('patrick',           ['x86.linux-dev', 'x86.linux-release'], ['RTS', 'HDE', 'SRC'], 6500))
    list.append(TestHost('patrick-winxp',       ['x86.win32-release'], ['HDE', 'RTS']))
    list.append(TestHost('xlrw04.testnet.ptnl', ['x86.linux-dev', 'x86.linux-release']))
    list.append(TestHost('xlrw05.testnet.ptnl', ['x86.win32-dev', 'x86.win32-release']))
    list.append(TestHost('xlrw06.testnet.ptnl', ['x86.linux-dev']))
    list.append(TestHost('xlrw07.testnet.ptnl', ['x86.linux-dev']))
    list.append(TestHost('xlrw08.testnet.ptnl', ['x86.linux-dev']))
    list.append(TestHost('ssso09.testnet.ptnl', ['sparc.solaris8-dev', 'sparc.solaris8-release']))
    list.append(TestHost('sssx17.testnet.ptnl', ['sparc.solaris10_studio12-dev', 'sparc.solaris10_studio12-release']))
    list.append(TestHost('perf4.perfnet.ptnl',  ['x86_64.linux-dev', 'x86_64.linux-release']))
    return list

if __name__ == '__main__':
    if (len(sys.argv) != 2):
        print 'Usage: %s [fileName]' % sys.argv[0]
        sys.exit(1)
    list = hostList()
    output = open(sys.argv[1], 'wb')
    cPickle.dump(list, output)
