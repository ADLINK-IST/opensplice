namespace test.sacs
{
	public class Publisher3 : Test.Framework.TestCase
	{
		/// <summary>Test the creation and deletion of a DataWriters.</summary>
		/// <remarks>Test the creation and deletion of a DataWriters.</remarks>
		public Publisher3() : base("sacs_publisher_tc3", "sacs_publisher", "publisher", "Test the creation and deletion of a DataWriters."
			, "Test the creation and deletion of a DataWriters.", null)
		{
			this.AddPreItem(new test.sacs.PublisherItemInit());
			this.AddPreItem(new test.sacs.TopicInit());
			this.AddPostItem(new test.sacs.TopicDeinit());
			this.AddPostItem(new test.sacs.PublisherItemDeinit());
		}

		public override Test.Framework.TestResult Run()
		{
			DDS.IDomainParticipant participant;
			DDS.IPublisher publisher;
			DDS.IPublisher publisher2;
			DDS.PublisherQos qos;
			DDS.IDataWriter writer;
			DDS.IDataWriter writer2;
			DDS.DataWriterQos dataWriterQosHolder;
			DDS.DataWriterQos dataWriterQos;
			DDS.ITopic topic;
			DDS.ITopic topic2;
			DDS.TopicQos defaultTopicQos;
			mod.tstTypeSupport typeSupport;
			string expResult = "Successful creation and deletion of DataWriters";
			Test.Framework.TestResult result;
			DDS.ReturnCode rc;
			result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
				Test.Framework.TestVerdict.Fail);
			participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
			publisher = (DDS.IPublisher)this.ResolveObject("publisher");
			qos = (DDS.PublisherQos)this.ResolveObject("publisherQos");
			topic = (DDS.ITopic)this.ResolveObject("topic");
			defaultTopicQos = (DDS.TopicQos)this.ResolveObject("topicQos");
			publisher.GetDefaultDataWriterQos(out dataWriterQosHolder);
			writer = publisher.CreateDataWriter(topic, ref dataWriterQosHolder, null, 0);
			if (writer == null)
			{
				result.Result = "could not create a writer (1).";
				return result;
			}
			rc = publisher.DeleteDataWriter(writer);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "could not delete a writer (2).";
				return result;
			}
			writer = publisher.CreateDataWriter(null, ref dataWriterQosHolder, null, 0);
			if (writer != null)
			{
				result.Result = "could create a writer with invalid topic (3).";
				return result;
			}
			typeSupport = new mod.tstTypeSupport();
			if (typeSupport == null)
			{
				result.Result = "Creation of tstTypeSupport failed.";
				return result;
			}
			rc = typeSupport.RegisterType(participant, "my_other_type");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Register type failed.";
				return result;
			}
			publisher2 = participant.CreatePublisher(ref qos, null, 0);
			if (publisher2 == null)
			{
				result.Result = "creation of a second publisher failed.";
				return result;
			}
			topic2 = participant.CreateTopic("my_other_topic", "my_other_type", ref defaultTopicQos
				, null, 0);
			if (topic2 == null)
			{
				result.Result = "Topic2 could not be created.";
				return result;
			}
			writer = publisher.CreateDataWriter(topic2, ref dataWriterQosHolder, null, 0);
			if (writer == null)
			{
				result.Result = "could not create a writer with another topic (4).";
				return result;
			}
			writer2 = publisher.LookupDataWriter("my_other_topic");
			if (writer2 == null)
			{
				result.Result = "could not lookup writer (5).";
				return result;
			}
			if (writer2 != writer)
			{
				result.Result = "looked-up writer != initial writer (6).";
				return result;
			}
			writer2 = null;
			writer2 = publisher.LookupDataWriter("unknown_topic_name");
			if (writer2 != null)
			{
				result.Result = "could lookup writer with unknown topic name (7).";
				return result;
			}
			rc = publisher2.DeleteDataWriter(writer);
			if (rc != DDS.ReturnCode.PreconditionNotMet)
			{
				result.Result = "Expected returncode PRECONDITION_NOT_MET instead of " + rc + " after deleting a writer on wrong publisher (8).";
				return result;
			}
			rc = publisher.DeleteDataWriter(writer);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "could not delete a writer (9).";
				return result;
			}
			rc = participant.DeleteTopic(topic2);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "could not delete a topic (10).";
				return result;
			}
			writer = publisher.CreateDataWriter(topic2, ref dataWriterQosHolder, null, 0);
			if (writer != null)
			{
				result.Result = "could create a writer with an already deleted topic (11).";
				return result;
			}
			dataWriterQos = dataWriterQosHolder;
			dataWriterQos.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
			dataWriterQos.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
			writer = publisher.CreateDataWriter(topic, ref dataWriterQos, null, 0);
			if (writer == null)
			{
				result.Result = "could not create a writer with TRANSIENT durabilityQosPolicy (12).";
				return result;
			}
			rc = publisher.DeleteDataWriter(writer);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "could not delete a writer (13).";
				return result;
			}
			dataWriterQos.Durability.Kind = DDS.DurabilityQosPolicyKind.PersistentDurabilityQos;
			writer = publisher.CreateDataWriter(topic, ref dataWriterQos, null, 0);
			if (writer == null)
			{
				result.Result = "could not create a writer with PERSISTENT durabilityQosPolicy (14).";
				return result;
			}
			rc = publisher.DeleteDataWriter(writer);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "could not delete a writer (15).";
				return result;
			}
			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
