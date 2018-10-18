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
Created on Nov 23, 2017

@author: prismtech
'''
import sys
from symbol import except_clause
sys.path.append('../dds')
from dds import DomainParticipant, Topic, Listener, Qos, DurabilityQosPolicy, DDSDurabilityKind, DDSException
import unittest
import time
import xmlrunner
import ddsutil
from threading import Event

# Data available listener
class DataAvailableListener(Listener):
    def __init__(self, holder, event):
        Listener.__init__(self)
        self.holder = holder
        self.event = event

    def on_data_available(self, entity):
        l = entity.read(10)
        for (sd, si) in l:
            if si.valid_data:
                sd.print_vars()
                self.holder.result = str(sd)
        self.event.set()

class ResultHolder(object):
    
    def __init__(self):
        self._result = ''
        
    @property
    def result(self):
        return self._result
    
    @result.setter
    def result(self, value):
        self._result = value

class Test(unittest.TestCase):


    def setUp(self):
        pass


    def tearDown(self):
        pass


    def testAccessingTopicMetaData(self):
        idl_path = 'idl/' + 'Thales' + '.idl'
        type_info = 'test::C_TopicStruct'
        gen_info = ddsutil.get_dds_classes_from_idl(idl_path, type_info)
        
        dp1 = DomainParticipant()
        tq1 = Qos([DurabilityQosPolicy(DDSDurabilityKind.PERSISTENT)])
        topic_dp1 = gen_info.register_topic(dp1, 'DP1_C_TopicStruct',tq1)
        
        self.assertEqual('DP1_C_TopicStruct', topic_dp1.name, 'Topic name not as expected')
        self.assertEqual(type_info, topic_dp1.type_name, 'Type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp1.keylist, 'Key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp1.metadescriptor, 'Meta descriptor not as expected')
        
        dp2 = DomainParticipant()
        topic_dp2 = dp2.find_topic('DP1_C_TopicStruct')
        self.assertIsNotNone(topic_dp2, 'Found topic is not None')
        self.assertEqual('DP1_C_TopicStruct', topic_dp2.name, 'Found topic name not as expected')
        self.assertEqual(type_info, topic_dp2.type_name, 'Found type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp2.keylist, 'Found key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp2.metadescriptor, 'Found meta descriptor not as expected')
        
        pub2 = dp2.create_publisher()
        sub2 = dp2.create_subscriber()
        try:
            wr2 = pub2.create_datawriter(topic_dp2)
            self.assertFalse('Expected pub2.create_datawriter to throw exception')
        except DDSException:
            pass
        try:
            rd2 = sub2.create_datareader(topic_dp2)
            self.assertFalse('Expected sub2.create_datareader to throw exception')
        except DDSException:
            pass
        
        # now register the found topic locally, and test reading and writing between the participants
        local_topic_dp2 = ddsutil.register_found_topic_as_local(topic_dp2)
        
        gen_info2 = ddsutil.get_dds_classes_for_found_topic(topic_dp2)
        self.assertIsNotNone(gen_info2, 'Returned gen_info2 is None')
        
        tq2 = topic_dp2.qos
        self.assertIsNotNone(tq2)
        
        # TODO: need get copy topic qos
        #local_topic_dp2 = register_topic_locally(topic_dp2)
#        local_topic_dp2 = gen_info2.register_topic(dp2, topic_dp2.get_name(), tq2)
        
        # Reader for dp2
        result_holder = ResultHolder()
        event = Event()
        rd2 = sub2.create_datareader(local_topic_dp2, tq2, DataAvailableListener(result_holder,event))
        
        # Writer for dp1
        pub1 = dp1.create_publisher()
        wr1 = pub1.create_datawriter(topic_dp1, tq1)
        
        # create the data
        Outer = gen_info.get_class('test::C_TopicStruct')
        Inner = gen_info.get_class('test::T_ID')
        data1 = Outer()
        data1.A_ID = Inner(A_ID=1,A_subID=12)
        data1.A_ForeignIDList = [Inner(A_ID=2,A_subID=23),Inner(A_ID=3,A_subID=34)]
        data1.value = 42
        
        wr1.write(data1)
        
        # let the listener catch up...
        self.assertTrue(event.wait(10.0), 'wait timed out')
        
        self.assertEqual(str(data1), result_holder.result, 'read and write results do not match')
        
    def testSimplifiedFindAPI(self):
        idl_path = 'idl/' + 'Thales' + '.idl'
        type_info = 'test::C_TopicStruct'
        gen_info = ddsutil.get_dds_classes_from_idl(idl_path, type_info)
        
        dp1 = DomainParticipant()
        tq1 = Qos([DurabilityQosPolicy(DDSDurabilityKind.PERSISTENT)])
        topic_dp1 = gen_info.register_topic(dp1, 'DP1_C_TopicStruct_2',tq1)
        
        self.assertEqual('DP1_C_TopicStruct_2', topic_dp1.name, 'Topic name not as expected')
        self.assertEqual(type_info, topic_dp1.type_name, 'Type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp1.keylist, 'Key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp1.metadescriptor, 'Meta descriptor not as expected')
        
        dp2 = DomainParticipant()
        
        topic_dp2, gen_info2 = ddsutil.find_and_register_topic(dp2, 'DP1_C_TopicStruct_2')
        self.assertIsNotNone(topic_dp2, 'Found topic is not None')
        self.assertEqual('DP1_C_TopicStruct_2', topic_dp2.name, 'Found topic name not as expected')
        self.assertEqual(type_info, topic_dp2.type_name, 'Found type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp2.keylist, 'Found key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp2.metadescriptor, 'Found meta descriptor not as expected')
        
        sub2 = dp2.create_subscriber()
        
        # now register the found topic locally, and test reading and writing between the participants
        self.assertIsNotNone(gen_info2, 'Returned gen_info2 is None')
        
        tq2 = topic_dp2.qos
        self.assertIsNotNone(tq2)
        
        # TODO: need get copy topic qos
        #local_topic_dp2 = register_topic_locally(topic_dp2)
#        local_topic_dp2 = gen_info2.register_topic(dp2, topic_dp2.get_name(), tq2)
        
        # Reader for dp2
        result_holder = ResultHolder()
        event = Event()
        rd2 = sub2.create_datareader(topic_dp2, tq2, DataAvailableListener(result_holder, event))
        
        # Writer for dp1
        pub1 = dp1.create_publisher()
        wr1 = pub1.create_datawriter(topic_dp1, tq1)
        
        # create the data
        Outer = gen_info.get_class('test::C_TopicStruct')
        Inner = gen_info.get_class('test::T_ID')
        data1 = Outer()
        data1.A_ID = Inner(A_ID=21,A_subID=212)
        data1.A_ForeignIDList = [Inner(A_ID=22,A_subID=223),Inner(A_ID=23,A_subID=234)]
        data1.value = 242
        
        wr1.write(data1)
        
        # let the listener catch up...
        self.assertTrue(event.wait(10.0), 'wait timed out')
        
        self.assertEqual(str(data1), result_holder.result, 'read and write results do not match')
        
    def testSimplifiedFindAPIWithNamedTuple(self):
        idl_path = 'idl/' + 'Thales' + '.idl'
        type_info = 'test::C_TopicStruct'
        gen_info = ddsutil.get_dds_classes_from_idl(idl_path, type_info)
        
        dp1 = DomainParticipant()
        tq1 = Qos([DurabilityQosPolicy(DDSDurabilityKind.PERSISTENT)])
        topic_dp1 = gen_info.register_topic(dp1, 'DP1_C_TopicStruct_3',tq1)
        
        self.assertEqual('DP1_C_TopicStruct_3', topic_dp1.name, 'Topic name not as expected')
        self.assertEqual(type_info, topic_dp1.type_name, 'Type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp1.keylist, 'Key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp1.metadescriptor, 'Meta descriptor not as expected')
        
        dp2 = DomainParticipant()
        
        topic_info = ddsutil.find_and_register_topic(dp2, 'DP1_C_TopicStruct_3')

        self.assertIsNotNone(topic_info.topic, 'Found topic is not None')
        self.assertEqual('DP1_C_TopicStruct_3', topic_info.topic.name, 'Found topic name not as expected')
        self.assertEqual(type_info, topic_info.topic.type_name, 'Found type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_info.topic.keylist, 'Found key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_info.topic.metadescriptor, 'Found meta descriptor not as expected')
        
        sub2 = dp2.create_subscriber()
        
        # now register the found topic locally, and test reading and writing between the participants
        self.assertIsNotNone(topic_info.gen_info, 'Returned topic_info.gen_info is None')
        
        tq2 = topic_info.topic.qos
        self.assertIsNotNone(tq2)
        
        # TODO: need get copy topic qos
        #local_topic_dp2 = register_topic_locally(topic_dp2)
#        local_topic_dp2 = gen_info2.register_topic(dp2, topic_dp2.get_name(), tq2)
        
        # Reader for dp2
        result_holder = ResultHolder()
        event = Event()
        rd2 = sub2.create_datareader(topic_info.topic, tq2, DataAvailableListener(result_holder, event))
        
        # Writer for dp1
        pub1 = dp1.create_publisher()
        wr1 = pub1.create_datawriter(topic_dp1, tq1)
        
        # create the data
        Outer = gen_info.get_class('test::C_TopicStruct')
        Inner = gen_info.get_class('test::T_ID')
        data1 = Outer()
        data1.A_ID = Inner(A_ID=31,A_subID=312)
        data1.A_ForeignIDList = [Inner(A_ID=32,A_subID=323),Inner(A_ID=33,A_subID=334)]
        data1.value = 342
        
        wr1.write(data1)
        
        # let the listener catch up...
        self.assertTrue(event.wait(10.0), 'wait timed out')
        
        self.assertEqual(str(data1), result_holder.result, 'read and write results do not match')
        
    def testSimplifiedFindAPIWithNamedTupleAndDefaultPubSub(self):
        idl_path = 'idl/' + 'Thales' + '.idl'
        type_info = 'test::C_TopicStruct'
        gen_info = ddsutil.get_dds_classes_from_idl(idl_path, type_info)
        
        dp1 = DomainParticipant()
        tq1 = Qos([DurabilityQosPolicy(DDSDurabilityKind.PERSISTENT)])
        topic_dp1 = gen_info.register_topic(dp1, 'DP1_C_TopicStruct_4',tq1)
        
        self.assertEqual('DP1_C_TopicStruct_4', topic_dp1.name, 'Topic name not as expected')
        self.assertEqual(type_info, topic_dp1.type_name, 'Type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_dp1.keylist, 'Key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_dp1.metadescriptor, 'Meta descriptor not as expected')
        
        dp2 = DomainParticipant()
        
        topic_info = ddsutil.find_and_register_topic(dp2, 'DP1_C_TopicStruct_4')

        self.assertIsNotNone(topic_info.topic, 'Found topic is not None')
        self.assertEqual('DP1_C_TopicStruct_4', topic_info.topic.name, 'Found topic name not as expected')
        self.assertEqual(type_info, topic_info.topic.type_name, 'Found type name not as expected')
        self.assertEqual('A_ID.A_ID,A_ID.A_subID', topic_info.topic.keylist, 'Found key list not as expected')
        self.assertEqual('<MetaData version="1.0.0"><Module name="test"><Struct name="T_ID"><Member name="A_ID"><Long/></Member><Member name="A_subID"><Long/></Member></Struct><TypeDef name="T_IDList"><Sequence><Type name="T_ID"/></Sequence></TypeDef><Struct name="C_TopicStruct"><Member name="A_ID"><Type name="T_ID"/></Member><Member name="A_ForeignIDList"><Type name="T_IDList"/></Member><Member name="value"><Long/></Member></Struct></Module></MetaData>', 
                         topic_info.topic.metadescriptor, 'Found meta descriptor not as expected')
        
        # now register the found topic locally, and test reading and writing between the participants
        self.assertIsNotNone(topic_info.gen_info, 'Returned topic_info.gen_info is None')
        
        tq2 = topic_info.topic.qos
        self.assertIsNotNone(tq2)
        
        # TODO: need get copy topic qos
        #local_topic_dp2 = register_topic_locally(topic_dp2)
#        local_topic_dp2 = gen_info2.register_topic(dp2, topic_dp2.get_name(), tq2)
        
        # Reader for dp2
        result_holder = ResultHolder()
        event = Event()
        rd2 = dp2.create_datareader(topic_info.topic, tq2, DataAvailableListener(result_holder, event))
        
        # Writer for dp1
        wr1 = dp1.create_datawriter(topic_dp1, tq1)
        
        # create the data
        Outer = gen_info.get_class('test::C_TopicStruct')
        Inner = gen_info.get_class('test::T_ID')
        data1 = Outer()
        data1.A_ID = Inner(A_ID=41,A_subID=412)
        data1.A_ForeignIDList = [Inner(A_ID=42,A_subID=423),Inner(A_ID=43,A_subID=434)]
        data1.value = 442
        
        wr1.write(data1)
        
        # let the listener catch up...
        self.assertTrue(event.wait(10.0), 'wait timed out')
        
        self.assertEqual(str(data1), result_holder.result, 'read and write results do not match')
        
        


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testAccessingTopicMetaData']
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)