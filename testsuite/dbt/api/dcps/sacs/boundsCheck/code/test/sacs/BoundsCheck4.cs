namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck4 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck4(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc4", "sacs_boundsCheck", "DataWriter.write", 
		         "Check for null pointer sequence of string", 
		         "Check that a null pointer sequence of string is NOT accepted", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "null pointer sequence of string returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.name5 = null;
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "null pointer sequence of string did NOT return BAD_PARAMETER.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
