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
Created on Jan 5, 2018

@author: prismtech
'''
import unittest
import os
import dds
import ddsutil

class TestInstanceOperations(unittest.TestCase):

    idl_path = os.path.join('idl', 'Shapes.idl')
    shape_type_name = 'ShapeType'

    def test_registration(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_registration')
        
        wr = dp.create_datawriter(t)
        #data = ShapeType(color='RED')
        data = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        
        h = wr.register_instance(data)
        self.assertIsNotNone(h, 'handle is None')
        self.assertIsInstance(h, int, 'handle is not int')

        wr.unregister_instance(data, h)

    def test_unregistration(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_unregistration')
        
        wr = dp.create_datawriter(t)
        #data = ShapeType(color='RED')
        data = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        data2 = ShapeType(color='BLUE',x=0,y=0,z=0,t=Inner(foo=0))
        data3 = ShapeType(color='YELLOW',x=0,y=0,z=0,t=Inner(foo=0))
        data4 = ShapeType(color='PINK',x=0,y=0,z=0,t=Inner(foo=0))
        dataNever = ShapeType(color='NEVER',x=0,y=0,z=0,t=Inner(foo=0))
        
        h = wr.register_instance(data)
        h2 = wr.register_instance(data2)
        h3 = wr.register_instance(data3)
        h4 = wr.register_instance(data4)
        self.assertIsNotNone(h, 'handle is None')
        self.assertIsInstance(h, int, 'handle is not int')

        # test expected success paths
        wr.unregister_instance(handle=h)
        wr.unregister_instance(data=data2)
        
        # test failure paths
        try:
            wr.unregister_instance()
            self.fail('should not succeed; unregistering inconsistent data')
        except dds.DDSException:
            pass

        # unregister something that's already unregistered
        try:
            wr.unregister_instance(handle=h)
            self.fail('should not succeed; duplicate unregistration')
        except dds.DDSException:
            pass
        
        # unregister something that was never registered
        try:
            wr.unregister_instance(data=dataNever)
            self.fail('should not succeed; instance never registered')
        except dds.DDSException:
            pass

        # The following are not failures, but will produce oslp-error.log entries  
        
        # unregister something where data does not match        
        wr.unregister_instance(dataNever, h4)
        
    def test_wr_instance_lookup(self):
        dp = dds.DomainParticipant()

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')

        t = gci.register_topic(dp, 'ST_test_wr_instance_lookup')

        wr = dp.create_datawriter(t)
        #data = ShapeType(color='RED')
        data = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        dataUnreg = ShapeType(color='GREEN',x=0,y=0,z=0,t=Inner(foo=0))

        h = wr.register_instance(data)

        hlookup = wr.lookup_instance(data)
        self.assertEqual(hlookup, h)

        hUnreg = wr.lookup_instance(dataUnreg)
        self.assertIsNone(hUnreg)

#     def test_wr_get_key(self):
#         dp = dds.DomainParticipant()
# 
#         gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
#         ShapeType = gci.get_class('ShapeType')
#         Inner = gci.get_class('Inner')
# 
#         t = gci.register_topic(dp, 'ST_test_wr_get_key')
# 
#         wr = dp.create_datawriter(t)
#         #data = ShapeType(color='RED')
#         data = ShapeType(color='RED',x=1,y=2,z=3,t=Inner(foo=4))
#         dataUnreg = ShapeType(color='GREEN',x=0,y=0,z=0,t=Inner(foo=0))
# 
#         h = wr.register_instance(data)
# 
#         dataLookup = wr.get_key(h)
#         self.assertEqual(dataLookup.color, data.color)
#         self.assertEqual(dataLookup.x, 0)
#         self.assertEqual(dataLookup.y, 0)
#         self.assertEqual(dataLookup.z, 0)
#         self.assertEqual(dataLookup.t.foo, 0)
#         
#         try:
#             wr.get_key(11231997)
#             self.fail('Expected exception on invalid handle')
#         except dds.DDSException:
#             pass

    def test_take_instance(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_take_instance')
        
        rd = dp.create_datareader(t)
        wr = dp.create_datawriter(t)
        
        dataR = ShapeType(color='RED',x=1,y=1,z=1,t=Inner(foo=1))
        dataG = ShapeType(color='GREEN',x=1,y=1,z=1,t=Inner(foo=1))
        
        keyR = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        
        hR = rd.lookup_instance(keyR)
        self.assertIsNone(hR)

        wr.write(dataR)
        wr.write(dataG)
        
        keyR = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        
        hR = rd.lookup_instance(keyR)
        self.assertIsInstance(hR, int)
         
        data = rd.take_instance(hR,n=2)
        self.assertEqual(1, len(data))
        found = [d.color for d, _ in data]
        self.assertIn('RED',found)
        
        #do it again
        data = rd.take_instance(hR,n=2)
        self.assertEqual(0, len(data))

    def test_read_instance(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_read_instance')
        
        rd = dp.create_datareader(t)
        wr = dp.create_datawriter(t)
        
        dataR = ShapeType(color='RED',x=1,y=1,z=1,t=Inner(foo=1))
        dataG = ShapeType(color='GREEN',x=1,y=1,z=1,t=Inner(foo=1))
        
        keyR = ShapeType(color='RED',x=0,y=0,z=0,t=Inner(foo=0))
        
        hR = rd.lookup_instance(keyR)
        self.assertIsNone(hR)

        wr.write(dataR)
        wr.write(dataG)
        
        hR = rd.lookup_instance(keyR)
        self.assertIsInstance(hR, int)
        
        data = rd.read_instance(hR,n=2)
        self.assertEqual(1, len(data))
        found = [d.color for d, _ in data]
        self.assertIn('RED',found)
        
        #do it again
        data = rd.read_instance(hR,n=2)
        self.assertEqual(1, len(data))
        found = [d.color for d, _ in data]
        self.assertIn('RED',found)

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestInstanceOperations.test_register_instance']
    unittest.main()