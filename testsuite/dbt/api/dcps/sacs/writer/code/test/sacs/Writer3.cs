namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Writer3 : Test.Framework.TestCase
    {
        public Writer3()
            : base("sacs_writer_tc3", "sacs_writer", "sacs_writer", "test lookup_instance datawriter actions"
                , "test lookup_instance datawriter actions", null)
        {
            this.AddPreItem(new test.sacs.WriterInit());
            this.AddPostItem(new test.sacs.WriterDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataWriter writer;
            long handle;
            long handle2;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            mod.tst data = new mod.tst();
            data.long_1 = 1;
            data.long_2 = 2;
            data.long_3 = 3;
            string expResult = "lookup_instance test successful.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            handle = writer.RegisterInstance(data);
            if (handle == DDS.InstanceHandle.Nil)
            {
                result.Result = "register_instance failed.";
                return result;
            }
            handle2 = writer.LookupInstance(data);

            // TODO: JLS, java version is comparing handle to handle!
            if (handle != handle2)
            {
                result.Result = "lookup_instance returned wrong instance.";
                return result;
            }
            rc = writer.UnregisterInstance(data, handle);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "unregister_instance failed.";
                return result;
            }
            handle2 = writer.LookupInstance(data);
            if (handle2 != DDS.InstanceHandle.Nil)
            {
                result.Result = "lookup_instance found non-existing instance.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
