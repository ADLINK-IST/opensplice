namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck3 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck3(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc3", "sacs_boundsCheck", "DataWriter.write", 
		         "Check for violation of string bounds", 
		         "Check that a string bound violation is not accepted", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "string bound violation returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.name5 = "more_than_5_characters";
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "string bound violation did NOT return BAD_PARAMETER.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
