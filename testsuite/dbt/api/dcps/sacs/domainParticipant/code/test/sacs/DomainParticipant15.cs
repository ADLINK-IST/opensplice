namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant15 : Test.Framework.TestCase
    {
        public DomainParticipant15()
            : base("sacs_domainParticipant_tc15", "sacs_domainParticipant"
                , "domainParticipant", "Test if default publisherQos is used when PUBLISHER_QOS_DEFAULT is specified."
                , "Test if default publisherQos is used when PUBLISHER_QOS_DEFAULT is specified."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Default publisherQos is used when PUBLISHER_QOS_DEFAULT is specified.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.PublisherQos pubQosHolder = null;
			DDS.PublisherQos pubQosHolder2 = null;
            DDS.IPublisher pub;
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

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Publisher qos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            string[] name = new string[2];
            name[0] = "Publisher";
            name[1] = "QoS";
            pubQosHolder.Partition.Name = name;
            returnCode = participant.SetDefaultPublisherQos(pubQosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Setting default publisher QoS failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            pub = participant.CreatePublisher();
            if (pub == null)
            {
                result = new Test.Framework.TestResult(expResult, "Publisher could not be created."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            pub.GetQos(ref pubQosHolder2);
            if (!pubQosHolder.Partition.Name[0].Equals(pubQosHolder.Partition.Name
                [0]) || !pubQosHolder.Partition.Name[1].Equals(pubQosHolder.Partition
                .Name[1]))
            {
                result = new Test.Framework.TestResult(expResult, "Default publisher QoS is not taken when PUBLISHER_QOS_DEFAULT is specified."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = participant.DeletePublisher(pub);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deletion of Publisher failed."
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
