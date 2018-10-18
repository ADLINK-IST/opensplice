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
Created on Jan 8, 2018

@author: prismtech
'''
import unittest
from dds import Listener, DomainParticipant
import ddsutil
import os
import threading


class TestOSPL11503(unittest.TestCase):

    idl_path = os.path.join('idl', 'Shapes.idl')
    shape_type_name = 'ShapeType'

    def setUp(self):
        print('setUp')
#         self.dp1 = DomainParticipant()
        self.dp2 = DomainParticipant()
        print('Created event')
        self.event = threading.Event()

    def tearDown(self):
        print('tearDown')
#         self.dp1.close()
        print('dp1 closed')
        self.dp2.close()
        print('tearDown - done')

#     @unittest.skip("This test causes OSPL/Python to hang")
    def test_on_data_available(self):
        topic_name = 'ST_on_data_available'
        event = self.event

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')

        print('createing writer')
        t1 = gci.register_topic(self.dp2, topic_name)
        wr1 = self.dp2.create_datawriter(t1)

        class L(Listener):
#             def on_requested_deadline_missed(self, _, __):
#                 Listener.on_requested_deadline_missed(self)
            def on_data_available(self,_):
                print('triggering event')
                event.set()

        print('creating reader with listener')
        t2 = gci.register_topic(self.dp2, topic_name)
        rd2 = self.dp2.create_datareader(t2,listener=L())
        
        print('writing')
        data = ShapeType(color='RED',x=1,y=2,z=3,t=Inner(foo=4))
        wr1.write(data)
        try:
            print('waiting for listener to trigger')
            self.assertTrue(self.event.wait(2.0),'Did not receive on_data_available')
            print('done waiting')
        finally:
            print('closing rd2')
#             rd2.close()
            print('all done')

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()