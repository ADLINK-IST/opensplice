namespace test.sacs
{
    /// <date>Sep 5, 2009</date>
    public class Builtin1 : Test.Framework.TestCase
    {
        public Builtin1()
            : base("sacs_builtin_tc1", "sacs_builtin", "sacs_builtin", "test builtin topics"
                , "test builtin topics", null)
        {
            this.AddPreItem(new test.sacs.BuiltinInit());
            this.AddPostItem(new test.sacs.BuiltinDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.ITopic topic2;
            Test.Framework.TestResult result;
            string expResult = "Builtin topic test succeeded.";
            result = new Test.Framework.TestResult(
                    expResult, 
                    string.Empty, 
                    Test.Framework.TestVerdict.Pass, 
                    Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)participant.FindTopic("DCPSParticipant", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic DCPSParticipant could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("DCPSParticipant");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic DCPSParticipant could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for DCPSParticipant.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("DCPSPublication", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic DCPSPublication could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("DCPSPublication");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic DCPSPublication could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for DCPSPublication.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("DCPSSubscription", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic DCPSSubscription could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("DCPSSubscription");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic DCPSSubscription could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for DCPSSubscription.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("DCPSTopic", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic DCPSTopic could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("DCPSTopic");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic DCPSTopic could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for DCPSTopic.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("CMParticipant", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic CMParticipant could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("CMParticipant");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic CMParticipant could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for CMParticipant.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("CMPublisher", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic CMPublisher could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("CMPublisher");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic CMPublisher could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for CMPublisher.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("CMSubscriber", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic CMSubscriber could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("CMSubscriber");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic CMSubscriber could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for CMSubscriber.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("CMDataWriter", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic CMDataWriter could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("CMDataWriter");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic CMDataWriter could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for CMDataWriter.";
                return result;
            }
            topic = (DDS.ITopic)participant.FindTopic("CMDataReader", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic CMDataReader could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("CMDataReader");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic CMDataReader could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for CMDataReader.";
                return result;
            }

            topic = (DDS.ITopic)participant.FindTopic("DCPSType", DDS.Duration.Infinite);
            if (topic == null)
            {
                result.Result = "Builtin Topic DCPSType could not be found.";
                return result;
            }
            topic2 = (DDS.ITopic)participant.LookupTopicDescription("DCPSType");
            if (topic2 == null)
            {
                result.Result = "Builtin Topic DCPSType could not be found(2).";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Resolved topics do not match for DCPSType.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
