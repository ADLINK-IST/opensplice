namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck12 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck12(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc12", "sacs_boundsCheck", "DataWriter.write",
                 "Check for correct structs arrays",
                 "Check that null pointer struct arrays are not accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "null pointer struct array returns BAD_PARAMETER";
            Test.Framework.TestResult result = new Test.Framework.TestResult(
                expResult,
                string.Empty,
                Test.Framework.TestVerdict.Fail,
                Test.Framework.TestVerdict.Fail);

            if (result.Result != string.Empty)
            {
                return result;
            }

            bce.message2 = new mod.embeddedStructType();
            bce.message2.tstArr = null;
            // scdds2162 - Once solved for CopyIn, re-enable the Write instruction.
            //rc = bce.datawriter2.Write(bce.message2, 0);
            rc = DDS.ReturnCode.Ok; // Temporary to make testcase FAIL. Please Remove when Write is re-enabled. 
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "null pointer struct array did NOT return BAD_PARAMETER.";
            	this.testFramework.TestMessage(Test.Framework.TestMessage.Note, "See scdds2162: CopyIn should do more validity checking.");
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
