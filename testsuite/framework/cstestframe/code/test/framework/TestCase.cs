using System;
using System.Collections;

namespace Test.Framework
{
    /// <class>TestCase</class>
    /// <date>Jun 16, 2004</date>
    /// <brief></brief>
    public abstract class TestCase
    {
        protected string testcaseId;
        protected string component;
        protected string function;
        protected string title;
        protected string purpose;
        protected string input;
        protected TestFramework testFramework;
        private readonly ArrayList preItems = new ArrayList();
        private readonly ArrayList postItems = new ArrayList();
        private readonly Hashtable registered = new Hashtable();

        public virtual string Function
        {
            get
            {
                return function;
            }
        }

        public virtual string Input
        {
            get
            {
                return input;
            }
        }

        public virtual string Purpose
        {
            get
            {
                return purpose;
            }
        }

        public virtual string TestcaseId
        {
            get
            {
                return testcaseId;
            }
        }

        public virtual string Title
        {
            get
            {
                return title;
            }
        }

        public virtual string Component
        {
            get
            {
                return component;
            }
        }

        public TestCase(string testcaseId, string component, string function, string title,
            string purpose, string input)
        {
            this.testcaseId = testcaseId;
            this.component = component;
            this.function = function;
            this.title = title;
            this.purpose = purpose;
            this.input = input;
        }

        public void AddPreItem(TestItem item)
        {
            preItems.Add(item);
        }

        public void AddPostItem(TestItem item)
        {
            postItems.Add(item);
        }

        public TestResult Execute(TestFramework fw)
        {
            TestItem item;
            TestResult result = null;
            TestResult itemResult;
            bool proceed = true;
            int i;
            this.testFramework = fw;
            for (i = 0; i < preItems.Count && proceed; i++)
            {
                item = (TestItem)preItems[i];
                fw.TestItem(item.title);
                try
                {
                    itemResult = item.Run(this);
                    fw.ItemComposeVerdict(itemResult.ExpectedVerdict, itemResult.Verdict);
                    proceed = item.MayProceed(itemResult);
                }
                catch (System.Exception exc)
                {
                    fw.TestMessage(TestMessage.Error, GetExceptionReport(exc));
                    itemResult = new TestResult("UNKNOWN", exc.Message, TestVerdict
                        .Pass, TestVerdict.Fail);
                    fw.ItemComposeVerdict(itemResult.ExpectedVerdict, itemResult.Verdict);
                    proceed = false;
                }
            }
            while (i < preItems.Count)
            {
                item = (TestItem)preItems[i];
                fw.TestItem(item.title);
                fw.ItemComposeVerdict(TestVerdict.Unresolved, TestVerdict
                    .Unresolved);
                i++;
            }
            if (proceed)
            {
                try
                {
                    result = this.Run();
                }
                catch (System.Exception exc)
                {
                    fw.TestMessage(TestMessage.Error, GetExceptionReport(exc));
                    result = new TestResult("UNKNOWN", exc.Message, TestVerdict.Pass,
                        TestVerdict.Fail);
                }
            }
            else
            {
                result = new TestResult("Test succeeded", "Test could not be initialized",
                    TestVerdict.Unresolved, TestVerdict.Unresolved);
            }
            proceed = true;
            for (i = 0; i < postItems.Count && proceed; i++)
            {
                item = (TestItem)postItems[i];
                fw.TestItem(item.title);
                try
                {
                    itemResult = item.Run(this);
                    fw.ItemComposeVerdict(itemResult.ExpectedVerdict, itemResult.Verdict);
                    proceed = item.MayProceed(itemResult);
                }
                catch (System.Exception exc)
                {
                    fw.TestMessage(TestMessage.Error, GetExceptionReport(exc));
                    itemResult = new TestResult("UNKNOWN", exc.Message, TestVerdict.Pass,
                        TestVerdict.Fail);
                    fw.ItemComposeVerdict(itemResult.ExpectedVerdict, itemResult.Verdict);
                    proceed = false;
                }
            }
            if (i < postItems.Count)
            {
                result.Verdict = TestVerdict.Unresolved;
            }
            while (i < postItems.Count)
            {
                item = (TestItem)postItems[i];
                fw.TestItem(item.title);
                fw.ItemComposeVerdict(TestVerdict.Unresolved, TestVerdict.Unresolved);
                i++;
            }
            return result;
        }

        public object ResolveObject(string name)
        {
            return registered[name];
        }

        public void RegisterObject(string name, object obj)
        {
            registered[name] = obj;
        }

        public bool UnregisterObject(string name)
        {
            if (registered.ContainsKey(name))
            {
                registered.Remove(name);
                return true;
            }
            else
                return false;
        }

        public abstract TestResult Run();

        private string GetExceptionReport(System.Exception exc)
        {
            return exc.ToString();
            //string result;
            //result = "Unhandled exception(" + exc.ToString() + ") occurred:\n";
            //java.lang.StackTraceElement[] elements = exc.st.getStackTrace();
            //for (int i = 0; i < elements.Length; i++)
            //{
            //    result += "                         at " + elements[i].getFileName() + ", line " 
            //        + elements[i].getLineNumber() + " (method '" + elements[i].getMethodName() + "')\n";
            //}
            //return result;
        }
    }
}
