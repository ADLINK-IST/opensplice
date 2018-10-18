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
Created on Dec 4, 2017

@author: prismtech
'''
import unittest
import struct
import dds
import countTest

if countTest.count_test == False:
    import NestedStruct.basic.module_NestedStruct



class Test(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = NestedStruct.basic.module_NestedStruct.NestedStruct_struct(
            long1=11,
            inner1=NestedStruct.basic.module_NestedStruct.Inner(long1=21, double1=22.2))
        
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = NestedStruct.basic.module_NestedStruct.NestedStruct_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(data.inner1.long1, data1.inner1.long1)
        self.assertEqual(data.inner1.double1, data1.inner1.double1)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()