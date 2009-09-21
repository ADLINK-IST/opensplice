namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant17 : Test.Framework.TestCase
    {
        public DomainParticipant17()
            : base("sacs_domainParticipant_tc17", "sacs_domainParticipant"
                , "domainParticipant contains_entity", "Test contains_entity", "Check contains_entity returns the correct result"
                , null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "contains_entity returns the correct result";
            DDS.ReturnCode returnCode;
            DDS.IDomainParticipant participant;
            DDS.DomainParticipantFactory factory;
            DDS.TopicQos tHolder = new DDS.TopicQos();
            DDS.PublisherQos pHolder = new DDS.PublisherQos();
            DDS.SubscriberQos sHolder = new DDS.SubscriberQos();
            DDS.DataWriterQos wHolder = new DDS.DataWriterQos();
            DDS.DataReaderQos rHolder = new DDS.DataReaderQos();
            DDS.ITopic topic;
            DDS.IPublisher publisher;
            DDS.ISubscriber subscriber;
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            long handle;
            DDS.ReturnCode rc;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            if (participant.ContainsEntity(0))
            {
                result.Result = "contains_entity with nil handle incorrect";
                return result;
            }
            if (participant.ContainsEntity(100))
            {
                result.Result = "contains_entity with incorrect handle incorrect";
                return result;
            }

            handle = participant.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle returned 0";
                return result;
            }
            if (participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with own handle incorrect";
                return result;
            }
            mod.tstTypeSupport typeSupport = new mod.tstTypeSupport();
            rc = typeSupport.RegisterType(participant, "type1");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }
            participant.GetDefaultTopicQos(out tHolder);
            topic = participant.CreateTopic("TestTopic", "type1", ref tHolder, null, 0);
            if (topic == null)
            {
                result.Result = "Create Topic failed.";
                return result;
            }
            participant.GetDefaultPublisherQos(out pHolder);
            publisher = participant.CreatePublisher(ref pHolder, null, 0);
            if (publisher == null)
            {
                result.Result = "Create Publisher failed.";
                return result;
            }
            participant.GetDefaultSubscriberQos(out sHolder);
            subscriber = participant.CreateSubscriber(ref sHolder, null, 0);
            if (subscriber == null)
            {
                result.Result = "Create Subscriber failed.";
                return result;
            }
            publisher.GetDefaultDataWriterQos(out wHolder);
            writer = publisher.CreateDataWriter(topic, ref wHolder, null, 0) as mod.tstDataWriter;
            if (writer == null)
            {
                result.Result = "Create Writer failed.";
                return result;
            }

            subscriber.GetDefaultDataReaderQos(out rHolder);
            reader = subscriber.CreateDataReader(topic, ref rHolder, null, 0) as mod.tstDataReader;
            if (reader == null)
            {
                result.Result = "Create Reader failed.";
                return result;
            }
            handle = topic.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle (topic) returned 0";
                return result;
            }
            if (!participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with topic handle incorrect";
                return result;
            }
            handle = publisher.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle (publisher) returned != 0";
                return result;
            }
            if (!participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with publisher handle incorrect";
                return result;
            }
            handle = subscriber.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle (subscriber) returned != 0";
                return result;
            }
            if (!participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with subscriber handle incorrect";
                return result;
            }
            handle = writer.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle (writer) returned 0";
                return result;
            }
            if (!participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with writer handle incorrect";
                return result;
            }
            handle = reader.GetInstanceHandle();
            if (handle == 0)
            {
                result.Result = "get_instance_handle (reader) returned 0";
                return result;
            }
            if (!participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity with reader handle incorrect";
                return result;
            }
            rc = participant.DeleteContainedEntities();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_contained_entities failed.";
                return result;
            }
            if (participant.ContainsEntity(handle))
            {
                result.Result = "contains_entity of deleted reader incorrect";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
