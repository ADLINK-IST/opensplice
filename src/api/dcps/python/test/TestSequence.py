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
    import Sequence.basic.module_Sequence

class TestSequence(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = Sequence.basic.module_Sequence.Sequence_struct(
            long1=12,
            seq1=[21, 32, 43]
            )
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = Sequence.basic.module_Sequence.Sequence_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(data.seq1[0],data1.seq1[0])
        self.assertEqual(data.seq1[1],data1.seq1[1])
        self.assertEqual(data.seq1[2],data1.seq1[2])

    def testCopyInCopyOutEmpty(self):
        data = Sequence.basic.module_Sequence.Sequence_struct(
            long1=12,
            seq1=[]
            )
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = Sequence.basic.module_Sequence.Sequence_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestSequence.testCopyInCopyOut']
    unittest.main()