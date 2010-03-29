namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Waitset3 : Test.Framework.TestCase
    {
        public Waitset3()
            : base("sacs_waitset_tc3", "waitset", "waitset", "test subscriber status condition in waitset"
                , "test subscriber status condition in waitset", null)
        {
            this.AddPreItem(new test.sacs.WaitsetInit());
            this.AddPostItem(new test.sacs.WaitsetDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            DDS.IStatusCondition subscriberCondition;
            DDS.WaitSet waitset;
            mod.tst[] tstHolder;
            DDS.ICondition[] conditionHolder;
            DDS.SampleInfo[] sampleInfoHolder;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "StatusCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            subscriberCondition = subscriber.StatusCondition;
            if (subscriberCondition == null)
            {
                result.Result = "Could not resolve subscriber condition.";
                return result;
            }
            waitset = new DDS.WaitSet();
            rc = waitset.AttachCondition(subscriberCondition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not attach subscriber condition.";
                return result;
            }
            conditionHolder = new DDS.ICondition[0];
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (conditionHolder.Length > 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't.";
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
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (conditionHolder.Length < 1)
            {
                result.Result = "WaitSet.Wait returned no conditions where it should (1).";
                return result;
            }
            if (conditionHolder[0] != subscriberCondition)
            {
                result.Result = "WaitSet.Wait returned wrong condition.";
                return result;
            }
            tstHolder = new mod.tst[0];
            sampleInfoHolder = new DDS.SampleInfo[0];
            rc = reader.Take(ref tstHolder, ref sampleInfoHolder, 1, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed.";
                return result;
            }
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(2).";
                return result;
            }
            if (conditionHolder.Length > 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (2).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
