using System;
using System.Threading;
using System.Collections.Generic;
namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener6 : Test.Framework.TestCase
    {
        Dictionary<DDS.StatusKind, Semaphore> semaphoresParticipant = null;
        Dictionary<DDS.StatusKind, Semaphore> semaphoresReader = null;
        public Listener6()
            : base("sacs_listener_tc6", "sacs_listener", "listener", "Test if a DomainParticipantListener works."
                , "Test if a DomainParticipantListener works.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            mod.tst[] tstHolder;
            DDS.SampleInfo[] sampleInfoHolder;
            Test.Framework.TestResult result;
            test.sacs.MyParticipantListener listener;
            test.sacs.MyDataReaderListener listener2;
            string expResult = "DomainParticipantListener test succeeded.";
            DDS.ReturnCode rc;
            semaphoresParticipant = new Dictionary<DDS.StatusKind, Semaphore>();
            semaphoresParticipant.Add(DDS.StatusKind.DataAvailable, new Semaphore(0, 1));

            semaphoresReader = new Dictionary<DDS.StatusKind, Semaphore>();
            semaphoresReader.Add(DDS.StatusKind.DataAvailable, new Semaphore(0, 2));
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            listener = new test.sacs.MyParticipantListener(semaphoresParticipant);
            listener2 = new test.sacs.MyDataReaderListener(semaphoresReader);
            rc = participant.SetListener(listener, (DDS.StatusKind)0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_listener on DomainParticipant failed.";
                return result;
            }
            rc = participant.SetListener(null, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Null Listener could not be attached.";
                return result;
            }
            rc = participant.SetListener(listener, (DDS.StatusKind)1012131412);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (2).";
                return result;
            }
            rc = participant.SetListener(listener, DDS.StatusKind.DataAvailable);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (3).";
                return result;
            }
            mod.tst data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;
            rc = writer.Write(data, 0L);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed.";
                return result;
            }
            if (semaphoresParticipant[DDS.StatusKind.DataAvailable].WaitOne(10000))
            {
                if (!listener.onDataAvailableCalled)
                {
                    result.Result = "on_data_available not called.";
                    return result;
                }
            }
            else
            {
                result.Result = "on_data_available did not trigger";
                return result;
            }
            listener.Reset();
            tstHolder = new mod.tst[0];
            sampleInfoHolder = new DDS.SampleInfo[0];
            rc = reader.Take(ref tstHolder, ref sampleInfoHolder, 1, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataReader.take failed.";
                return result;
            }
            rc = reader.SetListener(listener2, DDS.StatusKind.DataAvailable);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (4).";
                return result;
            }
            rc = writer.Write(data, 0L);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "tstDataWriter.write failed.";
                return result;
            }
            try
            {
                System.Threading.Thread.Sleep(1000);
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine(e);
            }
            if (listener.onDataAvailableCalled)
            {
                result.Result = "on_data_available is called but shouldn't be.";
                return result;
            }
            if (!listener2.onDataAvailableCalled)
            {
                result.Result = "on_data_available not called (2).";
                return result;
            }
            listener.Reset();
            listener2.Reset();
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
