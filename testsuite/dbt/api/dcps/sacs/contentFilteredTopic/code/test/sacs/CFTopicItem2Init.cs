namespace test.sacs
{
    /// <summary>
    /// Initialize a ContentFilteredTopic, Publisher, Subscriber a DataReader and a
    /// DataWriter.
    /// </summary>
    /// <remarks>
    /// Initialize a ContentFilteredTopic, Publisher, Subscriber a DataReader and a
    /// DataWriter.
    /// </remarks>
    public class CFTopicItem2Init : Test.Framework.TestItem
    {
        /// <summary>
        /// Initialize a ContentFilteredTopic, Publisher, Subscriber a DataReader
        /// and a DataWriter.
        /// </summary>
        /// <remarks>
        /// Initialize a ContentFilteredTopic, Publisher, Subscriber a DataReader
        /// and a DataWriter.
        /// </remarks>
        public CFTopicItem2Init()
            : base("Initialize DataReader with ContentFilteredTopic"
                )
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            string filteredTypeName = "my_filtered_topic";
            string filterExpression = "long_1 < %0";
            //        final String expressionParameters[] = {"1", "2", "3"};
            string[] expressionParameters = new string[] { "1" };
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IContentFilteredTopic filteredTopic;
            DDS.IPublisher publisher;
            DDS.ISubscriber subscriber;
            mod.tstDataReader reader;
            mod.tstDataWriter writer;
            DDS.PublisherQos publisherQos = null;
            DDS.SubscriberQos subscriberQos = null;
            DDS.DataReaderQos dataReaderQos = null;
            DDS.DataWriterQos dataWriterQos = null;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            topic = (DDS.ITopic)testCase.ResolveObject("topic");
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            filteredTopic = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression, expressionParameters);
            if (filteredTopic == null)
            {
                result.Result = "participant.create_contentfilteredtopic failed (1).";
                return result;
            }

            if (participant.GetDefaultPublisherQos(ref publisherQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "participant.get_default_publisher_qos failed (2).";
                return result;
            }
            publisher = participant.CreatePublisher(publisherQos);//, null, 0);
            if (publisher == null)
            {
                result.Result = "participant.create_publisher failed (3).";
                return result;
            }

            if (publisher.GetDefaultDataWriterQos(ref dataWriterQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "publisher.get_default_datawriter_qos failed (4).";
                return result;
            }

            writer = publisher.CreateDataWriter(topic, dataWriterQos) as mod.tstDataWriter;
            if (writer == null)
            {
                result.Result = "could not create a tstDataWriter (5).";
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref subscriberQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "participant.get_default_subscriber_qos failed (6).";
                return result;
            }
            subscriber = participant.CreateSubscriber(subscriberQos);//, null, 0);
            if (subscriber == null)
            {
                result.Result = "participant.create_subscriber failed (7).";
                return result;
            }

            if (subscriber.GetDefaultDataReaderQos(ref dataReaderQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "subscriber.get_default_datareader_qos failed (8).";
                return result;
            }

            reader = subscriber.CreateDataReader(filteredTopic, dataReaderQos) as mod.tstDataReader;
            if (reader == null)
            {
                result.Result = "subscriber.create_datareader failed (9).";
                return result;
            }
            testCase.RegisterObject("filteredTopic", filteredTopic);
            testCase.RegisterObject("publisher", publisher);
            testCase.RegisterObject("subscriber", subscriber);
            testCase.RegisterObject("reader", reader);
            testCase.RegisterObject("dataReaderQos", dataReaderQos);
            testCase.RegisterObject("writer", writer);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
