namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Topic1 : Test.Framework.TestCase
    {
        public Topic1()
            : base("sacs_topic_tc1", "sacs_topic", "topic", "Test if Topic functions work correctly."
                , "Test if Topic functions work correctly.", null)
        {
            this.AddPreItem(new test.sacs.TopicItemInit());
            this.AddPostItem(new test.sacs.TopicItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ITopic topic;
            DDS.TopicQos qos;
            DDS.TopicQos qos2;
            string expResult = "Topic test succeeded";
			DDS.TopicQos topQosHolder = null;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string typeName;
            string typeName2;
            string name;
            string name2;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            topic = (DDS.ITopic)this.ResolveObject("topic");
            qos2 = (DDS.TopicQos)this.ResolveObject("topicQos");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            name = (string)this.ResolveObject("topicName");
            typeName = (string)this.ResolveObject("topicTypeName");

            rc = topic.GetQos(ref topQosHolder);
            qos = topQosHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Get TopicQos failed.";
                return result;
            }
            if (!test.sacs.TopicQosComparer.TopicQosEqual(qos, qos2))
            {
                result.Result = "TopicQosses not equal.";
                return result;
            }
            qos.Deadline.Period.NanoSec = 1234;
            qos.Deadline.Period.Sec = 9;
            rc = topic.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set TopicQos failed (2).";
                return result;
            }
            topic.GetQos(ref topQosHolder);
            qos2 = topQosHolder;
            if (!test.sacs.TopicQosComparer.TopicQosEqual(qos, qos2))
            {
                result.Result = "TopicQosses not equal (2).";
                return result;
            }
            name2 = topic.Name;
            if (!name.Equals(name2))
            {
                result.Result = "Resolved Topic name is not correct. ('" + name + "' != '" + name2
                     + "')";
                return result;
            }
            participant2 = topic.Participant;
            if (participant != participant2)
            {
                result.Result = "Resolved DomainParticipant is not correct.";
                return result;
            }
            typeName2 = topic.TypeName;
            if (!typeName.Equals(typeName2))
            {
            result.Result = "Resolved Topic type name is not correct. ('" + typeName + "' != '"
                     + typeName2 + "')";
                System.Console.Out.WriteLine("NOTE\t\t: See STR/CP: TH020");
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
