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
    public class Publisher4 : Test.Framework.TestCase
    {
        /// <summary>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </summary>
        /// <remarks>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </remarks>
        public Publisher4()
            : base("sacs_publisher_tc4", "sacs_publisher", "publisher", "Test if a DataWriterQos can be resolved/set."
                , "Test if a DataWriterQos can be resolved/set.", null)
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
			DDS.DataWriterQos qos = null;
            string expResult = "DataWriterQos test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");

            if (publisher.GetDefaultDataWriterQos(ref qos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Failed to get the default datawriter qos (1)";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(qos, test.sacs.QosComparer.defaultDataWriterQos
                ))
            {
                result.Result = "The default DataWriterQos != default (2)";
                return result;
            }
            qos.Deadline.Period.NanoSec = DDS.Duration.InfiniteNanoSec + 1;
            rc = publisher.SetDefaultDataWriterQos(qos);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "Received return code " + rc + " but expected RETCODE_BAD_PARAMETER (3)";
                return result;
            }

            if (publisher.GetDefaultDataWriterQos(ref qos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Failed to get the default datawriter qos (4)";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(qos, test.sacs.QosComparer.defaultDataWriterQos))
            {
                result.Result = "The default DataWriterQos != default (4)";
                return result;
            }
            qos.History.Depth = 8;
            qos.ResourceLimits.MaxSamplesPerInstance = 2;
            rc = publisher.SetDefaultDataWriterQos(qos);
            if (rc != DDS.ReturnCode.InconsistentPolicy)
            {
                result.Result = "Received return code " + rc + " but expected RETCODE_INCONSISTENT_POLICY (5)";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
