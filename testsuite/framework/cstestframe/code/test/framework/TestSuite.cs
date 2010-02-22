using System;
using System.Collections;
using System.Text;
using System.IO;

namespace Test.Framework
{
    /// <class>TestSuite</class>
    /// <date>Jun 16, 2004</date>
    /// <brief></brief>
    public class TestSuite
    {
        private readonly ArrayList testCases = new ArrayList();
        private readonly ArrayList timers = new ArrayList();
        private readonly TestFramework framework = new TestFramework();
        private DateTime startTime;
        private StringWriter testWriter;
        private readonly string dashLine = "-----------------------------------------------------------------------------------------------";
        private readonly string starLine = "***********************************************************************************************";

        public TestSuite()
        {
            testWriter = new StringWriter();
            testWriter.WriteLine("TestCase report");
            testWriter.WriteLine(dashLine);
            testWriter.WriteLine("TestCases");
            testWriter.WriteLine(dashLine);
            testWriter.WriteLine("Start time\t\t| Stop time\t\t| dt(ms)\t| Verdict\t| Testcase");
            testWriter.WriteLine(dashLine);
        }

        public virtual void AddTest(TestCase tc)
        {
            if (tc != null)
            {
                testCases.Add(tc);
            }
        }

        public virtual void AddMilestone(string description)
        {
            MileStone tp = new MileStone(this, description);
            testCases.Add(tp);
            timers.Add(tp);
        }

        public virtual void PrintReport()
        {
            Console.WriteLine(GetReport());
        }

        public virtual string GetReport()
        {
            StringBuilder builder = new StringBuilder();
            builder.Append(testWriter.ToString());
            builder.AppendLine();
            builder.AppendLine();
            builder.AppendLine(dashLine);
            builder.AppendLine("Milestones");
            builder.AppendLine(dashLine);
            builder.AppendLine("Time\t\t\t| dt start (ms)\t| dt prev (ms)\t| Description");
            builder.AppendLine(dashLine);

            DateTime prevTime = startTime;
            for (int i = 0; i < timers.Count; i++)
            {
                MileStone tp = (MileStone)timers[i];
                DateTime dateTime = DateTime.Now;
                TimeSpan totalTimeSpan = dateTime - startTime;
                TimeSpan prevTimeSpan = dateTime - prevTime;
                int totalMilliseconds = (int)Math.Round(totalTimeSpan.TotalMilliseconds);
                builder.Append(GetDateString(dateTime) + "\t| " + totalMilliseconds);
                if (totalMilliseconds < 100000)
                {
                    builder.Append("\t\t| ");
                }
                else
                {
                    builder.Append("\t| ");
                }

                int prevMilliseconds = (int)Math.Round(prevTimeSpan.TotalMilliseconds);
                builder.Append(prevMilliseconds);
                if (prevMilliseconds < 100000)
                {
                    builder.Append("\t\t| ");
                }
                else
                {
                    builder.Append("\t| ");
                }
                builder.AppendLine(tp.Description);
                prevTime = dateTime;
            }
            builder.AppendLine(dashLine);

            return builder.ToString();
        }

        public virtual TestVerdict[] RunTests()
        {
            if (testCases.Count > 0)
            {
                TestVerdict[] results = new TestVerdict[testCases.Count];
                TestCase tc;
                object obj;
                startTime = DateTime.Now;
                for (int i = 0; i < testCases.Count; i++)
                {
                    obj = testCases[i];
                    if (obj is TestCase)
                    {
                        tc = (TestCase)testCases[i];
                        DateTime start = DateTime.Now;
                        results[i] = this.RunTest(tc);
                        DateTime stop = DateTime.Now;
                        int milliseconds = (int)Math.Round((stop - start).TotalMilliseconds);
                        testWriter.Write(GetDateString(start) + "\t| " +
                            GetDateString(stop) + "\t| " + milliseconds);

                        if (milliseconds > 100000)
                        {
                            testWriter.Write("\t| ");
                        }
                        else
                        {
                            testWriter.Write("\t\t| ");
                        }
                        string result = TestVerdictString.GetValue(results[i]);
                        if (result.Length > 5)
                        {
                            testWriter.Write(result + "\t| ");
                        }
                        else
                        {
                            testWriter.Write(result + "\t\t| ");
                        }
                        testWriter.WriteLine(tc.TestcaseId);
                    }
                    else
                    {
                        if (obj is TestSuite.MileStone)
                        {
                            TestSuite.MileStone tp = (TestSuite.MileStone)obj;
                            tp.SetTime();
                        }
                    }
                }
                try
                {
                    string result = this.GetReport();
                    if (result != null)
                    {
                        DateTime cal = DateTime.Now;
                        StreamWriter writer = new StreamWriter("performance.log", true);
                        string userName = Environment.UserName;
                        string osName = Environment.OSVersion.Platform.ToString();
                        string osArch = "Running On Other";
                        Type t = Type.GetType("Mono.Runtime");
                        if (t != null)
                        {
                            osArch = "Running On Mono";
                        }
                        string osVersion = Environment.OSVersion.VersionString;
                        writer.WriteLine(starLine);
                        writer.WriteLine("TEST TIME: " + GetDateString(startTime) + " - " + GetDateString(cal));
                        writer.WriteLine("USER     : " + userName);
                        writer.WriteLine("PLATFORM : " + osName + " " + osVersion + " " + osArch);
                        writer.WriteLine(result);
                        writer.Flush();
                        writer.Close();
                    }
                }
                catch (System.IO.IOException)
                {
                }
                return results;
            }
            return null;
        }

        private string GetDateString()
        {
            return GetDateString(DateTime.Now);
        }

        private string GetDateString(DateTime dateTime)
        {
            return string.Format("{0:yyyy/MM/dd HH:mm:ss}", dateTime);
        }

        private TestVerdict RunTest(TestCase tc)
        {
            lock (this)
            {
                framework.TestStart(tc.TestcaseId, tc.Component, tc.Function);
                if (tc.Title != null)
                {
                    framework.TestTitle(tc.Title);
                }
                if (tc.Purpose != null)
                {
                    framework.TestPurpose(tc.Purpose);
                }
                if (tc.Input != null)
                {
                    framework.TestInput(tc.Input);
                }
                TestResult result = tc.Execute(framework);
                TestVerdict verdict = framework.TestResult(result.ExpectedResult,
                    result.Result, result.ExpectedVerdict, result.Verdict);
                framework.TestFinish();
                return verdict;
            }
        }

        private class MileStone
        {
            private readonly TestSuite enclosing;
            private readonly string description;
            private DateTime? time = null;

            public MileStone(TestSuite enclosing, string description)
            {
                this.enclosing = enclosing;
                this.description = description;
            }

            public virtual void SetTime()
            {
                time = DateTime.Now;
            }

            public virtual DateTime? Time
            {
                get
                {
                    return time;
                }
            }

            public virtual string Description
            {
                get
                {
                    return description;
                }
            }
        }
    }
}
