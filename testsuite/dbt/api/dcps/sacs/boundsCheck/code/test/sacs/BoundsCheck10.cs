namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck10 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck10(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc10", "sacs_boundsCheck", "init", 
		         "Initialize some more test entities", 
		         "test entities are initialized successfully", null)
		{
            this.bce = bce;
		}

		public override Test.Framework.TestResult Run()
		{
            DDS.TopicQos tQos = null;
            DDS.DataWriterQos dwQos = null;
			DDS.ReturnCode rc;
			string expResult = "Initialization success";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.typeSupport2 = new mod.embeddedStructTypeTypeSupport();
			rc = bce.typeSupport2.RegisterType(bce.participant, "embeddedStructType");
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
			bce.topic2 = bce.participant.CreateTopic("embeddedStruct", "embeddedStructType", tQos);
			if (bce.topic2 == null)
			{
				result.Result = "Topic could not be created.";
				return result;
			}
			rc = bce.publisher.GetDefaultDataWriterQos(ref dwQos);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DataWriterQos could not be resolved.";
				return result;
			}
			bce.datawriter2 = (mod.embeddedStructTypeDataWriter)bce.publisher.CreateDataWriter(bce.topic2, dwQos);
			if (bce.datawriter2 == null)
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
