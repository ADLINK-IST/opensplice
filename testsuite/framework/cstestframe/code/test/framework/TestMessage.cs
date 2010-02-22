using System;

namespace Test.Framework
{
    /// <class>TestMessage</class>
    /// <date>Jun 16, 2004</date>
    /// <brief></brief>
    public enum TestMessage
    {
        Error = 0,
        Warning = 1,
        Note = 2,
    }

    public static class TestMessageString
    {
        public static string GetValue(TestMessage msg)
        {
            switch (msg)
            {
                case TestMessage.Error:
                    return "ERROR";
                case TestMessage.Warning:
                    return "WARNING";
                case TestMessage.Note:
                    return "NOTE";
                default:
                    return string.Empty;
            }
        }
    }
}
