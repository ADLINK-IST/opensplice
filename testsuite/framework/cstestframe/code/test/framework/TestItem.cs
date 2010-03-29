using System;

namespace Test.Framework
{
    /// <date>May 30, 2005</date>
    public abstract class TestItem
    {
        public readonly string title = null;

        public TestItem(string title)
        {
            this.title = title;
        }

        public virtual bool MayProceed(TestResult result)
        {
            TestVerdict vd = result.Verdict;
            if (vd == TestVerdict.Pass || vd == TestVerdict.XPass || vd == TestVerdict.XFail)
            {
                return true;
            }
            return false;
        }

        public abstract TestResult Run(TestCase testCase);
    }
}
