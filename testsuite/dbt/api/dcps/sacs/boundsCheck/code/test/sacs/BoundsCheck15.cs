namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck15 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck15(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc15", "sacs_boundsCheck", "DataWriter.write",
                 "Check for correct structs array bounds",
                 "Check that struct arrays equal to their specified size are accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "struct array with correct size returns Ok";
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
            bce.message2.tstArr = new mod.tst[3];
            for (int i = 0; i < 3; i++) {
                bce.message2.tstArr[i] = new mod.tst(); // Prevent uninitialized array from causing error.
            }
            rc = bce.datawriter2.Write(bce.message2, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "struct array with correct size did NOT return Ok.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
