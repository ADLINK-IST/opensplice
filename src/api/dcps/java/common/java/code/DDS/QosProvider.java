/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

import java.io.Closeable;
import java.io.IOException;

import org.opensplice.dds.dcps.QosProviderBase;

public class QosProvider extends QosProviderBase implements DDS.QosProviderInterface, Closeable
{
    private static final long serialVersionUID = -4512906977859245929L;
    /* the QosProvider does not have a userlayer object so it does not need to extend from ObjectImpl */
    private long qosProviderObject = 0;
    static {
        DomainParticipantFactory.get_instance();
    }

    public QosProvider(
            String uri,
            String profile)
        throws NullPointerException
    {
        long qosProvider = 0;
        qosProvider = jniQosProviderNew(uri, profile);
        if (qosProvider != 0) {
            qosProviderObject = qosProvider;
        } else {
            throw new NullPointerException("Could not create QosProvider; jniQosProviderNew(...) failed.");
        }
    }

    @Override
    public int get_participant_qos (DDS.DomainParticipantQosHolder participantQos, String id) {
        int result;

        if(participantQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedDomainParticipantQosHolder qos = new DDS.NamedDomainParticipantQosHolder();

            result = jniGetParticipantQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                participantQos.value = qos.value.domainparticipant_qos;
            }
        }
        return result;
    }

    @Override
    public int get_topic_qos (DDS.TopicQosHolder topicQos, String id){
        int result;

        if(topicQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedTopicQosHolder qos = new DDS.NamedTopicQosHolder();

            result = jniGetTopicQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                topicQos.value = qos.value.topic_qos;
            }
        }
        return result;
    }

    @Override
    public int get_subscriber_qos (DDS.SubscriberQosHolder subscriberQos, String id){
        int result;

        if(subscriberQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedSubscriberQosHolder qos = new DDS.NamedSubscriberQosHolder();

            result = jniGetSubscriberQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                subscriberQos.value = qos.value.subscriber_qos;
            }
        }
        return result;
    }

    @Override
    public int get_datareader_qos (DDS.DataReaderQosHolder datareaderQos, String id){
        int result;

        if(datareaderQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedDataReaderQosHolder qos = new DDS.NamedDataReaderQosHolder();

            result = jniGetDatareaderQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                datareaderQos.value = qos.value.datareader_qos;
            }
        }
        return result;
    }

    @Override
    public int get_publisher_qos (DDS.PublisherQosHolder publisherQos, String id){
        int result;

        if(publisherQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedPublisherQosHolder qos = new DDS.NamedPublisherQosHolder();

            result = jniGetPublisherQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                publisherQos.value = qos.value.publisher_qos;
            }
        }
        return result;
    }

    @Override
    public int get_datawriter_qos (DDS.DataWriterQosHolder datawriterQos, String id){
        int result;

        if(datawriterQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedDataWriterQosHolder qos = new DDS.NamedDataWriterQosHolder();

            result = jniGetDatawriterQos(qosProviderObject, qos, id);
            if(result == DDS.RETCODE_OK.value){
                datawriterQos.value = qos.value.datawriter_qos;
            }
        }
        return result;
    }

    private native long jniQosProviderNew (String uri, String profile);
    private native void jniQosProviderFree (long qosProvider);
    private native int jniGetParticipantQos (long qosProvider, DDS.NamedDomainParticipantQosHolder participantQos, String id);
    private native int jniGetTopicQos (long qosProvider, DDS.NamedTopicQosHolder topicQos, String id);
    private native int jniGetSubscriberQos (long qosProvider, DDS.NamedSubscriberQosHolder subscriberQos, String id);
    private native int jniGetDatareaderQos (long qosProvider, DDS.NamedDataReaderQosHolder datareaderQos, String id);
    private native int jniGetPublisherQos (long qosProvider, DDS.NamedPublisherQosHolder publisherQos, String id);
    private native int jniGetDatawriterQos (long qosProvider, DDS.NamedDataWriterQosHolder datawriterQos, String id);

    @Override
    public void close() throws IOException {
        jniQosProviderFree(qosProviderObject);
        qosProviderObject = 0;
    }
}
