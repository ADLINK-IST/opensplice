namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck6 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck6(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc6", "sacs_boundsCheck", "DataWriter.write", 
		         "Check for sequence of string elements that are null", 
		         "Check that a sequence of string does NOT contain a null pointer", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "null pointer element in a sequence of string returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.strSeq2 = new string[2];
			bce.message.strSeq2[0] = "0";
			bce.message.strSeq2[1] = null;
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "null pointer element in a sequence of string did NOT return BAD_PARAMETER.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
