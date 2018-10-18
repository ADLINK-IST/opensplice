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
Created on Jan 22, 2018

@author: prismtech
'''
import unittest
import dds
import ddsutil
from threading import Event

class TestOspl11530(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        cls.dp = dds.DomainParticipant()
        
    @classmethod
    def tearDownClass(cls):
        cls.dp.close()

    def testParticipant_textData(self):
        myData = 'DP Text User Data'
        builtinTopicName = 'DCPSParticipant'
        sawMyData = Event()
        dp_qos_txt = dds.Qos(policies=[dds.UserdataQosPolicy(myData)])
        dp2 = dds.DomainParticipant(qos=dp_qos_txt)
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.user_data.value).decode('ISO-8859-1')
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()
        dp2.close()

    def testParticipant_bytesData(self):
        myData = b'DP Bytes User Data'
        builtinTopicName = 'DCPSParticipant'
        sawMyData = Event()
        dp_qos_txt = dds.Qos(policies=[dds.UserdataQosPolicy(myData)])
        dp2 = dds.DomainParticipant(qos=dp_qos_txt)
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.user_data.value)
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()
        dp2.close()

    def testTopic_textData(self):
        myData = 'Topic Text Topic Data'
        builtinTopicName = 'DCPSTopic'
        sawMyData = Event()
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.topic_data.value).decode('ISO-8859-1')
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())

        # Register a topic with a TopicdataQosPolicy
        t_qos = dds.Qos(policies=[dds.TopicdataQosPolicy(myData)])
        info = ddsutil.get_dds_classes_from_idl('idl/Shapes.idl', "ShapeType")
        info.register_topic(self.dp, "T_testTopic_textData", t_qos)
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()

    def testTopic_bytesData(self):
        myData = b'Topic Bytes Topic Data'
        builtinTopicName = 'DCPSTopic'
        sawMyData = Event()
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.topic_data.value)
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())

        # Register a topic with a TopicdataQosPolicy
        t_qos = dds.Qos(policies=[dds.TopicdataQosPolicy(myData)])
        info = ddsutil.get_dds_classes_from_idl('idl/Shapes.idl', "ShapeType")
        t = info.register_topic(self.dp, "T_testTopic_bytesData", t_qos)
        self.assertIsNotNone(t, "Topic registration failed")
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()

    def testGroup_textData(self):
        myData = 'Pub Text Group Data'
        builtinTopicName = 'DCPSPublication'
        sawMyData = Event()
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.group_data.value).decode('ISO-8859-1')
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())

        # Register a topic with a TopicdataQosPolicy
        p_qos = dds.Qos(policies=[dds.GroupdataQosPolicy(myData)])
        pub = self.dp.create_publisher(p_qos);
        
        info = ddsutil.get_dds_classes_from_idl('idl/Shapes.idl', "ShapeType")
        t = info.register_topic(self.dp, "T_testGroup_textData")
        self.assertIsNotNone(t, "Topic registration failed")
        
        dw = pub.create_datawriter(t)
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()
        dw.close()
        pub.close()

    def testGroup_bytesData(self):
        myData = b'Pub Bytes Group Data'
        builtinTopicName = 'DCPSPublication'
        sawMyData = Event()
        
        class L(dds.Listener):
            def on_data_available(self, reader):
                nonlocal sawMyData
                while True:
                    r = reader.take(n = 1)
                    if len(r) == 0:
                        break
                    elif r[0].status.valid_data:
                        try:
                            v = bytes(r[0].data.group_data.value)
                            if v == myData:
                                sawMyData.set()
                                break
                        except UnicodeError:
                            pass
        
        builtin_sub = self.dp.create_subscriber(qos=dds.Qos([dds.PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        builtin_topic_info = ddsutil.find_and_register_topic(self.dp, builtinTopicName)
        builtin_reader = builtin_sub.create_datareader(builtin_topic_info.topic, qos=builtin_topic_info.topic.qos, listener=L())

        # Register a topic with a TopicdataQosPolicy
        p_qos = dds.Qos(policies=[dds.GroupdataQosPolicy(myData)])
        pub = self.dp.create_publisher(p_qos);
        
        info = ddsutil.get_dds_classes_from_idl('idl/Shapes.idl', "ShapeType")
        t = info.register_topic(self.dp, "T_testGroup_bytesData")
        self.assertIsNotNone(t, "Topic registration failed")
        
        dw = pub.create_datawriter(t)
        
        self.assertTrue(sawMyData.wait(10), 'Did not see expected data')
        builtin_reader.close()
        dw.close()
        pub.close()


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestOspl11530.testParticipant_textData']
    unittest.main()