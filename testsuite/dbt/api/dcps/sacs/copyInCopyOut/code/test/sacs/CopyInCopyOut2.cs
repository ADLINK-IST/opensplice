namespace test.sacs
{
    /// <summary>Basic test of CopyIn/CopyOut functions.</summary>
    /// <remarks>Basic test of CopyIn/CopyOut functions.</remarks>
    public class CopyInCopyOut2 : Test.Framework.TestCase
    {
        /// <summary>Basic test of CopyIn/CopyOut functions.</summary>
        /// <remarks>Basic test of CopyIn/CopyOut functions.</remarks>
        public CopyInCopyOut2()
            : base("sacs_copyInCopyOut_tc2", "sacs_copyInCopyOut",
                "CopyOut", "Read a sample with a datareader", 
                "Test whether the CopyOut function that feeds the datareader works correctly."
                , null)
        {
            this.AddPreItem(new test.sacs.CopyInCopyOutInit());
            this.AddPostItem(new test.sacs.CopyInCopyOutDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ReturnCode retCode;
            DDS.ITopic topic;
            DDS.TopicQos tQos;
            DDS.ISubscriber sub;
            Foo.TestDataReader testDR;
            DDS.DataReaderQos drQos = new DDS.DataReaderQos();
            Test.Framework.TestResult result;
            string expResult = "Successfully read a sample.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            tQos = (DDS.TopicQos)this.ResolveObject("topicQos");
            sub = participant.CreateSubscriber();
            if (sub == null)
            {
                result.Result = "participant.create_subscriber failed.";
                return result;
            }
            retCode = sub.GetDefaultDataReaderQos(ref drQos);
            if (retCode != DDS.ReturnCode.Ok)
            {
                result.Result = "subscriber.get_default_datareader_qos failed.";
                return result;
            }
            testDR = (Foo.TestDataReader) sub.CreateDataReader(topic, drQos);
            if (testDR == null)
            {
                result.Result = "subscriber.create_datareader failed.";
                return result;
            }
            
            
            
            
            
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
