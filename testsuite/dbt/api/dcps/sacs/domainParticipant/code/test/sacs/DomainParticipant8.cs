namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant8 : Test.Framework.TestCase
    {
        public DomainParticipant8()
            : base("sacs_domainParticipant_tc8", "sacs_domainParticipant"
                , "domainParticipant", "Additional test.", "Additional test.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "Additional test succeeded.";
            DDS.ReturnCode returnCode;
            DDS.IStatusCondition sc;
            DDS.IStatusCondition sc2;
            int domainId;
            int domainId2;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.DomainParticipantFactory factory;
            DDS.DomainParticipantQos qos;
            DDS.IPublisher publisher;
			DDS.PublisherQos pholder = null;
            DDS.ISubscriber subscriber;
			DDS.SubscriberQos sholder = null;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            qos = (DDS.DomainParticipantQos)this.ResolveObject("participantQos");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            returnCode = participant.Enable();
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Enabling of DomainParticipant succeeded.";
                return result;
            }
            sc = participant.StatusCondition;
            if (sc == null)
            {
                result.Result = "get_statuscondition failed.";
                return result;
            }
            sc2 = participant.StatusCondition;
            if (sc2 == null)
            {
                result.Result = "get_statuscondition failed (2).";
                return result;
            }
            if (sc != sc2)
            {
                result.Result = "Resolved statusconditions do not match.";
                return result;
            }
            domainId = participant.DomainId;
            domainId2 = participant.DomainId;
           
            if (domainId != domainId2)
            {
                result.Result = "Get domain id does not match 2nd domain id.";
                return result;
            }
           
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, qos);//, null, 0);
            if (participant2 == null)
            {
                result.Result = "Create DomainParticipant failed.";
                return result;
            }

            participant2.GetDefaultPublisherQos(ref pholder);
            publisher = participant2.CreatePublisher(pholder);//, null, 0);
            if (publisher == null)
            {
                result.Result = "Create Publisher failed.";
                return result;
            }
            participant2.GetDefaultSubscriberQos(ref sholder);
            subscriber = participant2.CreateSubscriber(sholder);//, null, 0);
            if (subscriber == null)
            {
                result.Result = "Create Subscriber failed.";
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.PreconditionNotMet)
            {
                result.Result = "Delete DomainParticipant with contained entities succeeded.";
                return result;
            }
            returnCode = participant2.DeleteContainedEntities();
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete contained entities on DomainParticipant failed.";
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete DomainParticipant failed.";
                return result;
            }
            returnCode = participant2.IgnoreParticipant(1);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Action on deleted DomainParticipant succeeded.";
                return result;
            }
            returnCode = participant2.IgnorePublication(1);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Action on deleted DomainParticipant succeeded(2).";
                return result;
            }
            returnCode = participant2.IgnoreSubscription(1);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Action on deleted DomainParticipant succeeded(3).";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
