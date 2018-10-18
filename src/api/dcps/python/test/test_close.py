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

class CloseTests(unittest.TestCase):

    def setUp(self):
        unittest.TestCase.setUp(self)

    def _check(self, entity, closed):
        try:
            entity._check_handle()
            self.assertTrue(not closed)
        except DDSException as e:
            self.assertTrue(closed)

    def check_closed(self, entity):
        self._check(entity, True)

    def check_not_closed(self, entity):
        self._check(entity, False)

    def test_close_individual(self):
        gen_info = get_dds_classes_from_idl('idl/SimpleTypes.idl', 'basic::module_SimpleTypes::SimpleTypes_struct')
        dp = DomainParticipant()
        pub = dp.create_publisher()
        sub = dp.create_subscriber()
        topic = gen_info.register_topic(dp, 'simpletopic')
        writer = pub.create_datawriter(topic)
        reader = sub.create_datareader(topic)

        self.check_not_closed(dp)
        self.check_not_closed(pub)
        self.check_not_closed(sub)
        self.check_not_closed(topic)
        self.check_not_closed(writer)
        self.check_not_closed(reader)

        reader.close()
        self.check_closed(reader)
        sub.close()
        self.check_closed(sub)
        writer.close()
        self.check_closed(writer)
        pub.close()
        self.check_closed(pub)
        topic.close()
        self.check_closed(topic)
        dp.close()
        self.check_closed(dp)

    def test_close_individual_w_default_pub_sub(self):
        gen_info = get_dds_classes_from_idl('idl/SimpleTypes.idl', 'basic::module_SimpleTypes::SimpleTypes_struct')
        dp = DomainParticipant()
        topic = gen_info.register_topic(dp, 'simpletopic')
        writer = dp.create_datawriter(topic)
        reader = dp.create_datareader(topic)

        self.check_not_closed(dp)
        self.check_not_closed(topic)
        self.check_not_closed(writer)
        self.check_not_closed(reader)

        reader.close()
        self.check_closed(reader)
        writer.close()
        self.check_closed(writer)
        topic.close()
        self.check_closed(topic)
        dp.close()
        self.check_closed(dp)

    def test_close_recursive(self):
        gen_info = get_dds_classes_from_idl('idl/SimpleTypes.idl', 'basic::module_SimpleTypes::SimpleTypes_struct')
        dp = DomainParticipant()
        pub = dp.create_publisher()
        sub = dp.create_subscriber()
        topic = gen_info.register_topic(dp, 'simpletopic2')
        writer = pub.create_datawriter(topic)
        reader = sub.create_datareader(topic)

        dp.close()
        self.check_closed(dp)
        self.check_closed(pub)
        self.check_closed(sub)
        self.check_closed(topic)
        self.check_closed(writer)
        self.check_closed(reader)

    def test_close_recursive_w_default_pub_sub(self):
        gen_info = get_dds_classes_from_idl('idl/SimpleTypes.idl', 'basic::module_SimpleTypes::SimpleTypes_struct')
        dp = DomainParticipant()
        topic = gen_info.register_topic(dp, 'simpletopic2')
        writer = dp.create_datawriter(topic)
        reader = dp.create_datareader(topic)

        dp.close()
        self.check_closed(dp)
        self.check_closed(topic)
        self.check_closed(writer)
        self.check_closed(reader)

    @classmethod
    def setUpClass(cls):
        super(CloseTests, cls).setUpClass()


if __name__ == '__main__':
    unittest.main()
