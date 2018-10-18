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
Created on Jan 4, 2018

@author: prismtech
'''
import os
import unittest
import dds
import ddsutil

class TestReadConditions(unittest.TestCase):

    idl_path = os.path.join('idl', 'Shapes.idl')
    shape_type_name = 'ShapeType'

    def test_take_cond(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_take_cond')
        
        rd = dp.create_datareader(t)
        wr = dp.create_datawriter(t)
        
        qc = rd.create_querycondition(expression='color = %0', parameters=['RED'])
        
        dataR = ShapeType(color='RED',x=1,y=1,z=1,t=Inner(foo=1))
        dataG = ShapeType(color='GREEN',x=1,y=1,z=1,t=Inner(foo=1))
        
        wr.write(dataR)
        wr.write(dataG)
        
        data = rd.take_cond(qc,2)
        self.assertEqual(1, len(data))
        found = [d.color for d, _ in data]
        self.assertIn('RED',found)

    def test_read_cond(self):
        dp = dds.DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        t = gci.register_topic(dp, 'ST_test_take_cond')
        
        rd = dp.create_datareader(t)
        wr = dp.create_datawriter(t)
        
        qc = rd.create_querycondition(expression='x = 1')
        
        dataR = ShapeType(color='RED',x=1,y=1,z=1,t=Inner(foo=1))
        dataG = ShapeType(color='GREEN',x=1,y=1,z=1,t=Inner(foo=1))
        dataB = ShapeType(color='BLUE',x=2,y=1,z=1,t=Inner(foo=1))
        
        wr.write(dataR)
        wr.write(dataG)
        wr.write(dataB)
        
        samples = rd.read_cond(qc,3)
        self.assertEqual(2, len(samples))
        found = [s.data.color for s in samples]
        self.assertIn('RED',found)
        self.assertIn('GREEN',found)



if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestReadConditions.test_take_cond']
    unittest.main()