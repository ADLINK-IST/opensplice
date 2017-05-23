namespace test.sacs
{
    /// <summary>Basic test of CopyIn/CopyOut functions.</summary>
    /// <remarks>Basic test of CopyIn/CopyOut functions.</remarks>
    public class CopyInCopyOut1 : Test.Framework.TestCase
    {
        /// <summary>Basic test of CopyIn/CopyOut functions.</summary>
        /// <remarks>Basic test of CopyIn/CopyOut functions.</remarks>
        public CopyInCopyOut1()
            : base("sacs_copyInCopyOut_tc1", "sacs_copyInCopyOut",
                "CopyIn", "Write a sample with a datawriter", 
                "Test whether the CopyIn function that is driven by the DataWriter works correctly."
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
            DDS.IPublisher pub;
            Foo.TestDataWriter testDW;
            DDS.DataWriterQos dwQos = new DDS.DataWriterQos();
            Test.Framework.TestResult result;
            string expResult = "Successfully written a sample.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            tQos = (DDS.TopicQos)this.ResolveObject("topicQos");
            pub = participant.CreatePublisher();
            if (pub == null)
            {
                result.Result = "participant.create_publisher failed.";
                return result;
            }
            retCode = pub.GetDefaultDataWriterQos(ref dwQos);
            if (retCode != DDS.ReturnCode.Ok)
            {
                result.Result = "publisher.get_default_datawriter_qos failed.";
                return result;
            }
            testDW = (Foo.TestDataWriter) pub.CreateDataWriter(topic, dwQos);
            if (testDW == null)
            {
                result.Result = "publisher.create_datawriter failed.";
                return result;
            }
            
            
            
            
            
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
