namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck2 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck2(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc2", "sacs_boundsCheck", "DataWriter.write", 
		         "Check unbounded string", 
		         "Check that an unbounded string does NOT violate any bounds", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "unbounded string returns OK";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.name = "01234567890123456789012345678901234567890123456789" + 
			                   "01234567890123456789012345678901234567890123456789" +
			                   "01234567890123456789012345678901234567890123456789" +
			                   "01234567890123456789012345678901234567890123456789";
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Unbounded string did NOT return OK.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
