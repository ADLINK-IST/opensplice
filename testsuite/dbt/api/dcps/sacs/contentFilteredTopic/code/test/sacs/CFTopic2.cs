namespace test.sacs
{
    /// <summary>Tests for the creation of a ContentFilteredTopic.</summary>
    /// <remarks>Tests for the creation of a ContentFilteredTopic.</remarks>
    public class CFTopic2 : Test.Framework.TestCase
    {
        /// <summary>Tests for the creation of a ContentFilteredTopic.</summary>
        /// <remarks>Tests for the creation of a ContentFilteredTopic.</remarks>
        public CFTopic2()
            : base("sacs_content_filtered_topic_tc2", "sacs_content_filtered_topic"
                , "sacs_content_filtered_topic", "Test the use of a ContentFilteredTopic for a DataReader"
                , "Test the use of a ContentFilteredTopic for a DataReader", null)
        {
            this.AddPreItem(new test.sacs.CFTopicItem1Init());
            this.AddPreItem(new test.sacs.CFTopicItem2Init());
            this.AddPostItem(new test.sacs.CFTopicItem2Deinit());
            this.AddPostItem(new test.sacs.CFTopicItem1Deinit());
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "ContentFilteredTopic test succeeded";
            mod.tstDataReader reader;
            mod.tstDataWriter writer;
            mod.tst[] seqHolder;
            DDS.SampleInfo[] infoList;
            Test.Framework.TestResult result;
            mod.tst testObject;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            reader = (mod.tstDataReader)this.ResolveObject("reader");
            writer = (mod.tstDataWriter)this.ResolveObject("writer");
            testObject = new mod.tst();
            testObject.long_1 = 0;
            testObject.long_2 = 2;
            testObject.long_3 = 3;
            rc = writer.Write(testObject, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Recieved return code " + rc + " after calling writer.write (1).";
                return result;
            }
            seqHolder = new mod.tst[0];
            infoList = new DDS.SampleInfo[0];
            rc = reader.Take(ref seqHolder, ref infoList, DDS.Length.Unlimited, DDS.SampleStateKind.Any,
                DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Recieved return code " + rc + " after calling reader.read (2).";
                return result;
            }
            if (seqHolder.Length != 1)
            {
                result.Result = "Recieved unexpected number of samples (2)";
                return result;
            }
            if (seqHolder[0].long_1 != 0)
            {
                result.Result = "Recieved incorrect data (2)";
                return result;
            }
            rc = reader.ReturnLoan(ref seqHolder, ref infoList);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "reader.return_loan failed";
                return result;
            }
            testObject.long_1 = 5;
            testObject.long_2 = 2;
            testObject.long_3 = 3;
            rc = writer.Write(testObject, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Recieved return code " + rc + " after calling writer.write (3).";
                return result;
            }
            seqHolder = new mod.tst[0];
            infoList = new DDS.SampleInfo[0];
            rc = reader.Take(ref seqHolder, ref infoList, DDS.Length.Unlimited, DDS.SampleStateKind.Any,
                DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
            if (rc != DDS.ReturnCode.NoData)
            {
                if (rc == DDS.ReturnCode.Ok)
                {
                    reader.ReturnLoan(ref seqHolder, ref infoList);
                }
                result.Result = "Recieved return code " + rc + " after calling reader.read for filtered data (4).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
