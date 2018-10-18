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
    from SequenceOfSimpleArray.basic.module_SequenceOfSimpleArray import SequenceOfSimpleArray_struct

class TestSequenceOfSimpleArray(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = SequenceOfSimpleArray_struct(
            long1 = 13,
            sequence1 = [[11, 12], [21,22], [31,32]])
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = SequenceOfSimpleArray_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(len(data.sequence1), len(data1.sequence1))
        for i in range(len(data.sequence1)):
            self.assertEqual(data.sequence1[i], data1.sequence1[i], 'sequence[%d]'%(i))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestSequenceOfSimpleArray.testCopyInCopyOut']
    unittest.main()