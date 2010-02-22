namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Condition1 : Test.Framework.TestCase
    {
        public Condition1()
            : base("sacs_condition_tc1", "sacs_condition", "sacs_condition",
                "test guard condition", "test guard condition", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.GuardCondition condition;
            DDS.WaitSet waitset;
            Test.Framework.TestResult result;
            Condition1.GuardConditionWaitset threadedWaitset;
            DDS.ReturnCode rc;
            string expResult = "StatusCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            condition = new DDS.GuardCondition();
            waitset = new DDS.WaitSet();
            waitset.AttachCondition(condition);
            DDS.ICondition[] activeConditions = new DDS.ICondition[0];
            rc = waitset.Wait(ref activeConditions, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (activeConditions.Length > 0)
            {
                result.Result = "WaitSet.Wait returned condition where it shouldn't.";
                return result;
            }
            threadedWaitset = new test.sacs.Condition1.GuardConditionWaitset(this, waitset);
            threadedWaitset.Start();
            condition.SetTriggerValue(true);

            try
            {
                threadedWaitset.Join();
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine(e);
            }
            if (!threadedWaitset.Succeeded())
            {
                result.Result = "GuardCondition trigger does not work properly.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }

        private class GuardConditionWaitset
        {
            private System.Threading.Thread thread;
            private DDS.WaitSet waitset;

            private bool success;

            public GuardConditionWaitset(Condition1 _enclosing, DDS.WaitSet waitset)
            {
                this._enclosing = _enclosing;
                this.waitset = waitset;
            }

            public void Run()
            {
                thread = new System.Threading.Thread(delegate()
                {
                    this.success = false;
                    DDS.ICondition[] attachedConditions = null;
                    DDS.ReturnCode rc = this.waitset.GetConditions(ref attachedConditions);
                    if (rc == DDS.ReturnCode.Ok)
                    {
                        DDS.ICondition[] activeConditions = new DDS.ICondition[0];
                        rc = this.waitset.Wait(ref activeConditions, new DDS.Duration(30, 0));
                        if (rc == DDS.ReturnCode.Ok)
                        {
                            if (activeConditions.Length != 0)
                            {
                                this.success = true;
                            }
                        }
                    }
                });

                thread.Start();
            }

            public bool Succeeded()
            {
                return this.success;
            }

            private readonly Condition1 _enclosing;

            internal void Start()
            {
                Run();
            }

            internal void Join()
            {
                thread.Join();
            }
        }
    }
}
