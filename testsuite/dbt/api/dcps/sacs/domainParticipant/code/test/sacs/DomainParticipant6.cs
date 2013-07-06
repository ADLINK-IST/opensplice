namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant6 : Test.Framework.TestCase
    {
        public DomainParticipant6()
            : base("sacs_domainParticipant_tc6", "sacs_domainParticipant"
                , "domainParticipant", "Subscriber test.", "Subscriber test.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.DomainParticipantFactory factory;
            string expResult = "DomainParticipant Subscriber test succeeded.";
			DDS.SubscriberQos subQosHolder = null;
			DDS.DomainParticipantQos qos = null;
            DDS.ISubscriber subscriber;
            DDS.ISubscriber subscriber2;
            DDS.ReturnCode returnCode;
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            qos = (DDS.DomainParticipantQos)this.ResolveObject("participantQos");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);

            if (participant.GetDefaultSubscriberQos(ref subQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Get default SubscriberQos failed.";
                return result;
            }
            subscriber = participant.CreateSubscriber(subQosHolder, null, 0);
            if (subscriber == null)
            {
                result.Result = "Create Subscriber failed.";
                return result;
            }
            subscriber2 = participant.CreateSubscriber(null, null, 0);
            if (subscriber2 != null)
            {
                this.testFramework.TestMessage(Test.Framework.TestMessage.Note, "See scdds213");
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                participant.DeleteContainedEntities();
                result.Result = "Create Subscriber with BAD_PARAM succeeded.";
                return result;
            }
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, qos, null, 0);
            if (participant2 == null)
            {
                result.Result = "Create Participant failed.";
                return result;
            }
            returnCode = participant2.DeleteSubscriber(subscriber);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Subscriber on wrong Participant succeeded.";
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Participant failed.";
                return result;
            }
            returnCode = participant.DeleteSubscriber(subscriber);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Subscriber failed.";
                return result;
            }
            returnCode = participant.DeleteSubscriber(subscriber);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete of already deleted Subscriber succeeded.";
                return result;
            }
            returnCode = participant.DeleteSubscriber(null);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete null Subscriber succeeded.";
                return result;
            }
            subscriber = participant.BuiltInSubscriber;
            if (subscriber == null)
            {
                result.Result = "Could not resolve builtin Subscriber.";
                return result;
            }
            subscriber2 = participant.BuiltInSubscriber;
            if (subscriber2 == null)
            {
                result.Result = "Could not resolve builtin Subscriber (2).";
                return result;
            }
            if (subscriber != subscriber2)
            {
                result.Result = "Resolved builtin Subscribers do not match.";
                return result;
            }
            returnCode = participant.DeleteSubscriber(subscriber);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete builtin Subscriber failed.";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
