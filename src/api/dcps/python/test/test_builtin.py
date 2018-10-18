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

TOPIC_NAMES=['DCPSParticipant', 'DCPSTopic', 'DCPSPublication', 'DCPSSubscription',
             'CMParticipant', 'CMPublisher', 'CMSubscriber', 'CMDataWriter', 'CMDataReader', 'DCPSType']
topics=[]
dp=None

# Data available listener
class DataAvailableListener(Listener):
    def __init__(self, holder):
        Listener.__init__(self)
        self.holder = holder

    def on_data_available(self, entity):
        try:
            l = entity.read(1)
            for (sd, si) in l:
                if si.valid_data:
                    sd.print_vars()
                    self.holder.result = str(sd)
        except Exception as ex:
            self.holder.result = ex

class ResultHolder(object):

    def __init__(self, tname):
        self.result = ''
        self.tname = tname

class Test(unittest.TestCase):

    def setUp(self):
        pass


    def tearDown(self):
        pass


    def testCreateBuiltinTopics(self):
        global dp
        dp = DomainParticipant()
        for tname in TOPIC_NAMES:
            try:
                ft = dp.find_topic(tname)
                t = register_found_topic_as_local(ft)
                gen_info = get_dds_classes_for_found_topic(ft)
                topics.append(t)
            except DDSException:
                self.assertFalse('built-in topic registration threw exception')

    def testReadBuiltinTopics(self):
        sub = dp.create_subscriber(Qos([PartitionQosPolicy(['__BUILT-IN PARTITION__'])]))
        results = []
        for t in topics:
            try:
                result_holder = ResultHolder(t.name)
                results.append(result_holder)
                rd = sub.create_datareader(t,
                                           Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT)]),
                                           DataAvailableListener(result_holder))
            except Exception:
                self.assertFalse('sub.create_datareader threw exception')

        time.sleep(1)
        for result in results:
            self.assertFalse(isinstance(result_holder.result, Exception), 'Exception occurred while reading built in topic data')
            self.assertTrue(len(result_holder.result) == 0 or result.tname == 'DCPSType', 'Read results contain no data')
            # DCPSType not included due to no data available in fresh system

if __name__ == "__main__":
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)