namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class BoundsCheck17 : Test.Framework.TestCase
    {
        private BoundsCheckEntities bce;

        public BoundsCheck17(BoundsCheckEntities bce) :
            base("sacs_boundsCheck_tc17", "sacs_boundsCheck", "DataWriter.write",
                 "Check for correct sequence of structs bounds",
                 "Check that sequences of structs bigger than their specified bounds are NOT accepted", null)
        {
            this.bce = bce;
        }


        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            string expResult = "sequence of structs with length > bound returns BadParameter";
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
            for (int i = 0; i < 3; i++) {
                bce.message2.tstArr[i] = new mod.tst(); // Prevent uninitialized array from causing error.
            }
            bce.message2.tstSeq = new mod.tst[4];
            for (int i = 0; i < 4; i++) {
                bce.message2.tstSeq[i] = new mod.tst(); // Prevent uninitialized sequence elements from causing error.
            }
            rc = bce.datawriter2.Write(bce.message2, 0);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "sequence of structs with length > bound did NOT return BadParameter.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
