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
import countTest

if countTest.count_test == False:
    import Enum2.basic.module_Enum2
    import Enum2.basic.enumZ


class TestEnum2(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = Enum2.basic.module_Enum2.Enum2_struct(
            long1=11,
            color1=Enum2.basic.module_Enum2.Color.Blue,
            color2=Enum2.basic.enumZ.Color.Green,
            array=[Enum2.basic.enumZ.Color.Red, Enum2.basic.enumZ.Color.Green]
            )
        
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = Enum2.basic.module_Enum2.Enum2_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(data.color1, data1.color1)
        self.assertEqual(data.color2, data1.color2)
        self.assertEqual(len(data.array), len(data1.array))
        self.assertEqual(data.array[0], data1.array[0])
        self.assertEqual(data.array[1], data1.array[1])


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()