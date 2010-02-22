using System;

namespace Test.Framework
{
    /// <class>TestResult</class>
    /// <date>Jun 16, 2004</date>
    /// <brief></brief>
    public class TestResult
    {
        private string expectedResult;
        private string actualResult;
        private TestVerdict expectedVerdict;
        private TestVerdict actualVerdict;

        public TestResult(string expectedResult, string actualResult, TestVerdict expectedVerdict
            , TestVerdict actualVerdict)
        {
            this.expectedResult = expectedResult;
            this.actualResult = actualResult;
            this.expectedVerdict = expectedVerdict;
            this.actualVerdict = actualVerdict;
        }

        public virtual string ExpectedResult
        {
            get
            {
                return expectedResult;
            }
        }

        public virtual TestVerdict ExpectedVerdict
        {
            get
            {
                return expectedVerdict;
            }
            set
            {
                expectedVerdict = value;
            }
        }

        public virtual string Result
        {
            get
            {
                return actualResult;
            }
            set
            {
                actualResult = value;
            }
        }

        public virtual TestVerdict Verdict
        {
            get
            {
                return actualVerdict;
            }
            set
            {
                actualVerdict = value;
            }
        }
    }
}
