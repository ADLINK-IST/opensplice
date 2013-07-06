namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant5 : Test.Framework.TestCase
    {
        public DomainParticipant5()
            : base("sacs_domainParticipant_tc5", "sacs_domainParticipant"
                , "domainParticipant", "Publisher test.", "Publisher test.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "DomainParticipant Publisher test succeeded.";
			DDS.PublisherQos pubQosHolder = null;
            DDS.IPublisher publisher;
            DDS.IPublisher publisher2;
            DDS.ReturnCode returnCode;
			DDS.DomainParticipantQos qos = null;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.DomainParticipantFactory factory;
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            qos = (DDS.DomainParticipantQos)this.ResolveObject("participantQos");

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Get default PublisherQos failed.";
                return result;
            }
            publisher = participant.CreatePublisher(pubQosHolder);//, null, 0);
            if (publisher == null)
            {
                result.Result = "Create Publisher failed.";
                return result;
            }
            publisher2 = participant.CreatePublisher(null, null, 0);
            if (publisher2 != null)
            {
                this.testFramework.TestMessage(Test.Framework.TestMessage.Note, "See scdds213");
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                result.Result = "Create Publisher with BAD_PARAM succeeded.";
                participant.DeleteContainedEntities();
                return result;
            }
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, qos);//, null, 0);
            if (participant2 == null)
            {
                result.Result = "Create Participant failed.";
                return result;
            }
            returnCode = participant2.DeletePublisher(publisher);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete publisher on wrong Participant succeeded.";
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Participant failed.";
                return result;
            }
            returnCode = participant.DeletePublisher(publisher);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Publisher failed.";
                return result;
            }
            returnCode = participant.DeletePublisher(publisher);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete of already deleted Publisher succeeded.";
                return result;
            }
            try
            {
                returnCode = participant.DeletePublisher(null);
                if (returnCode == DDS.ReturnCode.Ok)
                {
                    result.Result = "Delete null Publisher succeeded.";
                    return result;
                }
            }
            catch {}

            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
