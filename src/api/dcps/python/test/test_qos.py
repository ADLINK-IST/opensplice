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

class QosProfileTests(unittest.TestCase):

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
    
    def test_default_xml_qos(self):
        
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
      
        conditions = waitset.wait()
      
        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))
            

    def test_volatile_xml_qos(self):
        
        qp = QosProfile('file://qos/DDS_VolatileQoS_All.xml', 'DDS VolatileQosProfile')

        # Create participant
        dp = DomainParticipant(qos=qp.get_participant_qos())
    
        # Create publisher
        pub = dp.create_publisher(qp.get_publisher_qos())
        
        # Create Subscriber
        sub = dp.create_subscriber(qp.get_subscriber_qos())
        
        idl_name = 'Lely_example1'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "SegregationGate"
                
        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = Topic(dp, "SegregationGate1", ts, qp.get_topic_qos())
        writer = DataWriter(pub, topic, qp.get_writer_qos()) 
        reader2 = DataReader(sub, topic, qp.get_reader_qos())
     
        waitset = WaitSet()
        condition = StatusCondition(reader2)
     
        waitset.attach(condition)
   
        LeftRightStates = gen_info.get_class('LeftRightStates')
        
        s = Sample(output_identifier="output1", state=LeftRightStates.LEFT)
        writer.write(s)
        time.sleep(1)
      
        conditions = waitset.wait()
      
        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))   

            
    def test_array_of_struct(self):
        qp = QosProfile('file://qos/DDS_QoS_Mismatched.xml', 'DDS DefaultQosProfile')
        
        # Create participant
        dp = DomainParticipant(qos=qp.get_participant_qos())
    
        # Create publisher
        pub = dp.create_publisher(qp.get_publisher_qos())
        
        # Create Subscriber
        sub = dp.create_subscriber(qp.get_subscriber_qos())

        idl_name = 'ArrayOfStruct'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')
        
        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        
        ts = SampleTypeSupport()
        topic = Topic(dp, 'ArrayOfStruct1', ts, qp.get_topic_qos())
        writer = DataWriter(pub, topic, qp.get_writer_qos()) 
        reader2 = DataReader(sub, topic, qp.get_reader_qos())
     
        waitset = WaitSet()
        condition = StatusCondition(reader2)
     
        waitset.attach(condition)
    
        Inner = gen_info.get_class("basic::module_{}::Inner".format(idl_name))
        inner1 = Inner(long1 = 999, double1=222)
        inner2 = Inner(long1 = 777, double1=333)
        s = Sample(long1=2,  array1 = [inner1, inner2], mylong1=5)
        writer.write(s)
        time.sleep(1)
      
        conditions = waitset.wait()
      
        l = reader2.take(1)
        
        self.assertTrue(len(l) == 0)

    def test_qos_policy(self):

        publisher_qos = Qos([PresentationQosPolicy(DDSPresentationAccessScopeKind.TOPIC, True, True), 
                            PartitionQosPolicy(['A', 'B'])])    
        subscriber_qos = Qos([PresentationQosPolicy(DDSPresentationAccessScopeKind.TOPIC, True, True), 
                            PartitionQosPolicy(['A', 'B'])])        
        # Create participant
        dp = DomainParticipant()
    
        # Create publisher
        pub = dp.create_publisher(publisher_qos)
        
        # Create Subscriber
        sub = dp.create_subscriber(subscriber_qos)
        
        # Generate python classes from IDL file
        gen_info = get_dds_classes_from_idl('idl/NestedStruct.idl', 'basic::module_NestedStruct::NestedStruct_struct')
         
        topic_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                         DurabilityServiceQosPolicy(DDSDuration(2, 500), DDSHistoryKind.KEEP_ALL, 2, 100, 100, 100),
                         DeadlineQosPolicy(DDSDuration(500)),
                         LatencyBudgetQosPolicy(DDSDuration(3000)),
                         LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT), 
                         ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()), 
                         DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                         HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                         ResourceLimitsQosPolicy(10,10,10),
                         TransportPriorityQosPolicy(700),
                         LifespanQosPolicy(DDSDuration(10, 500)), 
                         OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE),
                         TopicdataQosPolicy("TopicData")
                         ])
        
        topic = gen_info.register_topic(dp, "NestedStruct3", topic_qos)
        
        writer_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                          DeadlineQosPolicy(DDSDuration(500)),
                          LatencyBudgetQosPolicy(DDSDuration(3000)),
                          LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT),
                          ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()),
                          DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                          HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                          ResourceLimitsQosPolicy(10,10,10),
                          TransportPriorityQosPolicy(700),
                          LifespanQosPolicy(DDSDuration(10, 500)),
                          OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE), 
                          OwnershipStrengthQosPolicy(100),
                          WriterDataLifecycleQosPolicy(False),
                          UserdataQosPolicy("hello")
                          ])
    
        reader_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                          DeadlineQosPolicy(DDSDuration(500)),
                          LatencyBudgetQosPolicy(DDSDuration(3000)),
                          LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT),
                          ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()),
                          DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                          HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                          ResourceLimitsQosPolicy(10,10,10),
                          OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE), 
                          TimeBasedFilterQosPolicy(DDSDuration(2, 500)),
                          ReaderDataLifecycleQosPolicy(DDSDuration(3), DDSDuration(5)),
                          UserdataQosPolicy("dello")
                          ])
                
        writer = pub.create_datawriter(topic, writer_qos)
        reader = sub.create_datareader(topic, reader_qos)
    
        Inner = gen_info.get_class("basic::module_NestedStruct::Inner")
        inner = Inner(long1 = 1, double1 = 1.0)
        s = gen_info.topic_data_class(long1 = 1, inner1 = inner)

        waitset = WaitSet()
        condition = StatusCondition(reader)
     
        waitset.attach(condition)
        
        writer.write(s)
        
        time.sleep(1)
   
        conditions = waitset.wait()
      
        l = reader.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))
           
    def test_qos_policy_with_default_pub_sub(self):

        # Create participant
        dp = DomainParticipant()
    
        # Generate python classes from IDL file
        gen_info = get_dds_classes_from_idl('idl/NestedStruct.idl', 'basic::module_NestedStruct::NestedStruct_struct')
         
        topic_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                         DurabilityServiceQosPolicy(DDSDuration(2, 500), DDSHistoryKind.KEEP_ALL, 2, 100, 100, 100),
                         DeadlineQosPolicy(DDSDuration(500)),
                         LatencyBudgetQosPolicy(DDSDuration(3000)),
                         LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT), 
                         ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()), 
                         DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                         HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                         ResourceLimitsQosPolicy(10,10,10),
                         TransportPriorityQosPolicy(700),
                         LifespanQosPolicy(DDSDuration(10, 500)), 
                         OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE),
                         TopicdataQosPolicy("TopicData")
                         ])
        
        topic = gen_info.register_topic(dp, "NestedStruct3", topic_qos)
        
        writer_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                          DeadlineQosPolicy(DDSDuration(500)),
                          LatencyBudgetQosPolicy(DDSDuration(3000)),
                          LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT),
                          ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()),
                          DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                          HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                          ResourceLimitsQosPolicy(10,10,10),
                          TransportPriorityQosPolicy(700),
                          LifespanQosPolicy(DDSDuration(10, 500)),
                          OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE), 
                          OwnershipStrengthQosPolicy(100),
                          WriterDataLifecycleQosPolicy(False),
                          UserdataQosPolicy("hello")
                          ])
    
        reader_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                          DeadlineQosPolicy(DDSDuration(500)),
                          LatencyBudgetQosPolicy(DDSDuration(3000)),
                          LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT),
                          ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()),
                          DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                          HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                          ResourceLimitsQosPolicy(10,10,10),
                          OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE), 
                          TimeBasedFilterQosPolicy(DDSDuration(2, 500)),
                          ReaderDataLifecycleQosPolicy(DDSDuration(3), DDSDuration(5)),
                          UserdataQosPolicy("dello")
                          ])
                
        writer = dp.create_datawriter(topic, writer_qos)
        reader = dp.create_datareader(topic, reader_qos)
    
        Inner = gen_info.get_class("basic::module_NestedStruct::Inner")
        inner = Inner(long1 = 1, double1 = 1.0)
        s = gen_info.topic_data_class(long1 = 1, inner1 = inner)

        waitset = WaitSet()
        condition = StatusCondition(reader)
     
        waitset.attach(condition)
        
        writer.write(s)
        
        time.sleep(1)
   
        conditions = waitset.wait()
      
        l = reader.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))
           
    @classmethod
    def setUpClass(cls):
        super(QosProfileTests, cls).setUpClass()


if __name__ == '__main__':
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)
