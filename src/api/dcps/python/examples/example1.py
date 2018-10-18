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
from dds import *
import time
import ddsutil

# Data available listener
class DataAvailableListener(Listener):
    def __init__(self):
        Listener.__init__(self)

    def on_data_available(self, entity):
        print('on_data_available called')
        l = entity.read(10)
        for (sd, si) in l:
            sd.print_vars()

if __name__ == "__main__":


    qp = QosProfile('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

    # Create participant
    dp = DomainParticipant(qos = qp.get_participant_qos())

    # Create publisher
    pub = dp.create_publisher(qos = qp.get_publisher_qos())

    # Create Subscriber
    sub = dp.create_subscriber(qos = qp.get_subscriber_qos())


    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('example1.idl',
                                                'basic::module_SequenceOfStruct::SequenceOfStruct_struct')

    # Type support class
    topic = gen_info.register_topic(dp, "Example1", qp.get_topic_qos())

    # Create a writer
    writer = pub.create_datawriter(topic, qp.get_writer_qos())
    readerQos = qp.get_reader_qos()
    reader = sub.create_datareader(topic, readerQos)

    Inner = gen_info.get_class("basic::module_SequenceOfStruct::Inner")
    inner1 = Inner(short1 = 999, double1=222)
    inner2 = Inner(short1 = 777, double1=333)

    # Topic data class
    s = gen_info.topic_data_class(long1=2,  seq1 = [inner1, inner2])

    # Write data
    writer.write(s)


    time.sleep(1)

    # Create waitset
    waitset = WaitSet()
    qc = QueryCondition(reader, DDSMaskUtil.all_samples(), 'long1 > 1')

    waitset.attach(qc)

    # Wait for data
    conditions = waitset.wait()


    # Print data
    l = reader.take(10)
    for sd, si in l:
        sd.print_vars()

    # dispose instance
    writer.dispose_instance(s)






