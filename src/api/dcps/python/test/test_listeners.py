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
Created on Dec 27, 2017

@author: prismtech
'''
import unittest
from dds import Listener, DomainParticipant, Qos, DurabilityQosPolicy, DDSDurabilityKind, DDSException,\
    DeadlineQosPolicy, DDSDuration, LivelinessQosPolicy, DDSLivelinessKind,\
    OwnershipQosPolicy, DDSOwnershipKind, ResourceLimitsQosPolicy,\
    DestinationOrderQosPolicy, DDSDestinationOrderKind, DDSTime,\
    PublicationMatchedStatus, SubscriptionMatchedStatus,\
    OfferedDeadlineMissedStatus, OfferedIncompatibleQosStatus, QosPolicyId,\
    LivelinessLostStatus, LivelinessChangedStatus, RequestedDeadlineMissedStatus,\
    RequestedIncompatibleQosStatus, SampleRejectedStatus,\
    DDSSampleRejectedStatusKind, SampleLostStatus
import ddsutil
import os
import threading
import time
from symbol import nonlocal_stmt
import collections

Info = collections.namedtuple('Info', ['name', 'type'])

class TestListeners(unittest.TestCase):

    idl_path = os.path.join('idl', 'Shapes.idl')
    shape_type_name = 'ShapeType'
    time_out = 10.0

    def _check_status(self, status, type, field_info):
        self.assertIsInstance(status, type, 'status is not {}'.format(type))
        self.assertEqual(len(field_info), len(type._fields), 'incorrect number of field_info entries')
        for n, t in field_info:
            self.assertTrue(hasattr(status,n), 'status does not have attr {}'.format(n))
            self.assertIsInstance(getattr(status,n), t, 'status.{} is not a {}'.format(n,t))

    def test_on_data_available(self):
        topic_name = 'ST_on_data_available'
        event = threading.Event()

        dp1 = DomainParticipant()
        
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')

        t1 = gci.register_topic(dp1, topic_name)
        wr1 = dp1.create_datawriter(t1)

        class L(Listener):
            def on_data_available(self,_):
                event.set()

        dp2 = DomainParticipant()
        t2 = gci.register_topic(dp2, topic_name)
        rd2 = dp2.create_datareader(t2,listener=L())
        
        data = ShapeType(color='RED',x=1,y=2,z=3,t=Inner(foo=4))
        wr1.write(data)

        self.assertTrue(event.wait(self.time_out),'Did not receive on_data_available')

    def test_on_inconsistent_topic(self):
        '''
        from: osplo/testsuite/dbt/api/dcps/c99/utest/listener/code/listener_utests.c
        
        It's not that easy for OpenSplice to generate inconsistent_topic
        events. However, it is build on top of SAC and it works on that
        language binding. We can assume that this test succeeds when
        the other listener test pass as well...

        So, we will just check that the listener's actually got installed
        '''
        topic_name = 'ST_on_inconsistent_topic'
        event = threading.Event()

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
#         gci2 = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name + '2')

        class L(Listener):
            def on_inconsistent_topic(self, topic, status):
                print('on_inconsistent_topic triggered: topic name = {}, total_count = {}, total_change_count = {}'
                      .format(topic.get_name(), status.total_coutn, status.total_change_count))
                event.set()
                
        dp1 = DomainParticipant(listener=L())
        
        self.assertIsNotNone(dp1.listener, "DomainParticipant Listener was not set")

        t1 = gci.register_topic(dp1, topic_name, listener=L())

        self.assertIsNotNone(t1.listener, "Topic Listener was not set")

#         t2qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.PERSISTENT)])
#         try:
#             t2 = gci2.register_topic(dp2, topic_name, qos=None)
#             self.fail("expected this topic registeration to fail")
#         except DDSException as e:
#             pass
#         
#         try:
#             self.assertTrue(self.event.wait(self.time_out),'Did not receive on_inconsistent_topic')
#         finally:
#             pass

    def test_data_available_listeners(self):
        dp_on_data_available_event = threading.Event()
        dp_on_publication_matched_event = threading.Event()
        dp_on_subscription_matched_event = threading.Event()

        p_on_publication_matched_event = threading.Event()
        s_on_data_available_event = threading.Event()
        s_on_subscription_matched_event = threading.Event()

        wr_on_publication_matched_event = threading.Event()

        rd_on_data_available_event = threading.Event()
        rd_on_subscription_matched_event = threading.Event()
        
        opm_event = threading.Event()
        osm_event = threading.Event()
        oda_event = threading.Event()
        
        pub_match_status = None
        sub_match_status = None
        
        class DPL(Listener):
            def on_data_available(self,reader):
                dp_on_data_available_event.set()
                oda_event.set()
            def on_publication_matched(self,writer,status):
                dp_on_publication_matched_event.set()
                opm_event.set()
            def on_subscription_matched(self,reader,status):
                dp_on_subscription_matched_event.set()
                osm_event.set()
                
        class PL(Listener):
            def on_publication_matched(self,writer, status):
                p_on_publication_matched_event.set()

        class SL(Listener):
            def on_data_available(self,reader):
                s_on_data_available_event.set()
                oda_event.set()
            def on_subscription_matched(self,reader, status):
                s_on_subscription_matched_event.set()
                osm_event.set()
                
        class WL(Listener):
            def on_publication_matched(self,writer, status):
                nonlocal pub_match_status
                pub_match_status = status
                wr_on_publication_matched_event.set()
                opm_event.set()
        
        class RL(Listener):
            def on_data_available(self,reader):
                rd_on_data_available_event.set()
                oda_event.set()
            def on_subscription_matched(self,reader, status):
                nonlocal sub_match_status
                sub_match_status = status
                rd_on_subscription_matched_event.set()
                osm_event.set()
        
        dp = DomainParticipant(listener=DPL())
        self.assertIsInstance(dp.listener, DPL, 'listener is not a DPL')

        topic_name = 'ST_data_available_listeners'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)
        
        pub = dp.create_publisher(listener=PL())
        self.assertIsInstance(pub.listener, PL, 'listener is not a PL')

        sub = dp.create_subscriber(listener=SL())
        self.assertIsInstance(sub.listener, SL, 'listener is not a SL')

        wr = pub.create_datawriter(t, listener=WL())
        self.assertIsInstance(wr.listener, WL, 'listener is not a WL')

        rd = sub.create_datareader(t, listener=RL())
        self.assertIsInstance(rd.listener, RL, 'listener is not a RL')
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))
        
#         time.sleep(1.0)
        
        wr.write(data)

        
        TriggerState = collections.namedtuple('TriggerState',[
            'opm',
            'osm',
            'oda',
            ])

        actual_trigger_state = TriggerState(
            opm_event.wait(self.time_out), 
            osm_event.wait(self.time_out), 
            oda_event.wait(self.time_out))
        print(actual_trigger_state)
        
        self.assertEqual(
            actual_trigger_state,
            TriggerState(True, True, True)
            , 'Not all events triggered')

        EventState = collections.namedtuple('EventState',[
            'dp_opm',
            'p_opm',
            'wr_opm',
            'dp_osm',
            's_osm',
            'rd_osm',
            'dp_oda',
            's_oda',
            'rd_oda',
            ])
        
        actual_event_state = EventState(
            dp_on_publication_matched_event.is_set(),
            p_on_publication_matched_event.is_set(),
            wr_on_publication_matched_event.is_set(),
            dp_on_subscription_matched_event.is_set(),
            s_on_subscription_matched_event.is_set(),
            rd_on_subscription_matched_event.is_set(),
            dp_on_data_available_event.is_set(),
            s_on_data_available_event.is_set(),
            rd_on_data_available_event.is_set(),
            )
        expected_event_state = EventState(
            False, False, True,
            False, False, True,
            False, False, True,
            )
        print(actual_event_state)
        self.assertEqual(actual_event_state, expected_event_state, 'Incorrect listeners triggered')
#         time.sleep(1.0)
#         self.assertTrue(wr_on_publication_matched_event.wait(self.time_out), 'wr_on_publication_matched_event')
#         self.assertTrue(rd_on_subscription_matched_event.wait(self.time_out), 'rd_on_subscription_matched_event')
        
        self._check_status(pub_match_status, PublicationMatchedStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('current_count', int), 
            Info('current_count_change', int), 
            Info('last_subscription_handle', int),
            ])
        self._check_status(sub_match_status, SubscriptionMatchedStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('current_count', int), 
            Info('current_count_change', int), 
            Info('last_publication_handle', int),
            ])


    def test_on_offered_deadline_missed(self):
        handlerTriggered = threading.Event()
        write_time = 0.0
        delay = 0.0
        saved_status = None
        class L(Listener):
            def on_offered_deadline_missed(self, writer, status):
                nonlocal delay
                nonlocal saved_status
                handlerTriggered.set()
                saved_status = status
                delay = time.time() - write_time
        
        dp = DomainParticipant()
        
        topic_name = 'ST_on_offered_deadline_missed'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)

        wqos = Qos(policies=[
                DeadlineQosPolicy(DDSDuration(1,0))
            ])
        wr = dp.create_datawriter(t, wqos, L())
        rd = dp.create_datareader(t)
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        wr.write(data)
        write_time = time.time()
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self.assertGreaterEqual(delay, 1.0 - 0.05, 'Delay not >= 1.0s')
        self._check_status(saved_status, OfferedDeadlineMissedStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('last_instance_handle', int),
            ])


    def test_on_offered_incompatible_qos(self):
        handlerTriggered = threading.Event()
        saved_status = None
        class L(Listener):
            def on_offered_incompatible_qos(self, writer, status):
                nonlocal saved_status
                saved_status = status
                handlerTriggered.set()
        
        dp = DomainParticipant()
        
        topic_name = 'ST_on_offered_incompatible_qos'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)

        wqos = Qos(policies=[
                DurabilityQosPolicy(DDSDurabilityKind.VOLATILE)
            ])
        rqos = Qos(policies=[
                DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT)
            ])
        wr = dp.create_datawriter(t, wqos, L())
        rd = dp.create_datareader(t,rqos)
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        wr.write(data)
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self._check_status(saved_status, OfferedIncompatibleQosStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('last_policy_id', QosPolicyId),
            ])

    def test_liveliness(self):
        handlerTriggered = threading.Event()
        aliveTriggered = threading.Event()
        notaliveTriggered = threading.Event()
        write_time = 0.0
        delay = 0.0
        saved_lost_status = None
        saved_changed_status = None
        class L(Listener):
            def on_liveliness_lost(self, writer, status):
                nonlocal delay
                nonlocal saved_lost_status
                saved_lost_status = status
                handlerTriggered.set()
                delay = time.time() - write_time
        
        class RL(Listener):
            def on_liveliness_changed(self, reader, status):
                nonlocal saved_changed_status
                saved_changed_status = status
                if status.alive_count == 1:
                    aliveTriggered.set()
                else:
                    notaliveTriggered.set()

        qos = Qos(policies=[
                LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_TOPIC,
                                    DDSDuration(1,0)),
                OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE)
            ])
        dp = DomainParticipant()
        
        topic_name = 'ST_liveliness'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name, qos)

        wr = dp.create_datawriter(t, qos=qos, listener=L())
        rd = dp.create_datareader(t, qos=qos, listener=RL())
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        wr.write(data)
        write_time = time.time()
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self.assertGreaterEqual(delay, 1.0 - 0.05, 'Delay not >= 1.0s')
        self.assertTrue(aliveTriggered.wait(self.time_out), 'Alive not signaled to reader')
        self.assertTrue(notaliveTriggered.wait(self.time_out), 'Not Alive not signaled to reader')
        self._check_status(saved_lost_status, LivelinessLostStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            ])
        self._check_status(saved_changed_status, LivelinessChangedStatus, [
            Info('alive_count', int), 
            Info('not_alive_count', int), 
            Info('alive_count_change', int), 
            Info('not_alive_count_change', int), 
            Info('last_publication_handle', int), 
            ])


    def test_on_requested_deadline_missed(self):
        handlerTriggered = threading.Event()
        write_time = 0.0
        delay = 0.0
        saved_status = None
        class L(Listener):
            def on_requested_deadline_missed(self, reader, status):
                nonlocal delay
                nonlocal saved_status
                saved_status = status
                handlerTriggered.set()
                delay = time.time() - write_time
        
        dp = DomainParticipant()
        
        topic_name = 'ST_on_requested_deadline_missed'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)

        qos = Qos(policies=[
                DeadlineQosPolicy(DDSDuration(1,0))
            ])
        wr = dp.create_datawriter(t, qos)
        rd = dp.create_datareader(t, qos, L())
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        wr.write(data)
        write_time = time.time()
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self.assertGreaterEqual(delay, 1.0 - 0.05, 'Delay not >= 1.0s')
        self._check_status(saved_status, RequestedDeadlineMissedStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('last_instance_handle', int), 
            ])

    def test_on_requested_incompatible_qos(self):
        handlerTriggered = threading.Event()
        saved_status = None
        class L(Listener):
            def on_requested_incompatible_qos(self, reader, status):
                nonlocal saved_status
                saved_status = status
                handlerTriggered.set()
        
        dp = DomainParticipant()
        
        topic_name = 'ST_test_on_requested_incompatible_qos'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)

        wqos = Qos(policies=[
                DurabilityQosPolicy(DDSDurabilityKind.VOLATILE)
            ])
        rqos = Qos(policies=[
                DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT)
            ])
        wr = dp.create_datawriter(t, wqos)
        rd = dp.create_datareader(t,rqos, L())
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        wr.write(data)
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self._check_status(saved_status, RequestedIncompatibleQosStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('last_policy_id', QosPolicyId), 
            ])

    def test_on_sample_rejected(self):
        handlerTriggered = threading.Event()
        saved_status = None
        class L(Listener):
            def on_sample_rejected(self, reader, status):
                nonlocal saved_status
                saved_status = status
                handlerTriggered.set()
        
        dp = DomainParticipant()
        
        topic_name = 'ST_on_sample_rejected'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)
        
        qos = Qos(policies=[
                ResourceLimitsQosPolicy(max_samples=1)
            ])
        
        wr = dp.create_datawriter(t)
        rd = dp.create_datareader(t, qos, L())
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data1 = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))
        data2 = ShapeType(color='BLUE', x=222, y=233, z=244, t=Inner(foo=255))
        
        wr.write(data1)
        self.assertFalse(handlerTriggered.is_set(), 'Event already triggered')
        wr.write(data2)
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self._check_status(saved_status, SampleRejectedStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            Info('last_reason', DDSSampleRejectedStatusKind), 
            Info('last_instance_handle', int), 
            ])
        
    def test_on_sample_lost(self):
        handlerTriggered = threading.Event()
        saved_status = None
        
        class L(Listener):
            def on_sample_lost(self, reader, status):
                nonlocal saved_status
                saved_status = status
                handlerTriggered.set()
                
        qos = Qos(policies=[
            DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP)
            ])
        
        dp = DomainParticipant()
        
        topic_name = 'ST_on_sample_lost'

        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        
        t = gci.register_topic(dp, topic_name)
        
        wr = dp.create_datawriter(t, qos)
        rd = dp.create_datareader(t, qos, L())
        
        ShapeType = gci.get_class('ShapeType')
        Inner = gci.get_class('Inner')
        
        data1 = ShapeType(color='GREEN', x=22, y=33, z=44, t=Inner(foo=55))

        t1 = DDSTime(1000,0)
        t2 = DDSTime(1001,0)
        # write out-of-order samples
        wr.write_ts(data1, t2)
        rd.take()
        wr.write_ts(data1, t1)
        self.assertTrue(handlerTriggered.wait(self.time_out), 'Event not triggered')
        self._check_status(saved_status, SampleLostStatus, [
            Info('total_count', int), 
            Info('total_count_change', int), 
            ])
             

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()