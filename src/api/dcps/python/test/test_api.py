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
from ddsutil import *
from dds import *
import unittest
import time
import xmlrunner

type_info_fmt = "basic::module_{}::{}"

class APITests(unittest.TestCase):

    def setUp(self):
        unittest.TestCase.setUp(self)
        
    def compare_result(self, obj1, obj2):
        print("Comparing result:")
        obj1.print_vars()
        obj2.print_vars()
        return self._do_compare_result(obj1, obj2)
        
    def _do_compare_result(self, obj1, obj2):
        if isinstance(obj1, TopicDataClass):
            cls1_vars = obj1.get_vars()
            cls2_vars = obj2.get_vars()
            return self._do_compare_result(list(cls1_vars.values()), list(cls2_vars.values()))
        
        if isinstance(obj1, list):
            for i in range(len(obj1)):
                if not self._do_compare_result(obj1[i], obj2[i]):
                    return False
        else:
            if not obj1 == obj2:
                return False
        return True
    
    def test_waitset(self):
        
        qp = QosProfile('file://qos/DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')
        
        # Create participant
        dp = DomainParticipant(qos=qp.get_participant_qos())
    
        # Create publisher
        pub = dp.create_publisher(qp.get_publisher_qos())
        
        # Create Subscriber
        sub = dp.create_subscriber(qp.get_subscriber_qos())

        idl_name = 'MultiArrayBasicType'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')
        
        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        
        ts = SampleTypeSupport()
        topic = Topic(dp, 'MultiArrayBasicType1', ts, qp.get_topic_qos())
        writer = DataWriter(pub, topic, qp.get_writer_qos()) 
        reader2 = DataReader(sub, topic, qp.get_reader_qos())
     
        waitset = WaitSet()
        condition = StatusCondition(reader2)
     
        waitset.attach(condition)
    
        s = Sample(long1 = 4, array1=[[[1,2],[2,3],[3,4]],[[11,22],[22,33],[33,44]]])
        writer.write(s)
        time.sleep(1)
      
        conditions = waitset.wait(DDSDuration(5))
      
        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))
            
           
    @classmethod
    def setUpClass(cls):
        super(APITests, cls).setUpClass()


if __name__ == '__main__':
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)
