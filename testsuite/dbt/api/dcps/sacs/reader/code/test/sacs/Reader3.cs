namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Reader3 : Test.Framework.TestCase
    {
        public Reader3()
            : base("sacs_reader_tc3", "sacs_reader", "sacs_reader", "test reader lookup_instance"
                , "test reader lookup instance", null)
        {
            this.AddPreItem(new test.sacs.ReaderInit());
            this.AddPostItem(new test.sacs.ReaderDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataReader reader;
            mod.tstDataWriter writer;
            long handle;

            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            string expResult = "reader::lookup_instance test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            mod.tst t = new mod.tst();
            t.long_1 = 1;
            t.long_2 = 1;
            t.long_3 = 1;

            rc = writer.Write(t);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "write failed";
                return result;
            }

            // lookup instance with writer...
            handle = writer.LookupInstance(t);
            if (handle == 0)
            {
                result.Result = "writer LookupInstance, result == HANDLE_NIL";
                return result;
            }

            handle = reader.LookupInstance(t);
            if (handle == 0)
            {
                result.Result = "reader LookupInstance, result == HANDLE_NIL";
                return result;
            }

            mod.tst[] data = new mod.tst[0];
            DDS.SampleInfo[] info = new DDS.SampleInfo[0];
            rc = reader.ReadInstance(ref data, ref info, 1, handle, DDS.SampleStateKind.Any, DDS.ViewStateKind.Any,
                DDS.InstanceStateKind.Any);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "read_instance failed";
                return result;
            }
            if ((t.long_1 != data[0].long_1) || (t.long_2 != data[0].long_2) || (
                t.long_3 != data[0].long_3))
            {
                result.Result = "incorrect data read";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
