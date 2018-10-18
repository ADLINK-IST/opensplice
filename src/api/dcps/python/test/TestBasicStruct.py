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
    from BasicStruct.declaration_order import S11

class TestBasicStruct(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = S11(a=11,b=12,xa=21,xb=22,ya=31,yb=32,za=41,zb=42)
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = S11()
        data1._deserialize(list(values))
        
        for a in data._member_attributes:
            self.assertEqual(getattr(data,a),getattr(data1,a),a)
        


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestBasicStruct.testCopyInCopyOut']
    unittest.main()