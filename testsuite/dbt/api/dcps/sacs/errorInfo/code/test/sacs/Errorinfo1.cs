namespace test.sacs
{
    public class Errorinfo1 : Test.Framework.TestCase
    {
        public Errorinfo1()
            : base("sacs_errorinfo_tc1", "errorinfo", "errorinfo", "create an errorinfo instance and retrieve its items"
                , "Entity can be created but no info is returned before error reported ", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string sh;
            DDS.ErrorCode eh;
            DDS.ReturnCode ddsReturnCode;

            if (DDS.ErrorInfo.Update() != DDS.ReturnCode.NoData)
            {
                return new Test.Framework.TestResult("Update returns NO_DATA prior to error", "Update did not return NO_DATA prior to error"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            sh = "<>";
            ddsReturnCode = DDS.ErrorInfo.GetLocation(out sh);
            if ((ddsReturnCode != DDS.ReturnCode.NoData))// || (!sh.Equals("<>")))
            {
                return new Test.Framework.TestResult("get_location returns NO_DATA and does not modify string"
                    , "Different result and/or string modified", expVerdict, Test.Framework.TestVerdict.Fail);
            }
            ddsReturnCode = DDS.ErrorInfo.GetMessage(out sh);
            if ((ddsReturnCode != DDS.ReturnCode.NoData))// || (!sh.Equals("<>")))
            {
                return new Test.Framework.TestResult("get_message returns NO_DATA and does not modify string"
                    , "Different result and/or string modified", expVerdict, Test.Framework.TestVerdict.Fail);
            }
            ddsReturnCode = DDS.ErrorInfo.GetStackTrace(out sh);
            if ((ddsReturnCode != DDS.ReturnCode.NoData))// || (!sh.Equals("<>")))
            {
                return new Test.Framework.TestResult("get_stack_trace returns NO_DATA and does not modify string"
                    , "Different result and/or string modified", expVerdict, Test.Framework.TestVerdict.Fail);
            }
            ddsReturnCode = DDS.ErrorInfo.GetSourceLine(out sh);
            if ((ddsReturnCode != DDS.ReturnCode.NoData))// || (!sh.Equals("<>")))
            {
                return new Test.Framework.TestResult("get_source_line returns NO_DATA and does not modify string"
                    , "Different result and/or string modified", expVerdict, Test.Framework.TestVerdict.Fail);
            }
            eh = (DDS.ErrorCode)(-1);
            ddsReturnCode = DDS.ErrorInfo.GetCode(out eh);
            if ((ddsReturnCode != DDS.ReturnCode.NoData) || (eh != (DDS.ErrorCode)(-1)))
            {
                return new Test.Framework.TestResult("get_code returns NO_DATA and does not modify error code"
                    , "Different result and/or error code modified", expVerdict, Test.Framework.TestVerdict.Fail);
            }
            return new Test.Framework.TestResult("sacs_errorinfo_tc1 successfull", "sacs_errorinfo_tc1_successfull"
                , expVerdict, Test.Framework.TestVerdict.Pass);
        }
    }
}
