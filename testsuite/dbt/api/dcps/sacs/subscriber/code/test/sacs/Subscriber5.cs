namespace test.sacs
{
    /// <summary>Test function delete_contained_entities.</summary>
    /// <remarks>Test function delete_contained_entities.</remarks>
    public class Subscriber5 : Test.Framework.TestCase
    {
        /// <summary>Test function delete_contained_entities.</summary>
        /// <remarks>Test function delete_contained_entities.</remarks>
        public Subscriber5()
            : base("sacs_subscriber_tc5", "sacs_subscriber", "subscriber",
                "Test function delete_contained_entities.", "Test function delete_contained_entities."
                , null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ISubscriber subscriber;
			DDS.DataReaderQos dataReaderQos = null;
			DDS.DataReaderQos qosHolder = null;
            DDS.IDataReader reader1;
            DDS.IDataReader reader2;
            DDS.IDataReader reader3;
            DDS.ITopic topic;
            string expResult = "delete_contained_entities test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");

            rc = subscriber.GetDefaultDataReaderQos(ref qosHolder);
            dataReaderQos = qosHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos (1).";
                return result;
            }
            reader1 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader1 == null)
            {
                result.Result = "Could not create a DataReader (2).";
                return result;
            }
            reader2 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create a DataReader (3).";
                return result;
            }
            reader3 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader3 == null)
            {
                result.Result = "Could not create a DataReader (4).";
                return result;
            }
            rc = subscriber.DeleteContainedEntities();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but received " + rc + " after calling function delete_contained_entities (5).";
                return result;
            }
            rc = participant.DeleteTopic(topic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but received " + rc + " after calling function delete_topic (6).";
                return result;
            }
            this.UnregisterObject("topic");
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
