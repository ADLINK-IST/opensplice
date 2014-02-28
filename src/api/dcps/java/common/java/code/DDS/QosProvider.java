/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
package DDS;

import org.opensplice.dds.dcps.QosProviderBase;

public class QosProvider extends QosProviderBase implements DDS.QosProviderInterface
{
    public QosProvider(
            String uri,
            String profile)
    {
        /* This try-catch should preferably be removed. When a static initializer
         * is added in this class:
         *   static {
         *      System.loadLibrary("dcpssaj");
         *   }
         *
         *   public QosProvider( ....
         *
         * and we use on-load functionality of dynamic libraries to ensure the
         * library is always properly initialized, this dependency on the dpf is
         * not needed anymore. */
        try {
            jniQosProviderNew(uri, profile);
        } catch(UnsatisfiedLinkError ule){
            /* JNI library is not loaded if no instance of the
             * DomainParticipantFactory exists. */
            if(DomainParticipantFactory.get_instance() != null){
                jniQosProviderNew(uri, profile);
            }
        }
    }

    @Override
    protected void finalize()
            throws java.lang.Throwable {
        jniQosProviderFree();

        super.finalize();
    }

    @Override
    public int get_participant_qos (DDS.DomainParticipantQosHolder participantQos, String id) {
        int result;

        if(participantQos == null) {
            result =  DDS.RETCODE_BAD_PARAMETER.value;
        } else {
            DDS.NamedDomainParticipantQosHolder qos = new DDS.NamedDomainParticipantQosHolder();

            result = jniGetParticipantQos(qos, id);
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

            result = jniGetTopicQos(qos, id);
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

            result = jniGetSubscriberQos(qos, id);
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

            result = jniGetDatareaderQos(qos, id);
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

            result = jniGetPublisherQos(qos, id);
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

            result = jniGetDatawriterQos(qos, id);
            if(result == DDS.RETCODE_OK.value){
                datawriterQos.value = qos.value.datawriter_qos;
            }
        }
        return result;
    }

    private native boolean jniQosProviderNew (String uri, String profile);
    private native void jniQosProviderFree ();
    private native int jniGetParticipantQos (DDS.NamedDomainParticipantQosHolder participantQos, String id);
    private native int jniGetTopicQos (DDS.NamedTopicQosHolder topicQos, String id);
    private native int jniGetSubscriberQos (DDS.NamedSubscriberQosHolder subscriberQos, String id);
    private native int jniGetDatareaderQos (DDS.NamedDataReaderQosHolder datareaderQos, String id);
    private native int jniGetPublisherQos (DDS.NamedPublisherQosHolder publisherQos, String id);
    private native int jniGetDatawriterQos (DDS.NamedDataWriterQosHolder datawriterQos, String id);
}
