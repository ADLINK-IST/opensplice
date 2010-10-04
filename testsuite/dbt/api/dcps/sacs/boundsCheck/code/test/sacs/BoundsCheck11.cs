namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck11 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck11(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc11", "sacs_boundsCheck", "DataWriter.write",
                 "Check for correct null pointer structs",
                 "Check that null pointer structs are not accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "null pointer struct returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult,
                string.Empty,
                Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);

            if (result.Result != string.Empty)
            {
                return result;
            }

            bce.message2 = new mod.embeddedStructType();
            bce.message2.tstStr = null;
            rc = bce.datawriter2.Write(bce.message2, 0);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "null pointer struct did NOT return BAD_PARAMETER.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
