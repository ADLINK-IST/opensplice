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
Created on Dec 7, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    import MultiArrayBasicType.basic.module_MultiArrayBasicType

class TestMultiArrayBasicType(unittest.TestCase):


    def testCopyInCopyOut(self):
        d = [[[(i+1) * 100 + (j+1) * 10 + (k+1)*1 for k in range (2)] for j in range(3)] for i in range(2)]
        data = MultiArrayBasicType.basic.module_MultiArrayBasicType.MultiArrayBasicType_struct(
            long1 = 12,
            array1 = d
            )
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = MultiArrayBasicType.basic.module_MultiArrayBasicType.MultiArrayBasicType_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        for i in range(len(data.array1)):
            for j in range(len(data.array1[i])):
                for k in range(len(data.array1[i][j])):
                    self.assertEqual(data.array1[i][j][k], data1.array1[i][j][k], 'array1[%d][%d][%d]' % (i, j, k))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testCopyInCopyOut']
    unittest.main()