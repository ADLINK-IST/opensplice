using System;
using System.IO;

namespace Test.Framework
{
    /// <class>TestFramework</class>
    /// <date>Jun 15, 2004</date>
    /// <brief></brief>
    public class TestFramework
    {
        private StreamWriter tcWriter;

        public TestFramework()
        {
            tcWriter = null;
        }

        /// <exception cref="System.IO.IOException"></exception>
        private void Write(string txt)
        {
            tcWriter.Write(txt);
            Console.Write(txt);
        }

        private void WriteLine(string txt)
        {
            tcWriter.WriteLine(txt);
            Console.WriteLine(txt);
        }

        private TestVerdict TestComposeVerdict(TestVerdict
             expVerdict, TestVerdict verdict)
        {
            TestVerdict result;
            if (expVerdict.Equals(TestVerdict.Pass))
            {
                if ((verdict.Equals(TestVerdict.Pass)) || (verdict.Equals(TestVerdict.Fail)))
                {
                    result = verdict;
                }
                else
                {
                    result = TestVerdict.Unresolved;
                }
            }
            else
            {
                if (expVerdict.Equals(TestVerdict.Fail))
                {
                    if (verdict.Equals(TestVerdict.Pass))
                    {
                        result = TestVerdict.XPass;
                    }
                    else
                    {
                        if (verdict.Equals(TestVerdict.Fail))
                        {
                            result = TestVerdict.XFail;
                        }
                        else
                        {
                            result = TestVerdict.Unresolved;
                        }
                    }
                }
                else
                {
                    if (expVerdict.Equals(TestVerdict.Unresolved))
                    {
                        result = TestVerdict.Unresolved;
                    }
                    else
                    {
                        if (expVerdict.Equals(TestVerdict.Untested))
                        {
                            if (!(verdict.Equals(TestVerdict.Untested)))
                            {
                                result = TestVerdict.Unresolved;
                            }
                            else
                            {
                                result = TestVerdict.Untested;
                            }
                        }
                        else
                        {
                            if (expVerdict.Equals(TestVerdict.Unsupported))
                            {
                                if (!(verdict.Equals(TestVerdict.Unsupported)))
                                {
                                    result = TestVerdict.Unresolved;
                                }
                                else
                                {
                                    result = TestVerdict.Unsupported;
                                }
                            }
                            else
                            {
                                result = TestVerdict.Unresolved;
                            }
                        }
                    }
                }
            }
            return result;
        }

