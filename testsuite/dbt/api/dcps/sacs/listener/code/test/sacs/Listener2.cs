namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener2 : Test.Framework.TestCase
    {
        public Listener2()
            : base("sacs_listener_tc2", "sacs_listener", "listener", "Test if a SubscriberListener works."
                , "Test if a SubscriberListener works.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
            test.sacs.MySubscriberListener listener;
            mod.tstDataWriter datawriter;
            mod.tstDataReader datareader;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "SubscriberListener test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            datawriter = (mod.tstDataWriter)this.ResolveObject("datawriter");
            listener = new test.sacs.MySubscriberListener();
            rc = subscriber.SetListener(listener, DDS.StatusKind.DataOnReaders);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached.";
                return result;
            }
            mod.tst t = new mod.tst();
            t.long_1 = 1;
            t.long_2 = 2;
            t.long_3 = 3;
            rc = datawriter.Write(t, DDS.InstanceHandle.Nil);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Data could not be written.";
                return result;
            }
            try
            {
                System.Threading.Thread.Sleep(5000);
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine(e);
            }
            if (!listener.onDataOnReadersCalled)
            {
                result.Result = "on_data_on_readers does not work properly.";
                return result;
            }
            listener.Reset();
            rc = subscriber.SetListener(null, 0);
            datareader = (mod.tstDataReader)this.ResolveObject("datareader");
            mod.tst[] data = new mod.tst[0];
            DDS.SampleInfo[] info = new DDS.SampleInfo[0];
            rc = datareader.Take(ref data, ref info, DDS.Length.Unlimited, DDS.SampleStateKind.Any,
                DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            if (rc == DDS.ReturnCode.Ok)
            {
                datareader.ReturnLoan(ref data, ref info);
            }
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Null Listener could not be attached.";
                return result;
            }
            rc = subscriber.SetListener(listener, (DDS.StatusKind)1012131412);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (2).";
                return result;
            }
            rc = subscriber.SetListener(listener, DDS.StatusKind.DataOnReaders);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (3).";
                return result;
            }
            try
            {
                System.Threading.Thread.Sleep(5000);
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine(e);
            }
            if (listener.onDataOnReadersCalled)
            {
                result.Result = "on_data_on_readers does not work properly (2).";
                return result;
            }
            listener.Reset();
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
