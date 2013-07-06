namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Reader1 : Test.Framework.TestCase
    {
        public Reader1()
            : base("sacs_reader_tc1", "sacs_reader", "sacs_reader", "test datareader qos"
                , "test datareader qos", null)
        {
            this.AddPreItem(new test.sacs.ReaderInit());
            this.AddPostItem(new test.sacs.ReaderDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDataReader reader;
			DDS.DataReaderQos qos = null;
			DDS.DataReaderQos qos2 = null;
			DDS.DataReaderQos holder = null;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            string expResult = "Reader test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            reader = (DDS.IDataReader)this.ResolveObject("datareader");
            qos = (DDS.DataReaderQos)this.ResolveObject("datareaderQos");

            if (reader.GetQos(ref holder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Qos of DataReader could not be resolved.";
                return result;
            }
            if (!test.sacs.QosComparer.DataReaderQosEquals(holder, qos))
            {
                result.Result = "Qos of DataReader does not match provided qos.";
                return result;
            }
            if (!test.sacs.QosComparer.DataReaderQosEquals(holder, test.sacs.QosComparer.defaultDataReaderQos))
            {
                result.Result = "Qos of DataWriter does not match default qos.";
                return result;
            }
            qos2 = holder;
            qos2.Deadline.Period = new DDS.Duration(3, 3);
            qos2.LatencyBudget.Duration = new DDS.Duration(6, 6);
            qos2.ReaderDataLifecycle.AutopurgeDisposedSamplesDelay = new DDS.Duration(
                5, 5);
            qos2.ReaderDataLifecycle.EnableInvalidSamples = false;
            qos2.ReaderDataLifecycle.InvalidSampleVisibility.Kind = DDS.InvalidSampleVisibilityQosPolicyKind.MinimumInvalidSamples;
            qos2.UserData.Value = new byte[2];
            qos2.UserData.Value[0] = 2;
            qos2.UserData.Value[0] = 4;
            rc = reader.SetQos(qos2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "New Qos could not be applied.";
                return result;
            }

            if (reader.GetQos(ref holder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Qos of DataReader could not be resolved (2).";
                return result;
            }
            if (!test.sacs.QosComparer.DataReaderQosEquals(holder, qos2))
            {
                result.Result = "Qos of DataReader does not match provided qos (2).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
