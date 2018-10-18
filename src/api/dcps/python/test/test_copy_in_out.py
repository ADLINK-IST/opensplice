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
from ddsutil import _class_dict
import unittest
import time
import xmlrunner

type_info_fmt = "basic::module_{}::{}"

class CopyInOutTests(unittest.TestCase):

    def setUp(self):
        unittest.TestCase.setUp(self)
        _class_dict.clear()

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

    def test_multi_array_basic_type(self):
        idl_name = 'MultiArrayBasicType'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        topic = gen_info.register_topic(self.dp, idl_name, self.qos)

        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = gen_info.topic_data_class(long1 = 4, array1=[[[1,2],[2,3],[3,4]],[[11,22],[22,33],[33,44]]])
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_array_of_struct(self):
        idl_name = 'ArrayOfStruct'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        Inner = gen_info.get_class("basic::module_{}::Inner".format(idl_name))
        inner1 = Inner(long1 = 999, double1=222)
        inner2 = Inner(long1 = 777, double1=333)
        s = Sample(long1=2,  array1 = [inner1, inner2], mylong1=5)
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))


    def test_array_of_struct_sequence(self):

        idl_name = 'ArrayOfStructSequence'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)


        Inner = gen_info.get_class("basic::module_{}::Inner".format(idl_name))
        inner1 = Inner(short1 = 1, double1=1)
        inner2 = Inner(short1 = 2, double1=2)
        inner3 = Inner(short1 = 3, double1=3)
        inner4 = Inner(short1 = 4, double1=4)
        inner5 = Inner(short1 = 5, double1=5)

        s = Sample(long1 = 4, array1 = [[inner1,inner2,inner3], [inner4,inner5]])

        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_array_of_simple_sequence(self):

        idl_name = 'ArrayOfSimpleSequence'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(long1 = 4, array1 = [[1,2,3], [4,5]])

        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_sequence_of_simple_array(self):

        idl_name = 'SequenceOfSimpleArray'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(long1 = 4, sequence1 = [[1,2], [3,4]])

        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_sequence_of_struct_array(self):

        idl_name = 'SequenceOfStructArray'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)


        Inner = gen_info.get_class("basic::module_{}::Inner".format(idl_name))
        inner1 = Inner(short1 = 1, double1=1)
        inner2 = Inner(short1 = 2, double1=2)
        inner3 = Inner(short1 = 3, double1=3)
        inner4 = Inner(short1 = 4, double1=4)
        inner5 = Inner(short1 = 5, double1=5)
        inner6 = Inner(short1 = 6, double1=6)

        s = Sample(long1 = 4, seq1 = [[inner1,inner2], [inner3,inner4], [inner5, inner6]])

        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_sequence_of_struct(self):

        idl_name = 'SequenceOfStruct'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        Inner = gen_info.get_class("basic::module_{}::Inner".format(idl_name))
        inner1 = Inner(short1 = 999, double1=222)
        inner2 = Inner(short1 = 777, double1=333)
        s = Sample(long1=2,  seq1 = [inner1, inner2])
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_string_types(self):
        idl_name = 'StringTypes'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(
            long1 = 4,
            string1 = 'Hello World',
            string2 = 'Five5',
            string_array = ['This', 'that', 'the other'],
            string_seq1 = ['Fee', 'Fi', 'Foe', 'Fum'],
            string_seq2 = ['Seven', 'Eleve']
            )
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_simple_types(self):
        idl_name = 'SimpleTypes'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, 'test_simple_types' + idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(
            long1 = 0x01020304,
            ulong1 = 0x04030201,
            longlong1 = 0x0102030405060708,
            ulonglong1 = 0x0807060504030201,
            float1 = 1.00,
            short1 = 0x0102,
            ushort1 = 0x0201,
            char1 = 'Z',
            octet1 = 13,
            double1 = 4.00,
            bool1 = True,
            string1 = 'Hello World!',
            )
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_enum(self):

        idl_name = 'Enum2'
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = type_info_fmt.format(idl_name, idl_name + '_struct')

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, idl_name, ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        ColorA = gen_info.get_class("basic::module_Enum2::Color")
        ColorZ = gen_info.get_class("basic::enumZ::Color")
        s = Sample(long1 = 4, color1 = ColorA.Green, color2=ColorZ.Blue, array=[ColorZ.Red, ColorZ.Blue])
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_user_idl01(self):

        idl_name = 'TestIDL2'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "VDM::SYS::SYS_ApplicationWindowState"

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, "SYS_ApplicationWindowState", ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        TCOM_AppPosition = gen_info.get_class("VDM::COM::TCOM_AppPosition")
        TCOM_AppSize = gen_info.get_class("VDM::COM::TCOM_AppSize")
        appPosition = TCOM_AppPosition(m_nX = 1, m_nY=2)
        appSize = TCOM_AppSize(m_nWidth = 3, m_nHeight = 4)
        s = Sample(m_unApplicationID = 12345678,
                   m_unOperatorID = 123,
                   m_tApplicationPosition = appPosition,
                   m_tApplicationSize = appSize,
                   m_unWindowState = 112,
                   m_sContextLevel = "hello dello")
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_user_idl02(self):

        idl_name = 'TestIDL2'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "VDM::HUMS::HUMS_UserActions"

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, "HUMS_UserActions", ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(m_unAppInstanceID = 110,
                   m_unOperatorID = 111,
                   m_unUserProfile = 112,
                   m_unOriginUserAction = 113,
                   m_unTypeUserAction = 114,
                   m_sAttributeName = "hello",
                   m_sAttributeValue = "dello")
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_basic_struct(self):

        idl_name = 'BasicStruct'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "declaration_order::S11"

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, "BasicStruct", ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        s = Sample(xb = 1,
                   za = 2,
                   ya = 3,
                   b = 4,
                   xa = 5,
                   yb = 6,
                   zb = 7,
                   yc = 8,
                   a = 9)
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_lely_example1(self):

        idl_name = 'Lely_example1'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "FarmGraph::RoutingDataResult"

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, "RoutingDataResult", ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        FarmGraphEntity = gen_info.get_class('FarmGraph::FarmGraphEntity')
        FarmGraphEdge = gen_info.get_class('FarmGraph::FarmGraphEdge')
        FarmGraphEntityType = gen_info.get_class('FarmGraph::FarmGraphEntityType')

        entity1 = FarmGraphEntity(identifier="entity1", robot_id=1,
                                  type=FarmGraphEntityType.BLOCKING_GATE)
        entity2 = FarmGraphEntity(identifier="entity2", robot_id=2,
                                  type=FarmGraphEntityType.SEGREGATION_GATE)
        entity3 = FarmGraphEntity(identifier="entity3", robot_id=3,
                                  type=FarmGraphEntityType.HIGH_PRIORITY_AREA)

        edge1 = FarmGraphEdge(entity_1_identifier="entity1", entity_2_identifier="entity2",
                              bidirectional=False, position="Forward")
        edge2 = FarmGraphEdge(entity_1_identifier="entity2", entity_2_identifier="entity3",
                              bidirectional=False, position="Backward")

        s = Sample(entities = [entity1, entity2, entity3], edges = [edge1, edge2])
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    def test_lely_example2(self):

        idl_name = 'Lely_example1'
        idl_path = '../test/idl/' + idl_name + '.idl'
        type_info = "SegregationGate"

        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleTypeSupport = gen_info.type_support_class
        ts = SampleTypeSupport()
        topic = dds.Topic(self.dp, "SegregationGate", ts, self.qos)
        writer = dds.DataWriter(self.pub, topic, self.qos)
        reader2 = dds.DataReader(self.sub, topic, self.qos)

        waitset = dds.WaitSet()
        condition = dds.StatusCondition(reader2)

        waitset.attach(condition)

        LeftRightStates = gen_info.get_class('LeftRightStates')

        s = Sample(output_identifier="output1", state=LeftRightStates.LEFT)
        writer.write(s)
        time.sleep(1)

        conditions = waitset.wait()

        l = reader2.take(1)
        sd, si = l[0]
        self.assertTrue(self.compare_result(s, sd))

    @classmethod
    def setUpClass(cls):
        super(CopyInOutTests, cls).setUpClass()
        durabilityQos = dds.DurabilityQosPolicy(dds.DDSDurabilityKind.TRANSIENT)
        historyQos = dds.HistoryQosPolicy(dds.DDSHistoryKind.KEEP_LAST)
        cls.qos = dds.Qos([durabilityQos, historyQos])
        cls.dp = dds.DomainParticipant()
        cls.pub = pub = dds.Publisher(cls.dp)
        cls.sub = dds.Subscriber(cls.dp)



if __name__ == '__main__':
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)
