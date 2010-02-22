namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Condition3 : Test.Framework.TestCase
    {
        public Condition3()
            : base("sacs_condition_tc3", "sacs_condition", "sacs_condition",
                "test querycondition", "test querycondition", null)
        {
            this.AddPreItem(new test.sacs.ConditionInit());
            this.AddPostItem(new test.sacs.ConditionDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            mod.tstDataReader reader2;
            DDS.IQueryCondition condition;
            DDS.WaitSet waitset;
            DDS.ISubscriber subscriber;
            DDS.ICondition[] activeConditions = new DDS.Condition[0];
            Test.Framework.TestResult result;
            DDS.DataReaderQos drQos = null;
            mod.tst[] tstHolder;
            DDS.SampleInfo[] sampleInfoHolder;
            string[] queryParams;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            string expression;
            string[] ssHolder;
            string expResult = "QueryCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            drQos = (DDS.DataReaderQos)this.ResolveObject("datareaderQos");
            expression = "long_1=%0";
            queryParams = new string[1];
            queryParams[0] = "1";
            condition = reader.CreateQueryCondition(DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any, expression, queryParams);
            if (condition == null)
            {
                result.Result = "Could not create query condition.";
                return result;
            }
            waitset = new DDS.WaitSet();
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not attach condition.";
                return result;
            }
            reader2 = (mod.tstDataReader)subscriber.CreateDataReader(topic, drQos); //, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create datareader.";
                return result;
            }

            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(3).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (3).";
                return result;
            }
            mod.tst data = new mod.tst();
            data.long_1 = 2;
            data.long_2 = 2;
            data.long_3 = 3;

            rc = writer.Write(data, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed.";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(3).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (4).";
                return result;
            }
            data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;

            rc = writer.Write(data, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed.";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed. ";
                return result;
            }
            if (activeConditions.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one.";
                return result;
            }
            if (activeConditions[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition.";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed(2).";
                return result;
            }
            if (activeConditions.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one(2).";
                return result;
            }
            if (activeConditions[0] != condition)
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
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (activeConditions.Length != 0)
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
            rc = waitset.GetConditions(ref activeConditions);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions failed.";
                return result;
            }
            if (activeConditions.Length != 1)
            {
                result.Result = "Returned conditions not valid.";
                return result;
            }
            if (activeConditions[0] != condition)
            {
                result.Result = "Returned condition does not equal set condition.";
                return result;
            }
            DDS.IDataReader reader3 = condition.GetDataReader();
            if (reader != reader3)
            {
                result.Result = "ReadCondition.get_datareader failed.";
                return result;
            }

            if (condition.GetInstanceStateMask() != DDS.InstanceStateKind.Any)
            {
                result.Result = "ReadCondition.get_instance_state_mask failed.";
                return result;
            }

            if (condition.GetViewStateMask() != DDS.ViewStateKind.Any)
            {
                result.Result = "ReadCondition.get_view_state_mask failed.";
                return result;
            }

            if (condition.GetSampleStateMask() != DDS.SampleStateKind.Any)
            {
                result.Result = "ReadCondition.get_sample_state_mask failed.";
                return result;
            }
            rc = reader.DeleteReadCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_readcondition failed.";
                return result;
            }
            rc = waitset.GetConditions(ref activeConditions);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions failed(2).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "Returned conditions not valid(2).";
                return result;
            }
            queryParams = new string[1];
            queryParams[0] = "1";
            condition = reader.CreateQueryCondition(DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any, "long_1=%0", queryParams);
            if (condition == null)
            {
                result.Result = "Could not create query condition.";
                return result;
            }
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not attach condition.";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(q1).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (q1).";
                return result;
            }

            data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;

            rc = writer.Write(data, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed(2).";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed (q2). ";
                return result;
            }
            if (activeConditions.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one(q2).";
                return result;
            }
            if (activeConditions[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition(q2).";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "WaitSet.Wait failed(2).";
                return result;
            }
            if (activeConditions.Length != 1)
            {
                result.Result = "WaitSet.Wait returned no or multiple conditions where it should return one(q3).";
                return result;
            }
            if (activeConditions[0] != condition)
            {
                result.Result = "WaitSet.Wait returned wrong condition(q3).";
                return result;
            }
            tstHolder = new mod.tst[0];

            rc = reader.TakeWithCondition(ref tstHolder, ref sampleInfoHolder, 1, condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed(q1).";
                return result;
            }
            rc = reader.ReturnLoan(ref tstHolder, ref sampleInfoHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.return_loan failed.";
                return result;
            }
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed(q5).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't (q6).";
                return result;
            }
            string expression2 = condition.GetQueryExpression();
            if (!expression.Equals(expression2))
            {
                result.Result = "QueryCondition.get_query_expression does not work properly.";
                return result;
            }
            ssHolder = new string[0];
            rc = condition.GetQueryParameters(ref ssHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "QueryCondition.get_query_parameters call failed.";
                return result;
            }
            string[] queryParams2 = ssHolder;
            if (queryParams2 == null)
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (1).";
                return result;
            }
            if (queryParams2.Length != queryParams.Length)
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (2).";
                return result;
            }
            if (!queryParams2[0].Equals(queryParams[0]))
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (3).";
                return result;
            }
            queryParams[0] = "5";
            rc = condition.SetQueryParameters(queryParams);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "QueryCondition.set_query_parameters does not work properly.";
                return result;
            }
            rc = condition.GetQueryParameters(ref ssHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "QueryCondition.get_query_parameters call failed (1).";
                return result;
            }
            queryParams2 = ssHolder;
            if (queryParams2 == null)
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (1).";
                return result;
            }
            if (queryParams2.Length != queryParams.Length)
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (2).";
                return result;
            }
            if (!queryParams2[0].Equals(queryParams[0]))
            {
                result.Result = "QueryCondition.get_query_parameters does not work properly (3).";
                return result;
            }
            rc = condition.SetQueryParameters(null);
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "QueryCondition.set_query_parameters does not work properly (2).";
                return result;
            }
            rc = reader.DeleteReadCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_readcondition failed(q7).";
                return result;
            }
            rc = waitset.GetConditions(ref activeConditions);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions failed(q7).";
                return result;
            }
            if (activeConditions.Length != 0)
            {
                result.Result = "Returned conditions not valid(q7).";
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
