namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Status2 : Test.Framework.TestCase
    {
        public Status2()
            : base("sacs_status_tc2", "sacs_status", "status", "Test if DataReader statusses work correctly."
                , "Test if DataReader statusses work correctly.", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ReturnCode rc;
            mod.tstDataReader datareader;
            DDS.SampleRejectedStatus srStatus = new DDS.SampleRejectedStatus();
            DDS.LivelinessChangedStatus lcStatus = new DDS.LivelinessChangedStatus();
            DDS.RequestedDeadlineMissedStatus rdmStatus = new DDS.RequestedDeadlineMissedStatus();
            DDS.RequestedIncompatibleQosStatus riqStatus = new DDS.RequestedIncompatibleQosStatus();
            DDS.SubscriptionMatchedStatus smStatus = new DDS.SubscriptionMatchedStatus();
            DDS.SampleLostStatus slStatus = new DDS.SampleLostStatus();
            string expResult = "DataReader status test succeeded";
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            datareader = (mod.tstDataReader)this.ResolveObject("datareader");
            rc = datareader.GetSampleRejectedStatus(srStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "SampleRejectedStatus could not be resolved.";
                return result;
            }
            if (srStatus.TotalCount != 0)
            {
                result.Result = "SampleRejectedStatus.TotalCount != 0.";
                return result;
            }
            if (srStatus.TotalCountChange != 0)
            {
                result.Result = "SampleRejectedStatus.TotalCountChange != 0.";
                return result;
            }
            rc = datareader.GetLivelinessChangedStatus(lcStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "LivelinessChangedStatus could not be resolved.";
                return result;
            }
            rc = datareader.GetRequestedIncompatibleQosStatus(riqStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "RequestedIncompatibleStatus could not be resolved.";
                return result;
            }
            if (riqStatus.TotalCount != 0)
            {
                result.Result = "RequestedIncompatibleQosStatus.TotalCount != 0.";
                return result;
            }
            if (riqStatus.TotalCountChange != 0)
            {
                result.Result = "RequestedIncompatibleQosStatus.TotalCountChange != 0.";
                return result;
            }
            rc = datareader.GetSubscriptionMatchedStatus(smStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "SubscriptionMatchStatus could not be resolved.";
                return result;
            }
            rc = datareader.GetSampleLostStatus(slStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "SampleLostStatus could not be resolved.";
                return result;
            }
            if (slStatus.TotalCount != 0)
            {
                result.Result = "SampleLostStatus.TotalCount != 0.";
                return result;
            }
            if (slStatus.TotalCountChange != 0)
            {
                result.Result = "SampleLostStatus.TotalCountChange != 0.";
                return result;
            }
            rc = datareader.GetRequestedDeadlineMissedStatus(rdmStatus);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "RequestedDeadlineMissedStatus could not be resolved (" + rc + ").";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
