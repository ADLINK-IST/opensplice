namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck5 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck5(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc5", "sacs_boundsCheck", "DataWriter.write", 
		         "Check for violation of sequence of string bounds", 
		         "Check that a sequence of string bound violation is NOT accepted", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "sequence of string bound violation returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.strSeq2 = new string[3];
			bce.message.strSeq2[0] = "0";
			bce.message.strSeq2[1] = "1";
			bce.message.strSeq2[2] = "2";
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "sequence of string bound violation did NOT return BAD_PARAMETER.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
