namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck999 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck999(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc999", "sacs_boundsCheck", "de-init", 
		         "de-init all entities", 
		         "Check that de-initialization is successful", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "de-initialization is successful";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			rc = bce.participant.DeleteContainedEntities();
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "delete_contained_entities failed.";
				return result;
			}
			rc = bce.factory.DeleteParticipant(bce.participant);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "delete_participant failed.";
				return result;
			}
			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
