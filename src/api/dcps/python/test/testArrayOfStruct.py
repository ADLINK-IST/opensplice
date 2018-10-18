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
    import ArrayOfStruct.basic.module_ArrayOfStruct

class TestArrayOfStruct(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = ArrayOfStruct.basic.module_ArrayOfStruct.ArrayOfStruct_struct(
            long1 = 12,
            array1=[ArrayOfStruct.basic.module_ArrayOfStruct.Inner(long1=21,double1=2.5),
                    ArrayOfStruct.basic.module_ArrayOfStruct.Inner(long1=31,double1=3.5)],
            mylong1 = 42)
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = ArrayOfStruct.basic.module_ArrayOfStruct.ArrayOfStruct_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(data.array1[0].long1, data1.array1[0].long1)
        self.assertEqual(data.array1[0].double1, data1.array1[0].double1)
        self.assertEqual(data.array1[1].long1, data1.array1[1].long1)
        self.assertEqual(data.array1[1].double1, data1.array1[1].double1)
        self.assertEqual(data.mylong1, data1.mylong1)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testCopyInCopyOut']
    unittest.main()