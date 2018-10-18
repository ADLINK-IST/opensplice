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
            print(si.source_timestamp, end=", ")
            sd.print_vars()


if __name__ == "__main__":

    qp = QosProfile('file://DDS_VolatileQoS_All.xml', 'DDS VolatileQosProfile')

    # Create participant
    dp = DomainParticipant(qos=qp.get_participant_qos())

    # Create publisher
    pub = dp.create_publisher(qos=qp.get_publisher_qos())

    # Create Subscriber
    sub = dp.create_subscriber(qos=qp.get_subscriber_qos())


    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('example2.idl', 'basic::module_Enum2::Enum2_struct')

    # Register topic
    topic = gen_info.register_topic(dp, "Example2", qp.get_topic_qos())

    # Create a writer
    writer = pub.create_datawriter(topic, qp.get_writer_qos())
    readerQos = qp.get_reader_qos()
    reader = sub.create_datareader(topic, readerQos, DataAvailableListener())
    reader2 = sub.create_datareader(topic, readerQos)

    # Topic data class
    ColorA = gen_info.get_class("basic::module_Enum2::Color")
    ColorZ = gen_info.get_class("basic::enumZ::Color")

    s = gen_info.topic_data_class(long1 = 4, color1 = ColorA.Green, color2=ColorZ.Blue, array=[ColorZ.Red, ColorZ.Blue])


    # Write data
    writer.write(s)

    time.sleep(1)

    # Create waitset
    waitset = WaitSet()
    condition = ReadCondition(reader2, DDSMaskUtil.all_samples())

    waitset.attach(condition)

    # Wait for data
    conditions = waitset.wait()

    # Print data
    l = reader2.take(10)
    for sd, si in l:
        sd.print_vars()

