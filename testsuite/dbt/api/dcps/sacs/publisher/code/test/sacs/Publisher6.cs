namespace test.sacs
{
    /// <summary>
    /// Tests the initial default DataWriterQos and the getting and setting
    /// of the default DataWriterQos in the Publisher.
    /// </summary>
    /// <remarks>
    /// Tests the initial default DataWriterQos and the getting and setting
    /// of the default DataWriterQos in the Publisher.
    /// </remarks>
    public class Publisher6 : Test.Framework.TestCase
    {
        /// <summary>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </summary>
        /// <remarks>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </remarks>
        public Publisher6()
            : base("sacs_publisher_tc6", "sacs_publisher", "publisher", "Additional publisher test."
                , "Additional publisher test.", null)
        {
            this.AddPreItem(new test.sacs.PublisherItemInit());
            this.AddPreItem(new test.sacs.TopicInit());
            this.AddPostItem(new test.sacs.TopicDeinit());
            this.AddPostItem(new test.sacs.PublisherItemDeinit());
        }

        /// <summary>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </summary>
        /// <remarks>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </remarks>
        public override Test.Framework.TestResult Run()
        {
            DDS.IPublisher publisher;
            string expResult = "Functions all supported.";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            rc = publisher.BeginCoherentChanges();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "begin_coherent_changes failed.";
                return result;
            }
            rc = publisher.EndCoherentChanges();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "end_coherent_changes failed.";
                return result;
            }
            rc = publisher.SuspendPublications();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "suspend_publication failed.";
                return result;
            }
            rc = publisher.ResumePublications();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "resume_publication failed.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
