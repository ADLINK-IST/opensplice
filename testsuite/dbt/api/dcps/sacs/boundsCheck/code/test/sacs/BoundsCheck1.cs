namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck1 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck1(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc1", "sacs_boundsCheck", "DataWriter.write",
                 "Check for null pointer strings",
                 "Check that null pointer strings are not accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "null pointer string returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult,
                string.Empty,
                Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);

            if (result.Result != string.Empty)
            {
                return result;
            }

            bce.message = new mod.boundsType();
            bce.message.name = null;
            rc = bce.datawriter.Write(bce.message, 0);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "null pointer string did NOT return BAD_PARAMETER.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
