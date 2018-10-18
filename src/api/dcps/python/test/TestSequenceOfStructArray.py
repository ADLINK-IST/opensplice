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
Created on Dec 8, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    from SequenceOfStructArray.basic.module_SequenceOfStructArray import SequenceOfStructArray_struct, Inner

class TestSequenceOfStructArray(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = SequenceOfStructArray_struct(
            long1 = 13,
            seq1= [
                [Inner(short1=11,double1=1.1),Inner(short1=12,double1=1.2)],
                [Inner(short1=21,double1=2.1),Inner(short1=22,double1=2.2)],
                [Inner(short1=31,double1=3.1),Inner(short1=32,double1=3.2)],
                ])
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = SequenceOfStructArray_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(len(data.seq1), len(data1.seq1))
        for i in range(len(data.seq1)):
            self.assertEqual(len(data.seq1[i]), len(data1.seq1[i]), 'len(seq1[%d])'%(i))
            for j in range(len(data.seq1[i])):
                self.assertEqual(data.seq1[i][j].short1, data1.seq1[i][j].short1, 'seq1[%d][%d].short1'%(i,j))
                self.assertEqual(data.seq1[i][j].double1, data1.seq1[i][j].double1, 'seq1[%d][%d].double1'%(i,j))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestSequenceOfStructArray.testCopyInCopyOut']
    unittest.main()