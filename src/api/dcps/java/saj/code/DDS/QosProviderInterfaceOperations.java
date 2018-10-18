/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package DDS;

public interface QosProviderInterfaceOperations
{
    int get_participant_qos (DDS.DomainParticipantQosHolder participantQos, String id);
    int get_topic_qos (DDS.TopicQosHolder topicQos, String id);
    int get_subscriber_qos (DDS.SubscriberQosHolder subscriberQos, String id);
    int get_datareader_qos (DDS.DataReaderQosHolder datareaderQos, String id);
    int get_publisher_qos (DDS.PublisherQosHolder publisherQos, String id);
    int get_datawriter_qos (DDS.DataWriterQosHolder datawriterQos, String id);
} // interface QosProviderInterfaceOperations
