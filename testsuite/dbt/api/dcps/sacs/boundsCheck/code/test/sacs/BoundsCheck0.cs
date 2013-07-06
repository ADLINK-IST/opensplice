namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck0 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck0(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc0", "sacs_boundsCheck", "init", 
		         "Initialize the test entities", 
		         "test entities are initialized successfully", null)
		{
            this.bce = bce;
		}

		public override Test.Framework.TestResult Run()
		{
            DDS.DomainParticipantQos pqos = null;
            DDS.PublisherQos pubQos = null;
            DDS.TopicQos tQos = null;
            DDS.DataWriterQos dwQos = null;
			DDS.ReturnCode rc;
			string expResult = "Initialization success";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.factory = DDS.DomainParticipantFactory.Instance;
			if (bce.factory == null)
			{
				result.Result = "DomainParticipantFactory could not be initialized.";
				return result;
			}
			rc = bce.factory.GetDefaultParticipantQos(ref pqos);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DomainParticipantQos could not be resolved.";
				return result;
			}
			bce.participant = bce.factory.CreateParticipant(DDS.DomainId.Default, pqos);
			if (bce.participant == null)
			{
				result.Result = "Creation of DomainParticipant failed.";
				return result;
			}
			bce.typeSupport = new mod.boundsTypeTypeSupport();
			rc = bce.typeSupport.RegisterType(bce.participant, "boundsType");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Typesupport could not be registered.";
				return result;
			}
			rc = bce.participant.GetDefaultTopicQos(ref tQos);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default TopicQos could not be resolved.";
				return result;
			}
			bce.topic = bce.participant.CreateTopic("bounds", "boundsType", tQos);
			if (bce.topic == null)
			{
				result.Result = "Topic could not be created.";
				return result;
			}
			rc = bce.participant.GetDefaultPublisherQos(ref pubQos);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default PublisherQos could not be resolved.";
				return result;
			}
			bce.publisher = bce.participant.CreatePublisher(pubQos);
			if (bce.publisher == null)
			{
				result.Result = "Publisher could not be created.";
				return result;
			}
			rc = bce.publisher.GetDefaultDataWriterQos(ref dwQos);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DataWriterQos could not be resolved.";
				return result;
			}
			bce.datawriter = (mod.boundsTypeDataWriter)bce.publisher.CreateDataWriter(bce.topic, dwQos);
			if (bce.datawriter == null)
			{
				result.Result = "DataWriter could not be created.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
