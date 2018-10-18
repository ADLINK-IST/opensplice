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
    from sample2.basic import Inner
    from sample2.basic.test import Type1

class Testsample2(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = Type1(
            long1 = '\x13',
            inner1 = [
                Inner(long1=11,double1=1.1,array1=[11,12,13],str1='One'),
                Inner(long1=21,double1=2.1,array1=[21,22,23],str1='Two'),
                Inner(long1=31,double1=3.1,array1=[31,32,33],str1='Three'),
                ],
            mylong1 = 23
            )
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        print('data._get_packing_args(): ', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = Type1()
        data1._deserialize(list(values))
        
        self.assertEqual(data.long1, data1.long1)
        self.assertEqual(data.mylong1, data.mylong1)
        self.assertEqual(len(data.inner1),len(data1.inner1))
        for i in range(len(data.inner1)):
            self.assertEqual(data.inner1[i].long1,data1.inner1[i].long1,'inner1[%d].long1'%i)
            self.assertEqual(data.inner1[i].double1,data1.inner1[i].double1,'inner1[%d].double1'%i)
            self.assertEqual(len(data.inner1[i].array1),len(data1.inner1[i].array1),'len(inner1[%d].array1)'%i)
            for j in range(len(data.inner1[i].array1)):
                self.assertEqual(data.inner1[i].array1[j],data1.inner1[i].array1[j],'inner1[%d].array1[%d]'%(i,j))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Testsample2.testCopyInCopyOut']
    unittest.main()