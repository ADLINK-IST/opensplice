namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Waitset7 : Test.Framework.TestCase
    {
        public Waitset7()
            : base("sacs_waitset_tc7", "waitset", "waitset", "test readcondition in waitset"
                , "test readcondition in waitset", null)
        {
            this.AddPreItem(new test.sacs.WaitsetInit());
            this.AddPostItem(new test.sacs.WaitsetDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            mod.tstDataReader reader2;
            DDS.IReadCondition condition;
            DDS.WaitSet waitset;
            DDS.ISubscriber subscriber;
            DDS.ICondition[] conditionHolder;
            Test.Framework.TestResult result;
            DDS.DataReaderQos drQos;
            mod.tst[] tstHolder;
            DDS.SampleInfo[] sampleInfoHolder;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            string expResult = "ReadCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            drQos = (DDS.DataReaderQos)this.ResolveObject("datareaderQos");
            condition = reader.CreateReadCondition(DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
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
            reader2 = (mod.tstDataReader)subscriber.CreateDataReader(topic, drQos);//, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create datareader.";
                return result;
            }
            conditionHolder = new DDS.ICondition[0];
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(3).";
                return result;
            }
            if (conditionHolder.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (3).";
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
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed(2).";
                return result;
            }
            if (conditionHolder.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one(2).";
                return result;
            }
            if (conditionHolder[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition(2).";
                return result;
            }
            tstHolder = new mod.tst[0];
            sampleInfoHolder = new DDS.SampleInfo[0];
            rc = reader.TakeWithCondition(ref tstHolder, ref sampleInfoHolder, 1, condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed.";
                return result;
            }
            rc = reader.ReturnLoan(ref tstHolder, ref sampleInfoHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.return_loan failed.";
                return result;
            }
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (conditionHolder.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (2).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "delete_datareader succeeded, but should not.";
                return result;
            }
            rc = waitset.GetConditions(ref conditionHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions failed.";
                return result;
            }
            if (conditionHolder.Length != 1)
            {
                result.Result = "Returned conditions not valid.";
                return result;
            }
            if (conditionHolder[0] != condition)
            {
                result.Result = "Returned condition does not equal set condition.";
                return result;
            }
            rc = reader.DeleteReadCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_readcondition failed.";
                return result;
            }
            rc = waitset.GetConditions(ref conditionHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions failed(2).";
                return result;
            }
            if (conditionHolder.Length != 0)
            {
                result.Result = "Returned conditions not valid(2).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_datareader failed.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
