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
            DDS.LivelinessLostStatus llStatus;
            DDS.OfferedDeadlineMissedStatus odmStatus;
            DDS.OfferedIncompatibleQosStatus oiqStatus;
            DDS.PublicationMatchedStatus pmStatus;
            string expResult = "DataWriter status test succeeded";
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            datawriter = (mod.tstDataWriter)this.ResolveObject("datawriter");
            rc = datawriter.GetLivelinessLostStatus(out llStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = "LivelinessLostStatus could not be resolved.";
                return result;
            }
            rc = datawriter.GetOfferedDeadlineMissedStatus(out odmStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = "OfferedDeadlineMissedStatus could not be resolved.";
                return result;
            }
            rc = datawriter.GetOfferedIncompatibleQosStatus(out oiqStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = "OfferedIncompatibleStatus could not be resolved.";
                return result;
            }
            rc = datawriter.GetPublicationMatchedStatus(out pmStatus);
            if (DDS.ReturnCode.Ok != rc)
            {
                result.Result = "PublicationMatchStatus could not be resolved.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
