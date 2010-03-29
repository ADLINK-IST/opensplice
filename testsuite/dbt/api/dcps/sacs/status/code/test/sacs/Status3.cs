namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Status3 : Test.Framework.TestCase
    {
        public Status3()
            : base("sacs_status_tc3", "sacs_status", "status", "Test if DataWriter statusses work correctly."
                , "Test if DataWriter statusses work correctly.", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter datawriter;
            DDS.ReturnCode rc;
			DDS.LivelinessLostStatus llStatus = null;
			DDS.OfferedDeadlineMissedStatus odmStatus = null;
			DDS.OfferedIncompatibleQosStatus oiqStatus = null;
			DDS.PublicationMatchedStatus pmStatus = null;
            string expResult = "DataWriter status test succeeded";
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            datawriter = (mod.tstDataWriter)this.ResolveObject("datawriter");
            rc = datawriter.GetLivelinessLostStatus(ref llStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = string.Format("LivelinessLostStatus could not be resolved. ReturnCode: {0}", rc);
                return result;
            }
            rc = datawriter.GetOfferedDeadlineMissedStatus(ref odmStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = string.Format("OfferedDeadlineMissedStatus could not be resolved. ReturnCode: {0}", rc);
                return result;
            }
            rc = datawriter.GetOfferedIncompatibleQosStatus(ref oiqStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = string.Format("OfferedIncompatibleStatus could not be resolved. ReturnCode: {0}", rc);
                return result;
            }
            rc = datawriter.GetPublicationMatchedStatus(ref pmStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = string.Format("OfferedIncompatibleStatus could not be resolved. ReturnCode: {0}", rc);
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
