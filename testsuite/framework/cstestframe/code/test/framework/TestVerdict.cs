namespace Test.Framework
{
    /// <class>TestVerdict</class>
    /// <date>Jun 16, 2004</date>
    /// <brief></brief>
    public enum TestVerdict
    {
        Pass = 0,
        XPass = 1,
        Fail = 2,
        XFail = 3,
        Unresolved = 4,
        Untested = 5,
        Unsupported = 6,
    }

    public static class TestVerdictString
    {
        public static string GetValue(TestVerdict tv)
        {
            switch (tv)
            {
                case TestVerdict.Pass:
                    return "PASS";
                case TestVerdict.XPass:
                    return "XPASS";
                case TestVerdict.Fail:
                    return "FAIL";
                case TestVerdict.XFail:
                    return "XFAIL";
                case TestVerdict.Unresolved:
                    return "UNRESOLVED";
                case TestVerdict.Untested:
                    return "UNTESTED";
                case TestVerdict.Unsupported:
                    return "Unsupported";
                default:
                    return string.Empty;
            }
        }
    }
}
