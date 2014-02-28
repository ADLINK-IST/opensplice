namespace test.sacs
{
    /// <summary>Tests if defaults are used when reading QoSs with policies set to default</summary>
    public class QosProviderDefaults : Test.Framework.TestCase
    {
        public string uri = "";

        public QosProviderDefaults(string uri)
            : base("sacs_qosProvider_tc2", "qosProvider", "qosProvider", "test the attach_condition and the get_conditions functions"
                , "conditions", null)
        {
            this.uri = uri + "/defaults.xml";
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            DDS.QosProvider qosProvider = null;
            DDS.DomainParticipantQos participantQos =
                test.sacs.QosComparer.defaultDomainParticipantQos;
            DDS.TopicQos topicQos =
                test.sacs.QosComparer.defaultTopicQos;
            DDS.SubscriberQos subscriberQos =
                test.sacs.QosComparer.defaultSubscriberQos;
            DDS.DataReaderQos readerQos =
                test.sacs.QosComparer.defaultDataReaderQos;
            DDS.PublisherQos publisherQos =
                test.sacs.QosComparer.defaultPublisherQos;
            DDS.DataWriterQos writerQos =
                test.sacs.QosComparer.defaultDataWriterQos;

            DDS.DomainParticipantQos xmlParticipantQos = new DDS.DomainParticipantQos ();
            DDS.TopicQos xmlTopicQos = new DDS.TopicQos ();
            DDS.SubscriberQos xmlSubscriberQos = new DDS.SubscriberQos ();
            DDS.DataReaderQos xmlReaderQos = new DDS.DataReaderQos ();
            DDS.PublisherQos xmlPublisherQos = new DDS.PublisherQos ();
            DDS.DataWriterQos xmlWriterQos = new DDS.DataWriterQos ();

            qosProvider = new DDS.QosProvider (this.uri, null);

            if (qosProvider.GetParticipantQos (ref xmlParticipantQos, null) != 0 ||
                qosProvider.GetTopicQos (ref xmlTopicQos, null) != 0 ||
                qosProvider.GetSubscriberQos (ref xmlSubscriberQos, null) != 0 ||
                qosProvider.GetDataReaderQos (ref xmlReaderQos, null) != 0 ||
                qosProvider.GetPublisherQos (ref xmlPublisherQos, null) != 0 ||
                qosProvider.GetDataWriterQos (ref xmlWriterQos, null) != 0)
            {
                result = new Test.Framework.TestResult (
                    "QosProvider should successfully parse XML file",
                    "QosProvider did not parse XML file " + uri,
                    Test.Framework.TestVerdict.Pass,
                    Test.Framework.TestVerdict.Fail);
            } else if (!QosComparer.DomainParticipantQosEquals (xmlParticipantQos, participantQos) ||
                       !QosComparer.TopicQosEquals (xmlTopicQos, topicQos) ||
                       !QosComparer.SubscriberQosEquals (xmlSubscriberQos, subscriberQos) ||
                       !QosComparer.DataReaderQosEquals (xmlReaderQos, readerQos) ||
                       !QosComparer.PublisherQosEquals (xmlPublisherQos, publisherQos) ||
                       !QosComparer.DataWriterQosEquals (xmlWriterQos, writerQos))
            {
                result = new Test.Framework.TestResult (
                    "QosProvider should successfully parse XML file",
                    "QosProvider did not correctly parse XML file " + uri,
                    Test.Framework.TestVerdict.Pass,
                    Test.Framework.TestVerdict.Fail);
            } else {
                result = new Test.Framework.TestResult (
                    "QosProvider should successfully parse XML file",
                    "QosProvider correctly parsed XML file " + uri,
                    Test.Framework.TestVerdict.Pass,
                    Test.Framework.TestVerdict.Pass);
            }

            return result;
        }
    }
}
