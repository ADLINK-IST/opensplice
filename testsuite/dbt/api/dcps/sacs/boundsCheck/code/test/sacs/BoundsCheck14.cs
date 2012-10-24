namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck14 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck14(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc14", "sacs_boundsCheck", "DataWriter.write",
                 "Check for correct structs array bounds",
                 "Check that struct arrays bigger than their specified size are not accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "struct array with too big size returns BAD_PARAMETER";
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
            bce.message2.tstArr = new mod.tst[4];
            for (int i = 0; i < 4; i++) {
                bce.message2.tstArr[i] = new mod.tst(); // Prevent uninitialized array from causing error.
            }
            // scdds2162 - Once solved for CopyIn, re-enable the Write instruction.
            //rc = bce.datawriter2.Write(bce.message2, 0);
            rc = DDS.ReturnCode.Ok; // Temporary to make testcase FAIL. Please Remove when Write is re-enabled. 
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "struct array with too big size did NOT return BAD_PARAMETER.";
            	this.testFramework.TestMessage(Test.Framework.TestMessage.Note, "See scdds2162: CopyIn should do more validity checking.");
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
