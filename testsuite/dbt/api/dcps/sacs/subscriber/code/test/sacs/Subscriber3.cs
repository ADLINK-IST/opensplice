namespace test.sacs
{
    /// <summary>Test the creation and deletion of DataReaders.</summary>
    /// <remarks>Test the creation and deletion of DataReaders.</remarks>
    public class Subscriber3 : Test.Framework.TestCase
    {
        /// <summary>Test the creation and deletion of DataReaders.</summary>
        /// <remarks>Test the creation and deletion of DataReaders.</remarks>
        public Subscriber3()
            : base("sacs_subscriber_tc3", "sacs_subscriber", "subscriber",
                "Test the creation and deletion of DataReaders.", "Test the creation and deletion of DataReaders."
                , null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPreItem(new test.sacs.SubscriberItem3Init());
            this.AddPostItem(new test.sacs.SubscriberItem3Deinit());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ISubscriber subscriber;
            DDS.ISubscriber subscriber2;
			DDS.SubscriberQos subscriberQos = null;
			DDS.DataReaderQos qos = null;
			DDS.DataReaderQos qosHolder = null;
            DDS.IDataReader reader;
            DDS.IDataReader reader2;
            DDS.IDataReader reader3;
            DDS.ITopic topic;
            DDS.ITopic otherTopic;
			DDS.TopicQos topicQos = null;
            string expResult = "DataReader creation and deletion test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            subscriberQos = (DDS.SubscriberQos)this.ResolveObject("subscriberQos");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            otherTopic = (DDS.ITopic)this.ResolveObject("otherTopic");
            topicQos = (DDS.TopicQos)this.ResolveObject("topicQos");

            rc = subscriber.GetDefaultDataReaderQos(ref qosHolder);
            qos = qosHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos.";
                return result;
            }
            reader = subscriber.CreateDataReader(null, qos);//, null, 0);
            if (reader != null)
            {
                result.Result = "Created a DataReader without a Topic (1).";
                return result;
            }
            subscriber2 = participant.CreateSubscriber(subscriberQos);//, null, 0);
            if (subscriber2 == null)
            {
                result.Result = "Could not create a new Subscriber (2).";
                return result;
            }
            rc = participant.DeleteSubscriber(subscriber2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a Subscriber (2).";
                return result;
            }
            reader = subscriber2.CreateDataReader(topic, qos);//, null, 0);
            if (reader != null)
            {
                result.Result = "Created a DataReader on a deleted Subscriber (2).";
                return result;
            }
            topic = participant.CreateTopic("subscriber3tc_topic", "my_type", topicQos);//, null
                //, 0);
            if (topic == null)
            {
                result.Result = "Could not create a new Topic (3).";
                return result;
            }
            rc = participant.DeleteTopic(topic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a Topic (3).";
                return result;
            }
            reader = subscriber.CreateDataReader(topic, qos);//, null, 0);
            if (reader != null)
            {
                result.Result = "Created a DataReader with a deleted Topic (3).";
                return result;
            }
            topic = (DDS.ITopic)this.ResolveObject("topic");
            reader = subscriber.CreateDataReader(topic, qos);//, null, 0);
            if (reader == null)
            {
                result.Result = "Could not create a DataReader (4).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but recieved " + rc + " after deleting a DataReader (5).";
                return result;
            }
            qos.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
            qos.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            reader = subscriber.CreateDataReader(topic, qos);//, null, 0);
            if (reader == null)
            {
                result.Result = "Could not create a DataReader with TRANSIENT DurabilityQosPolicy (6).";
                return result;
            }
            qos.History.Kind = DDS.HistoryQosPolicyKind.KeepLastHistoryQos;
            qos.Durability.Kind = DDS.DurabilityQosPolicyKind.PersistentDurabilityQos;
            reader2 = subscriber.CreateDataReader(otherTopic, qos);//, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create a DataReader with PERSISTENT DurabilityQosPolicy (7).";
                return result;
            }
            qos.Durability.Kind = DDS.DurabilityQosPolicyKind.VolatileDurabilityQos;
            subscriber2 = participant.CreateSubscriber(subscriberQos);//, null, 0);
            if (subscriber2 == null)
            {
                result.Result = "Could not create a Subscriber (8).";
                return result;
            }
            reader3 = subscriber2.CreateDataReader(otherTopic, qos);//, null, 0);
            if (reader3 == null)
            {
                result.Result = "Could not create a DataReader (8).";
                return result;
            }
            rc = subscriber2.DeleteDataReader(reader);
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "Could delete a DataReader on a wrong subscriber (9).";
                return result;
            }
            rc = subscriber2.DeleteDataReader(reader3);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but recieved " + rc + " after deleting a DataReader (10).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but recieved " + rc + " after deleting a DataReader (11).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but recieved " + rc + " after deleting a DataReader (12).";
                return result;
            }
            rc = participant.DeleteSubscriber(subscriber2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but recieved " + rc + " after deleting a Subscriber (13).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
