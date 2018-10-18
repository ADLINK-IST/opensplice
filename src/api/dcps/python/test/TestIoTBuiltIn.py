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
Created on Dec 18, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    from dds_IoTData.DDS.IoT import IoTData, IoTNVP, IoTType, IoTValue

class TestIoTBuiltIn(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = IoTData(
            typeName = 'Foo',
            instanceId = 'Bar',
            values=[
                IoTNVP(name='Tui8', value=IoTValue(ui8=127)),
                IoTNVP(name='Tui16', value=IoTValue(ui16=0x0102)),
                IoTNVP(name='Tui32', value=IoTValue(ui32=0x01020304)),
                IoTNVP(name='Tui64', value=IoTValue(ui64=0x0102030405060708)),
                IoTNVP(name='Ti8', value=IoTValue(i8='F')),
                IoTNVP(name='Ti16', value=IoTValue(i16=0x1102)),
                IoTNVP(name='Ti32', value=IoTValue(i32=0x11020304)),
                IoTNVP(name='Ti64', value=IoTValue(i64=0x1102030405060708)),
                IoTNVP(name='Tf32', value=IoTValue(f32=16.0)),
                IoTNVP(name='Tf64', value=IoTValue(f64=1024.0)),
                IoTNVP(name='Tb', value=IoTValue(b=True)),
                IoTNVP(name='Tstr', value=IoTValue(str='Hi There')),
                IoTNVP(name='Tch', value=IoTValue(ch='X')),

                IoTNVP(name='Tui8Seq', value=IoTValue(ui8Seq=[127,128,129])),
                IoTNVP(name='Tui16Seq', value=IoTValue(ui16Seq=[0x0102,0x0304])),
                IoTNVP(name='Tui32Seq', value=IoTValue(ui32Seq=[0x01020304,0x05060708])),
                IoTNVP(name='Tui64Seq', value=IoTValue(ui64Seq=[0x0102030405060708,0x0807060504030201])),
                IoTNVP(name='Ti8Seq', value=IoTValue(i8Seq=['F','G','H','I'])),
                IoTNVP(name='Ti16Seq', value=IoTValue(i16Seq=[0x0102,-0x0102])),
                IoTNVP(name='Ti32Seq', value=IoTValue(i32Seq=[0x11020304,-0x01020304])),
                IoTNVP(name='Ti64Seq', value=IoTValue(i64Seq=[0x1102030405060708,-0x0102030405060708])),
                IoTNVP(name='Tf32Seq', value=IoTValue(f32Seq=[16.0,32.0,64.0,128.0])),
                IoTNVP(name='Tf64Seq', value=IoTValue(f64Seq=[1024.0,1048.,4096.0])),
                IoTNVP(name='TbSeq', value=IoTValue(bSeq=[True,False,True])),
                IoTNVP(name='TstrSeq', value=IoTValue(strSeq=['Hi There','James','Bond'])),
                IoTNVP(name='TchSeq', value=IoTValue(chSeq=['X','Y','Z'])),
                ]
            )
        for t in (IoTData, IoTNVP, IoTValue):
            print('{0}._get_packing_fmt(): {1}, size={2}'.format( t.__name__, t._get_packing_fmt(), struct.calcsize(t._get_packing_fmt())))

        print('data._get_packing_args(): {1}', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = IoTData()
        data1._deserialize(list(values))
        
        self.assertEqual(data.typeName, data1.typeName)
        self.assertEqual(len(data.instanceId), len(data1.instanceId))
        for i in range(len(data.values)):
            self.assertEqual(str(data.values[i]), str(data1.values[i]), 'str(values[%d])'%(i))

    def testCopyInCopyOutEmpty(self):
        data = IoTData(
            typeName = 'Foo',
            instanceId = 'BarE'
            )
        for t in (IoTData, IoTNVP, IoTValue):
            print('{0}._get_packing_fmt(): {1}, size={2}'.format( t.__name__, t._get_packing_fmt(), struct.calcsize(t._get_packing_fmt())))

        print('data._get_packing_args(): {1}', data._get_packing_args())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = IoTData()
        data1._deserialize(list(values))
        
        self.assertEqual(data.typeName, data1.typeName)
        self.assertEqual(len(data.instanceId), len(data1.instanceId))
        for i in range(len(data.values)):
            self.assertEqual(str(data.values[i]), str(data1.values[i]), 'str(values[%d])'%(i))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestIoTBuiltIn.testCopyInCopyOut']
    unittest.main()