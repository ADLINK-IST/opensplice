namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck9 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck9(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc9", "sacs_boundsCheck", "DataWriter.write", 
		         "Check that unbounded sequence of octet", 
		         "Check that an unbounded sequence of octet does NOT violate any bounds", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "unbounded sequence of octet returns OK";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.octSeq = new byte[1000000];
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Unbounded sequence of octet did NOT return OK.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
