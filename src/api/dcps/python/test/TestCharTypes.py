#
#                         Vortex OpenSplice
#
#   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
#   Technology Limited, its affiliated companies and licensors. All rights
#   reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
'''
Created on Dec 17, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    from CharTypes.basic.module_CharTypes import CharTypes_struct

class TestCharTypes(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = CharTypes_struct(
            long1 = 32,
            char1='A',
            char_array=list('aBcDeFgHiJ'),
            char_seq1=list('ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
            char_seq2=list('qrstuvwxyz'),
            octet1=0x01,
            octet_array=[0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10],
            octet_seq1=[0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20],
            octet_seq2=[0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x30],
            )
        
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = CharTypes_struct()
        data1._deserialize(list(values))
        
        for attr in CharTypes_struct._member_attributes:
            self.assertEqual(getattr(data,attr),getattr(data1,attr),attr)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestCharTypes.testCopyInCopyOut']
    unittest.main()