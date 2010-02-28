namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Waitset5 : Test.Framework.TestCase
    {
        public Waitset5()
            : base("sacs_waitset_tc5", "waitset", "waitset", "test datareader status condition in waitset"
                , "test datareader status condition in waitset", null)
        {
            this.AddPreItem(new test.sacs.WaitsetInit());
            this.AddPostItem(new test.sacs.WaitsetDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            DDS.IStatusCondition condition;
            DDS.IPublisher publisher;
            DDS.WaitSet waitset;
            mod.tst[] tstHolder;
            DDS.ICondition[] conditionHolder;
            DDS.SampleInfo[] sampleInfoHolder;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "StatusCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            condition = reader.StatusCondition;
            if (condition == null)
            {
                result.Result = "Could not resolve reader condition.";
                return result;
            }
            waitset = new DDS.WaitSet();
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not attach condition.";
                return result;
            }
            conditionHolder = new DDS.ICondition[0];
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed. (1)";
                return result;
            }
            if (conditionHolder.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one.";
                return result;
            }
            if (conditionHolder[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition.";
                return result;
            }
            if (!test.sacs.StatusValidator.LivelinessChangedValid(reader, 1, 1, 0, 0))
            {
                result.Result = "liveliness_changed not valid.";
                return result;
            }
            if (!test.sacs.StatusValidator.LivelinessChangedValid(reader, 1, 0, 0, 0))
            {
                result.Result = "liveliness_changed not valid (2).";
                return result;
            }
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed. (2)";
                return result;
            }
            if (conditionHolder.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (2).";
                return result;
            }
            mod.tst data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;
            rc = writer.Write(data, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed.";
                return result;
            }

            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed. (3)";
                return result;
            }
            if (conditionHolder.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no conditions where it should (1).";
                return result;
            }
            if (conditionHolder[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition (1).";
                return result;
            }
            tstHolder = new mod.tst[0];
            sampleInfoHolder = new DDS.SampleInfo[0];
            rc = reader.Take(ref tstHolder, ref sampleInfoHolder, 1, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
            reader.ReturnLoan(ref tstHolder, ref sampleInfoHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed.";
                return result;
            }

            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(4).";
                return result;
            }
            if (conditionHolder.Length > 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't.(2)";
                return result;
            }
            rc = publisher.DeleteDataWriter(writer);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_datawriter failed.";
                return result;
            }
            try
            {
                System.Threading.Thread.Sleep(2000);
            }
            catch (System.Exception)
            {
                System.Console.Error.WriteLine("Sleep failed...");
            }

            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed. (5)";
                return result;
            }
            if (conditionHolder.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one.";
                return result;
            }
            if (conditionHolder[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition.";
                return result;
            }
            if (!test.sacs.StatusValidator.LivelinessChangedValid(reader, 0, 1, 0, 0))
            {
                result.Result = "liveliness_changed not valid (3).";
                return result;
            }
            if (!test.sacs.StatusValidator.LivelinessChangedValid(reader, 0, 0, 0, 0))
            {
                result.Result = "liveliness_changed not valid (4).";
                return result;
            }
            rc = reader.Take(ref tstHolder, ref sampleInfoHolder, 1, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
            reader.ReturnLoan(ref tstHolder, ref sampleInfoHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed.";
                return result;
            }

            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(6).";
                return result;
            }
            if (conditionHolder.Length > 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't.(3)";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }

        private void PrintStatusses(DDS.IDataReader reader)
        {
            DDS.ReturnCode rc;
            DDS.RequestedDeadlineMissedStatus rdmsHolder = new DDS.RequestedDeadlineMissedStatus();
            rc = reader.GetRequestedDeadlineMissedStatus(ref rdmsHolder);
            DDS.RequestedDeadlineMissedStatus rdms = rdmsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            DDS.RequestedIncompatibleQosStatus riqsHolder = new DDS.RequestedIncompatibleQosStatus();
            rc = reader.GetRequestedIncompatibleQosStatus(ref riqsHolder);
            DDS.RequestedIncompatibleQosStatus riqs = riqsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            DDS.SampleRejectedStatus srsHolder = new DDS.SampleRejectedStatus();
            rc = reader.GetSampleRejectedStatus(ref srsHolder);
            DDS.SampleRejectedStatus srs = srsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            DDS.LivelinessChangedStatus lcsHolder = new DDS.LivelinessChangedStatus();
            rc = reader.GetLivelinessChangedStatus(ref lcsHolder);
            DDS.LivelinessChangedStatus lcs = lcsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            DDS.SubscriptionMatchedStatus smsHolder = new DDS.SubscriptionMatchedStatus();
            rc = reader.GetSubscriptionMatchedStatus(ref smsHolder);
            DDS.SubscriptionMatchedStatus sms = smsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            DDS.SampleLostStatus slsHolder = new DDS.SampleLostStatus();
            rc = reader.GetSampleLostStatus(ref slsHolder);
            DDS.SampleLostStatus sls = slsHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                System.Console.Error.WriteLine("Unable to resolve status!");
                return;
            }
            System.Console.Out.WriteLine("requested_deadline_missed.TotalCount         : " +
                 rdms.TotalCount);
            System.Console.Out.WriteLine("requested_deadline_missed.TotalCountChange  : " +
                 rdms.TotalCountChange);
            System.Console.Out.WriteLine("requested_deadline_missed.LastInstanceHandle: " +
                 rdms.LastInstanceHandle);
            System.Console.Out.WriteLine("requested_incompatible_qos.TotalCount        : " +
                 riqs.TotalCount);
            System.Console.Out.WriteLine("requested_incompatible_qos.TotalCountChange : " +
                 riqs.TotalCountChange);
            System.Console.Out.WriteLine("requested_incompatible_qos.LastPolicyId     : " +
                 riqs.LastPolicyId);
            System.Console.Out.WriteLine("sample_rejected.TotalCount                   : " +
                 srs.TotalCount);
            System.Console.Out.WriteLine("sample_rejected.TotalCountChange            : " +
                 srs.TotalCountChange);
            System.Console.Out.WriteLine("sample_rejected.LastInstanceHandle          : " +
                 srs.LastInstanceHandle);
            System.Console.Out.WriteLine("liveliness_changed.AliveCount                : " +
                 lcs.AliveCount);
            System.Console.Out.WriteLine("liveliness_changed.AliveCountChange         : " +
                 lcs.AliveCountChange);
            System.Console.Out.WriteLine("liveliness_changed.not_alive_count            : " +
                 lcs.NotAliveCount);
            System.Console.Out.WriteLine("liveliness_changed.not_alive_count_change     : " +
                 lcs.NotAliveCountChange);
            System.Console.Out.WriteLine("subscription_match.TotalCount                : " +
                 sms.TotalCount);
            System.Console.Out.WriteLine("subscription_match.TotalCountChange         : " +
                 sms.TotalCountChange);
            System.Console.Out.WriteLine("subscription_match.last_publication_handle    : " +
                 sms.LastPublicationHandle);
            System.Console.Out.WriteLine("sample_lost.TotalCount                       : " +
                 sls.TotalCount);
            System.Console.Out.WriteLine("sample_lost.TotalCountChange                : " +
                 sls.TotalCountChange + "\n");
        }
    }
}
