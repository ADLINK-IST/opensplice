namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant14 : Test.Framework.TestCase
    {
        public DomainParticipant14()
            : base("sacs_domainParticipant_tc14", "sacs_domainParticipant"
                , "domainParticipant", "Test if default subscriberQos is used when SUBSCRIBER_QOS_DEFAULT is specified."
                , "Test if default subscriberQos is used when SUBSCRIBER_QOS_DEFAULT is specified."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Default subscriberQos is used when SUBSCRIBER_QOS_DEFAULT is specified.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.SubscriberQos subQosHolder = null;
			DDS.SubscriberQos subQosHolder2 = null;
            DDS.ISubscriber sub;
            DDS.ReturnCode returnCode;
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "Creation of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref subQosHolder) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Subscriber qos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            string[] name = new string[2];
            name[0] = "Subscriber";
            name[1] = "QoS";
            subQosHolder.Partition.Name = name;
            returnCode = participant.SetDefaultSubscriberQos(subQosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Setting default subscriber QoS failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            sub = participant.CreateSubscriber();
            if (sub == null)
            {
                result = new Test.Framework.TestResult(expResult, "Subscriber could not be created."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            sub.GetQos(ref subQosHolder2);
            if (!subQosHolder.Partition.Name[0].Equals(subQosHolder.Partition.Name
                [0]) || !subQosHolder.Partition.Name[1].Equals(subQosHolder.Partition
                .Name[1]))
            {
                result = new Test.Framework.TestResult(expResult, "Default subscriber QoS is not taken when SUBSCRIBER_QOS_DEFAULT is specified."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = participant.DeleteSubscriber(sub);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deletion of Subscriber failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deletion of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
