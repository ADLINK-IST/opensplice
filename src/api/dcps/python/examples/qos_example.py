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
from ddsutil import *
import time


if __name__ == "__main__":


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
    gen_info = get_dds_classes_from_idl('example3.idl', 'basic::module_NestedStruct::NestedStruct_struct')

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
                     OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE)
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
                      WriterDataLifecycleQosPolicy(False)
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
                      ReaderDataLifecycleQosPolicy(DDSDuration(3), DDSDuration(5))
                      ])

    writer = pub.create_datawriter(topic, writer_qos)
    reader = sub.create_datareader(topic, reader_qos)

    Inner = gen_info.get_class("basic::module_NestedStruct::Inner")
    inner = Inner(long1 = 1, double1 = 1.0)
    s = gen_info.topic_data_class(long1 = 1, inner1 = inner)

    # Write data
    writer.write(s)

    time.sleep(2)

    # Create waitset
    waitset = WaitSet()
    condition = StatusCondition(reader)

    waitset.attach(condition)

    # Wait for data
    conditions = waitset.wait()

    # Print data
    l = reader.take(10)
    for sd, si in l:
        sd.print_vars()

