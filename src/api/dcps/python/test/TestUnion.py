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
Created on Dec 16, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    from Union.basic.module_Union import ano21, Color, uone, utwo, uthree, ufour, ufive, Union_struct

class Test(unittest.TestCase):


    def testDefaultConstructor(self):
        u1 = uone()
        self.assertEqual(u1.discriminator, Color.Red)
        self.assertEqual(u1.s, 0)
        
        u2 = utwo()
        self.assertEqual(u2.discriminator, Color.Red)
        self.assertEqual(u2.s, 0)
        
        u3 = uthree()
        self.assertEqual(u3.discriminator, '\x00')
        self.assertIsNone(u3._v)
        
        u4 = ufour()
        self.assertEqual(u4.discriminator, 0)
        self.assertIsNone(u4._v)
        
        u5 = ufive()
        self.assertEqual(u5.discriminator, False)
        self.assertEqual(u5.sF, 0)
        
    def testDVConstructor(self):
        u1 = uone(Color.Green, 32)
        self.assertEqual(u1.discriminator, Color.Green)
        self.assertEqual(u1.l,32)
        
        u2 = utwo(Color.Green, 33)
        self.assertEqual(u2.discriminator, Color.Green)
        self.assertEqual(u2.l, 33)
        
        u3 = uthree('a',34)
        self.assertEqual(u3.discriminator, 'a')
        self.assertEqual(u3.sa, 34)
        
        u4 = ufour(2, 35)
        self.assertEqual(u4.discriminator, 2)
        self.assertEqual(u4.s2, 35)
        
        u5 = ufive(True, 36)
        self.assertEqual(u5.discriminator, True)
        self.assertEqual(u5.sT, 36)
        
    def testKWConstructor(self):
        u1 = uone(ll=41)
        self.assertEqual(u1.discriminator, Color.Yellow)
        self.assertEqual(u1.ll, 41)
        
        u2 = utwo(l=42)
        self.assertEqual(u2.discriminator, Color.Green)
        self.assertEqual(u2.l,42)
        
        u3 = uthree(sa=43)
        self.assertEqual(u3.discriminator, 'a')
        self.assertEqual(u3.sa, 43)
        
        u4 = ufour(s1=44)
        self.assertEqual(u4.discriminator, 1)
        self.assertEqual(u4.s1, 44)
        
        u5 = ufive(sT=45)
        self.assertEqual(u5.discriminator, True)
        self.assertEqual(u5.sT, 45)
        
    def testSetDefault(self):
        u2 = utwo()
        self.assertIsNotNone(u2._v)
        u2.set_default()
        self.assertIsNone(u2._v)
        
        u3 = uthree(sa=53)
        self.assertIsNotNone(u3._v)
        u3.set_default()
        self.assertIsNone(u3._v)
        
        u4 = ufour(s1=52)
        self.assertIsNotNone(u4._v)
        u4.set_default()
        self.assertIsNone(u4._v)
        
    def testSerializeDeserialize(self):
        for u in [uone(l=32), uone(s=16), uone(ll=64), uone()]:
            u_o = uone()
            u_o._deserialize(list(struct.unpack(uone._get_packing_fmt(),u._serialize())))
            self.assertEqual(str(u),str(u_o))
        
        for u in [utwo(s=15), utwo(l=333), utwo(Color.Blue,None), utwo()]:
            u_o = utwo()
            u_o._deserialize(list(struct.unpack(utwo._get_packing_fmt(),u._serialize())))
            self.assertEqual(str(u),str(u_o))
        
        for u in [uthree(sa=35), uthree(sb=333), uthree('x'), uthree()]:
            u_o = uthree()
            u_o._deserialize(list(struct.unpack(uthree._get_packing_fmt(),u._serialize())))
            self.assertEqual(str(u),str(u_o))       
        
        for u in [ufour(s1=155), ufour(s2=312), ufour(3,None), ufour()]:
            u_o = ufour()
            u_o._deserialize(list(struct.unpack(ufour._get_packing_fmt(),u._serialize())))
            self.assertEqual(str(u),str(u_o))
        
        for u in [ufive(sT=1066), ufive(sF=1944), ufive()]:
            u_o = ufive()
            u_o._deserialize(list(struct.unpack(ufive._get_packing_fmt(),u._serialize())))
            self.assertEqual(str(u),str(u_o))

        ano1 = ano21()
        ano1.discriminator = 97
        ano1.x1[0][0] = 11
        ano1.x1[1][1] = 22
        ano1.x1[11][11] = 1212
        
        ano2 = ano21()
        ano2.discriminator = 98
        ano2.x2[0][1][2][5] = Color.Blue
        ano2.x2[1][1][1][1] = Color.Orange
        for s in [Union_struct(long1=111,union1=ano1), Union_struct(long1=222,union1=ano2)]:
            s_o = Union_struct()
            s_o._deserialize(list(struct.unpack(Union_struct._get_packing_fmt(),s._serialize())))
            self.assertEqual(str(s),str(s_o))
            
        
if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testDefaultConstructor']
    unittest.main()