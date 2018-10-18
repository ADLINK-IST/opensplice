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
Created on Jan 3, 2018

@author: prismtech
'''
import os
import unittest
import dds
import ddsutil
from collections import namedtuple

class TestStatusConditions(unittest.TestCase):

    idl_path = os.path.join('idl', 'Shapes.idl')
    shape_type_name = 'ShapeType'

    def test_inconsistent_topic_status(self):
        dp = dds.DomainParticipant()
        t, _ = ddsutil.find_and_register_topic(dp, 'DCPSParticipant')
        
        status = t.inconsistent_topic_status()
        self.assertIsNotNone(status, 'status is None')
        self.assertTrue(hasattr(status, 'total_count'))
        self.assertTrue(hasattr(status, 'total_count_change'))
        self.assertIsInstance(status.total_count, int)
        self.assertIsInstance(status.total_count_change, int)
        
    def test_publication_matched_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_publication_matched_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        wr = dp.create_datawriter(t)
        
        status = wr.publication_matched_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('current_count', int),
            Info('current_count_change', int),
            Info('last_subscription_handle', int)]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.PublicationMatchedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_liveliness_lost_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_liveliness_lost_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        wr = dp.create_datawriter(t)
        
        status = wr.liveliness_lost_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.LivelinessLostStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_offered_deadline_missed_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_offered_deadline_missed'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        wr = dp.create_datawriter(t)
        
        status = wr.offered_deadline_missed_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('last_instance_handle', int),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.OfferedDeadlineMissedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_offered_incompatible_qos_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_offered_incompatible_qos'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        wr = dp.create_datawriter(t)
        
        status = wr.offered_incompatible_qos_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('last_policy_id', dds.QosPolicyId),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.OfferedIncompatibleQosStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_subscription_matched_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_subscription_matched_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.subscription_matched_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('current_count', int),
            Info('current_count_change', int),
            Info('last_publication_handle', int)]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.SubscriptionMatchedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_liveliness_changed_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_liveliness_changed_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.liveliness_changed_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('alive_count', int),
            Info('not_alive_count', int),
            Info('alive_count_change', int),
            Info('not_alive_count_change', int),
            Info('last_publication_handle', int)]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.LivelinessChangedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_sample_rejected_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_sample_rejected_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.sample_rejected_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('last_reason', dds.DDSSampleRejectedStatusKind),
            Info('last_instance_handle', int),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.SampleRejectedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    def test_sample_lost_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_sample_lost_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.sample_lost_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.SampleLostStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    #requested_deadline_missed_status
    def test_requested_deadline_missed_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_requested_deadline_missed_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.requested_deadline_missed_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('last_instance_handle', int),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.RequestedDeadlineMissedStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))
        
    def test_requested_incompatible_qos_status(self):
        dp = dds.DomainParticipant()
        
        topic_name = 'ST_requested_incompatible_qos_status'
        gci = ddsutil.get_dds_classes_from_idl(self.idl_path, self.shape_type_name)
        t = gci.register_topic(dp, topic_name)

        rd = dp.create_datareader(t)
        
        status = rd.requested_incompatible_qos_status()
        Info = namedtuple('Info', ['name','type'])
        attrs = [Info('total_count', int),
            Info('total_count_change', int),
            Info('last_policy_id', dds.QosPolicyId),
            ]
        self.assertIsNotNone(status, 'status is None')
        self.assertIsInstance(status, dds.RequestedIncompatibleQosStatus)
        for a in attrs:
            self.assertTrue(hasattr(status, a.name))
            self.assertIsInstance(getattr(status, a.name), a.type, '{} is not a {}'.format(a.name, a.type))

    
if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestStatusConditions.test_inconsistent_topic_status']
    unittest.main()
