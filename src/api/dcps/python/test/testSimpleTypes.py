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
Created on Dec 1, 2017

@author: prismtech
'''
import unittest
import struct
import dds
import countTest

if countTest.count_test == False:
    import SimpleTypes.basic.module_SimpleTypes

class Test(unittest.TestCase):


    def testName(self):
        data = SimpleTypes.basic.module_SimpleTypes.SimpleTypes_struct(
            string1='Hello World!',
            char1='X',
            bool1=True,
            octet1=127,
            long1=1123)
        
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = SimpleTypes.basic.module_SimpleTypes.SimpleTypes_struct()
        data1._deserialize(list(values))
        
        self.assertEqual(data.string1, data1.string1)
        self.assertEqual(data.char1, data1.char1)
        self.assertEqual(data.bool1, data1.bool1)
        self.assertEqual(data.octet1, data1.octet1)
        self.assertEqual(data.long1, data1.long1)

    def testDDSTopic(self):
        data = SimpleTypes.basic.module_SimpleTypes.SimpleTypes_struct(
            string1='Hello World!',
            char1='X',
            bool1=True,
            octet1=127,
            long1=1123)

        dp = dds.DomainParticipant()
        qos = None
        topic = dds.Topic(dp, 'SimpleTypes', SimpleTypes.basic.module_SimpleTypes.SimpleTypes_struct.get_type_support())
        
        self.assertIsNotNone(topic, 'Returned topic must not be None')

        pub = dp.create_publisher()
        sub = dp.create_subscriber()
        
        wr = pub.create_datawriter(topic)
        rd = sub.create_datareader(topic)
        
        wr.write(data)
        
        rs = rd.take(1)
        self.assertEqual(len(rs),1)
        
        self.assertEqual(str(rs[0][0]), str(data))

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()