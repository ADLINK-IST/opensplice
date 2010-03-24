namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck7 : Test.Framework.TestCase
	{
        private BoundsCheckEntities bce;
        
		public BoundsCheck7(BoundsCheckEntities bce) : 
		    base("sacs_boundsCheck_tc7", "sacs_boundsCheck", "DataWriter.write", 
		         "Check for sequence of string elements that are out of bounds", 
		         "Check that a sequence of string does NOT contain an element that is out of bounds", null)
		{
            this.bce = bce;
		}


		public override Test.Framework.TestResult Run()
		{
			DDS.ReturnCode rc;
			string expResult = "out of bounds element in a sequence of string returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult, 
                string.Empty, 
                Test.Framework.TestVerdict.Pass, 
                Test.Framework.TestVerdict.Fail);

			bce.message = new mod.boundsType();
			bce.message.strSeq2 = new string[2];
			bce.message.strSeq2[0] = "0";
			bce.message.strSeq2[1] = "123456";
			rc = bce.datawriter.Write(bce.message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "out of bounds element in a sequence of string did NOT return BAD_PARAMETER.";
				return result;
			}

			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