        public virtual bool TestStart(string tcId, string component, string function)
        {
            if ((component == null) || (tcId == null) || (function == null) || (tcWriter != null
                ))
            {
                return false;
            }

            string tcFileName = component + ".dbt";

            System.Diagnostics.StackTrace stackTrace = new System.Diagnostics.StackTrace(true);
            System.Diagnostics.StackFrame[] frames = stackTrace.GetFrames();

            string file;

            if (frames.Length > 3)
            {
                file = frames[3].GetFileName() + ":" + frames[3].GetFileLineNumber();
            }
            else
            {
                file = frames[frames.Length - 1].GetFileName() + ":" + frames[frames.Length
                     - 1].GetFileLineNumber();
            }
            try
            {
                tcWriter = new StreamWriter(tcFileName, true);
            }
            catch (System.IO.IOException)
            {
                //File could not be created.
                //Sharpen.Runtime.printStackTrace(e);
                return false;
            }
            string userName = Environment.UserName;
            string osName = Environment.OSVersion.Platform.ToString();
            string osArch = "Running On Other";
            Type t = Type.GetType("Mono.Runtime");
            if (t != null)
            {
                osArch = "Running On Mono";
            }
            string osVersion = Environment.OSVersion.VersionString;
            DateTime time = DateTime.Now;
            try
            {
                WriteLine("################");
                WriteLine("TESTCASE ID\t: " + tcId);
                WriteLine("COMPONENT\t: " + component);
                WriteLine("FUNCTION\t: " + function);
                WriteLine("FILE\t\t: " + file);
                WriteLine("USER\t\t: " + userName);
                WriteLine("PLATFORM\t: " + osName + " " + osVersion + " " + osArch);
                WriteLine(string.Format("DATE\t\t: {0:yyyy/MM/dd}", time));
                WriteLine(string.Format("TIME\t\t: {0:HH:mm:ss}", time));
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestTitle(string title)
        {
            try
            {
                this.WriteLine("TITLE\t\t: " + title);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestPurpose(string purpose)
        {
            try
            {
                this.WriteLine("PURPOSE\t\t: " + purpose);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestInput(string input)
        {
            try
            {
                this.WriteLine("INPUT\t\t: " + input);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestMessage(TestMessage messageType, string messageText)
        {
            try
            {
                WriteLine("MESSAGE\t\t: " + TestMessageString.GetValue(messageType) + ": " + messageText);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual TestVerdict TestResult(int exp_result, int result,
            TestVerdict exp_verdict, TestVerdict verdict)
        {
            return TestResult(exp_result.ToString(), result.ToString(), exp_verdict, verdict);
        }

        public virtual TestVerdict TestResult(long exp_result, long result,
            TestVerdict exp_verdict, TestVerdict verdict)
        {
            return TestResult(exp_result.ToString(), result.ToString(), exp_verdict, verdict);
        }

        public virtual TestVerdict TestResult(float exp_result, float result,
            TestVerdict exp_verdict, TestVerdict verdict)
        {
            return TestResult(exp_result.ToString(), result.ToString(), exp_verdict, verdict);
        }

        public virtual TestVerdict TestResult(double exp_result, double result,
            TestVerdict exp_verdict, TestVerdict verdict)
        {
            return TestResult(exp_result.ToString(), result.ToString(), exp_verdict, verdict);
        }

        public virtual TestVerdict TestResult(string exp_result, string result,
            TestVerdict exp_verdict, TestVerdict verdict)
        {
            try
            {
                TestVerdict realVerdict = this.TestComposeVerdict(exp_verdict, verdict
                    );
                this.WriteLine("EXPECTED RESULT\t: " + exp_result);
                this.WriteLine("RESULT\t\t: " + result);
                this.WriteLine("EXPECTED VERDICT: " + TestVerdictString.GetValue(exp_verdict));
                this.WriteLine("TEST VERDICT\t: " + TestVerdictString.GetValue(verdict));
                this.WriteLine("VERDICT\t\t: " + TestVerdictString.GetValue(realVerdict));
                return realVerdict;
            }
            catch (System.IO.IOException)
            {
                return TestVerdict.Untested;
            }
        }

        public virtual bool TestProblemList(string problemReportNumbers)
        {
            try
            {
                this.WriteLine("PROBLEM REPORT:\t: " + problemReportNumbers);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestFinish()
        {
            try
            {
                tcWriter.Flush();
                tcWriter.Close();
                tcWriter = null;
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual bool TestItem(string name)
        {
            try
            {
                this.WriteLine("TEST ITEM\t: " + name);
            }
            catch (System.IO.IOException)
            {
                return false;
            }
            return true;
        }

        public virtual TestVerdict ItemComposeVerdict(TestVerdict exp_verdict,
            TestVerdict verdict)
        {
            try
            {
                TestVerdict realVerdict = this.TestComposeVerdict(exp_verdict, verdict
                    );
                this.WriteLine("EXP ITEM RESULT\t: " + TestVerdictString.GetValue(exp_verdict));
                this.WriteLine("ITEM RESULT\t: " + TestVerdictString.GetValue(verdict));
                this.WriteLine("ITEM VERDICT\t: " + TestVerdictString.GetValue(realVerdict));
                return realVerdict;
            }
            catch (System.IO.IOException)
            {
                return TestVerdict.Untested;
            }
        }
    }
}
