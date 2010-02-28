namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Writer1 : Test.Framework.TestCase
    {
        public Writer1()
            : base("sacs_writer_tc1", "sacs_writer", "sacs_writer", "test datawriter qos"
                , "test datawriter qos", null)
        {
            this.AddPreItem(new test.sacs.WriterInit());
            this.AddPostItem(new test.sacs.WriterDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDataWriter writer;
            DDS.DataWriterQos qos;
            DDS.DataWriterQos qos2;
			DDS.DataWriterQos holder = null;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            string expResult = "Writer test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (DDS.IDataWriter)this.ResolveObject("datawriter");
            qos = (DDS.DataWriterQos)this.ResolveObject("datawriterQos");

            if (writer.GetQos(ref holder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Qos of DataWriter could not be resolved.";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(holder, qos))
            {
                result.Result = "Qos of DataWriter does not match provided qos.";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(holder, test.sacs.QosComparer.defaultDataWriterQos))
            {
                result.Result = "Qos of DataWriter does not match default qos.";
                return result;
            }
            qos2 = holder;
            qos2.Deadline.Period = new DDS.Duration(3, 3);
            qos2.LatencyBudget.Duration = new DDS.Duration(6, 6);
            qos2.Lifespan.Duration = new DDS.Duration(9, 9);
            qos2.OwnershipStrength.Value = 22;
            qos2.UserData.Value = new byte[2];
            qos2.UserData.Value[0] = 2;
            qos2.UserData.Value[0] = 4;
            rc = writer.SetQos(qos2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "New Qos could not be applied.";
                return result;
            }

            if (writer.GetQos(ref holder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Qos of DataWriter could not be resolved (2).";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(holder, qos2))
            {
                result.Result = "Qos of DataWriter does not match provided qos (2).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
