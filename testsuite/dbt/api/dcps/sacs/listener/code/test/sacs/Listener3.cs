namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener3 : Test.Framework.TestCase
    {
        public Listener3()
            : base("sacs_listener_tc3", "sacs_listener", "listener", "Test if a DataWriterListener works."
                , "Test if a DataWriterListener works.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter datawriter;
            DDS.IPublisher publisher;
            DDS.ITopic topic;
            DDS.DataWriterQos wQos;
            Test.Framework.TestResult result;
            test.sacs.MyDataWriterListener listener;
            DDS.ReturnCode rc;
            string expResult = "DataWriterListener test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            wQos = (DDS.DataWriterQos)this.ResolveObject("datawriterQos");
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            listener = new test.sacs.MyDataWriterListener();
            datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, wQos, listener, DDS.StatusKind.Any);
            if (datawriter == null)
            {
                result.Result = "DataWriter could not be created.";
                return result;
            }
            rc = datawriter.SetListener(null, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Null Listener could be attached.";
                return result;
            }
            rc = datawriter.SetListener(listener, (DDS.StatusKind)1012131412);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Invalid mask could be used when attaching a listener.";
                return result;
            }
            rc = datawriter.SetListener(listener, DDS.StatusKind.PublicationMatched);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (2).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
