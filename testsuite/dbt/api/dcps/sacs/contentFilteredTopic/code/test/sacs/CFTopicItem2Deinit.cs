namespace test.sacs
{
    /// <summary>
    /// Deinitialize a ContentFilteredTopic, Publisher, Subscriber a DataReader and a
    /// DataWriter.
    /// </summary>
    /// <remarks>
    /// Deinitialize a ContentFilteredTopic, Publisher, Subscriber a DataReader and a
    /// DataWriter.
    /// </remarks>
    public class CFTopicItem2Deinit : Test.Framework.TestItem
    {
        /// <summary>
        /// Deinitialize a ContentFilteredTopic, Publisher, Subscriber a DataReader
        /// and a DataWriter.
        /// </summary>
        /// <remarks>
        /// Deinitialize a ContentFilteredTopic, Publisher, Subscriber a DataReader
        /// and a DataWriter.
        /// </remarks>
        public CFTopicItem2Deinit()
            : base("Deinitialize DataReader with ContentFilteredTopic"
                )
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            //resolve reader
            //resolve writer
            //resolve contentFilteredTopic
            //resolve publisher
            //resolve subscriber
            DDS.IDomainParticipant participant;
            DDS.IContentFilteredTopic contentFilteredTopic;
            DDS.IPublisher publisher;
            DDS.ISubscriber subscriber;
            mod.tstDataReader reader;
            mod.tstDataWriter writer;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Deinitialization success", string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            contentFilteredTopic = (DDS.IContentFilteredTopic)testCase.ResolveObject("filteredTopic"
                );
            publisher = (DDS.IPublisher)testCase.ResolveObject("publisher");
            subscriber = (DDS.ISubscriber)testCase.ResolveObject("subscriber");
            reader = (mod.tstDataReader)testCase.ResolveObject("reader");
            writer = (mod.tstDataWriter)testCase.ResolveObject("writer");
            rc = publisher.DeleteDataWriter(writer);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete DataWriter.";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete DataReader.";
                return result;
            }
            rc = participant.DeleteContentFilteredTopic(contentFilteredTopic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete ContentFilteredTopic.";
                return result;
            }
            rc = participant.DeletePublisher(publisher);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete Publisher.";
                return result;
            }
            rc = participant.DeleteSubscriber(subscriber);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete Subscriber.";
                return result;
            }
            testCase.UnregisterObject("filteredTopic");
            testCase.UnregisterObject("publisher");
            testCase.UnregisterObject("subscriber");
            testCase.UnregisterObject("reader");
            testCase.UnregisterObject("writer");
            testCase.UnregisterObject("dataReaderQos");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
