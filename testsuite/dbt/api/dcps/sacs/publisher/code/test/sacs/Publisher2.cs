namespace test.sacs
{
    /// <summary>Tests the creation and deletion of an Publisher.</summary>
    /// <remarks>Tests the creation and deletion of an Publisher.</remarks>
    public class Publisher2 : Test.Framework.TestCase
    {
        /// <summary>Tests the creation and deletion of an Publisher.</summary>
        /// <remarks>Tests the creation and deletion of an Publisher.</remarks>
        public Publisher2()
            : base("sacs_publisher_tc2", "sacs_publisher", "publisher", "Test the creation and deletion of a Publisher."
                , "Test the creation and deletion of a Publisher.", null)
        {
            this.AddPreItem(new test.sacs.PublisherItemInit());
            this.AddPreItem(new test.sacs.TopicInit());
            this.AddPostItem(new test.sacs.TopicDeinit());
            this.AddPostItem(new test.sacs.PublisherItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.IPublisher publisher;
            DDS.IPublisher publisher2;
			DDS.PublisherQos qos = null;
            DDS.IDataWriter writer;
			DDS.DataWriterQos dataWriterQosHolder = null;
            DDS.ITopic topic;
            string expResult = "PublisherQos test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            qos = (DDS.PublisherQos)this.ResolveObject("publisherQos");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            rc = participant.DeletePublisher(publisher);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a publisher (1)";
                return result;
            }
            qos.Partition.Name = new string[] { "Partition1", "Partition2", "Partition3" };
            publisher2 = participant.CreatePublisher(qos);//, null, 0);
            if (publisher2 == null)
            {
                result.Result = "could not create a publisher (2)";
                return result;
            }

            publisher2.GetDefaultDataWriterQos(ref dataWriterQosHolder);
            if (topic == null)
            {
                result.Result = "topic == null";
                return result;
            }
            writer = publisher2.CreateDataWriter(topic, dataWriterQosHolder);//, null, 0);
            if (writer == null)
            {
                result.Result = "could not create a writer (3)";
                return result;
            }
            rc = participant.DeletePublisher(publisher2);
            if (rc != DDS.ReturnCode.PreconditionNotMet)
            {
                result.Result = "returncode = " + rc + ". Expected returncode 4 (PRECONDITION_NOT_MET) (4).";
                return result;
            }
            rc = publisher2.DeleteDataWriter(writer);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a datawriter (5)";
                return result;
            }
            rc = publisher2.DeleteDataWriter(writer);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "expected RETCODE_BAD_PARAMETER but received Retcode " + rc + " after deleting an already deleted datawriter (6)";
                return result;
            }
            participant2 = publisher2.GetParticipant();
            if (participant != participant2)
            {
                result.Result = "Looked up participant != participant (7)";
                return result;
            }
            rc = participant.DeletePublisher(publisher2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a publisher (8)";
                return result;
            }
            writer = publisher2.CreateDataWriter(topic, dataWriterQosHolder);//, null, 0);
            if (writer != null)
            {
                result.Result = "could create a writer on a deleted publisher (9)";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
