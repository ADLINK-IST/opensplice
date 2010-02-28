namespace test.sacs
{
    public class HistDataWithCondition6 : Test.Framework.TestCase
    {
        public HistDataWithCondition6()
            : base("sacs_histDataWithCondition_tc6", "histDataWithCondition"
                , "histDataWithCondition", "test time limits for historical data", "test hist_data_w_condition"
                , null)
        {
            AddPreItem(new test.sacs.HistDataWConditionInit());
            AddPostItem(new test.sacs.HistDataWConditionDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "Received 10 samples.";
            DDS.ReturnCode ddsReturnCode;
            mod.tst data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;

            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            DDS.DataReaderQos drQos;
            DDS.ISubscriber subscriber;
            DDS.ITopic topic;
            DDS.ResourceLimitsQosPolicy resource;
            DDS.Duration maxWait;
            DDS.Time maxSourceTime;
            DDS.Time minSourceTime;
            DDS.IDomainParticipant participant;
            DDS.Time minHolder = new DDS.Time();
            DDS.Time maxHolder = new DDS.Time();
            mod.tst[] dataList;
            DDS.SampleInfo[] infoList;
            ddsReturnCode = DDS.ReturnCode.Ok;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            drQos = (DDS.DataReaderQos)this.ResolveObject("datareaderQos");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            ddsReturnCode = DDS.ReturnCode.Ok;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            for (int i = 0; (i < 10) && (ddsReturnCode == DDS.ReturnCode.Ok); i++)
            {
                data.long_1 = i;
                data.long_2 = data.long_1 + 1;
                data.long_3 = data.long_1 + 2;
                System.Console.Out.WriteLine("Write: " + data.long_1 + ", " + data.long_2 + ", "
                    + data.long_3);
                ddsReturnCode = writer.Write(data, DDS.InstanceHandle.Nil);
            }
            participant.GetCurrentTime(out minHolder);
            minSourceTime = minHolder;
            for (int i = 10; (i < 20) && (ddsReturnCode == DDS.ReturnCode.Ok); i++)
            {
                data.long_1 = i;
                data.long_2 = data.long_1 + 1;
                data.long_3 = data.long_1 + 2;
                System.Console.Out.WriteLine("Write: " + data.long_1 + ", " + data.long_2 + ", "
                    + data.long_3);
                ddsReturnCode = writer.Write(data, DDS.InstanceHandle.Nil);
            }
            if (ddsReturnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Writing data failed.";
                return result;
            }
            drQos = (DDS.DataReaderQos)this.ResolveObject("datareaderQos");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            reader = (mod.tstDataReader)subscriber.CreateDataReader(topic, drQos);//, null, 0);
            if (reader == null)
            {
                result.Result = "creating datareader failed.";
                return result;
            }
            resource = new DDS.ResourceLimitsQosPolicy();
            resource.MaxInstances = -1;
            resource.MaxSamples = -1;
            resource.MaxSamplesPerInstance = -1;
            maxWait = new DDS.Duration(10, 0);
            participant.GetCurrentTime(out maxHolder);
            maxSourceTime = maxHolder;

            // TODO: JLS, WaitForHistoricalDataWithCondition is missing
            //            ddsReturnCode = reader.WaitForHistoricalDataWithCondition(null, null, minSourceTime
            //				, maxSourceTime, resource, maxWait);
            if (ddsReturnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "wait_for_historical_data_w_condition failed";
                subscriber.DeleteDataReader(reader);
                return result;
            }
            dataList = null;
            infoList = null;
            ddsReturnCode = reader.Take(ref dataList, ref infoList, 10, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
            if (ddsReturnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "datareader.take failed " + ddsReturnCode + " (" + dataList
                    .Length + " samples taken)";
                this.testFramework.TestMessage(Test.Framework.TestMessage.Note, "waitForHistoricalDataWithCondition not implemented yet.");
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail; subscriber.DeleteDataReader(reader);
                subscriber.DeleteDataReader(reader);
                return result;
            }
            for (int i = 0; i < dataList.Length; i++)
            {
                System.Console.Out.WriteLine("Read sample [" + dataList[i].long_1 + ", " +
                    dataList[i].long_2 + ", " + dataList[i].long_3 + "]");
            }
            if (dataList.Length != 10)
            {
                result.Result = "Unexpected number of samples received.";
                subscriber.DeleteDataReader(reader);
                return result;
            }
            reader.ReturnLoan(ref dataList, ref infoList);
            subscriber.DeleteDataReader(reader);
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
