namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Condition4 : Test.Framework.TestCase
    {
        public Condition4()
            : base("sacs_condition_tc4", "sacs_condition", "sacs_condition",
                "test statuscondition", "test statuscondition", null)
        {
            this.AddPreItem(new test.sacs.ConditionInit());
            this.AddPostItem(new test.sacs.ConditionDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            mod.tstDataReader reader;
            DDS.IStatusCondition condition;
            DDS.WaitSet waitset;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            bool value;
            DDS.StatusKind statusMask;
            DDS.ICondition[] holder;
            DDS.SubscriptionMatchedStatus smStatus = new DDS.SubscriptionMatchedStatus();
            DDS.LivelinessChangedStatus lcStatus = new DDS.LivelinessChangedStatus();
            string expResult = "ReadCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);

            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            condition = reader.StatusCondition;
            if (condition == null)
            {
                result.Result = "Could not resolve status condition.";
                return result;
            }
            waitset = new DDS.WaitSet();
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "attach_condition failed.";
                return result;
            }
            try
            {
                System.Threading.Thread.Sleep(3000);
            }
            catch (System.Exception)
            {
                System.Console.Error.WriteLine("Sleep failed...");
            }
            holder = new DDS.Condition[0];
            rc = waitset.Wait(ref holder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "wait failed (1). Retcode == " + rc;
                return result;
            }
            if (holder.Length != 1)
            {
                System.Console.Out.WriteLine("Holder length : " + holder.Length);
                System.Console.Out.WriteLine("Status changes: " + reader.StatusChanges);
                reader.GetSubscriptionMatchedStatus(ref smStatus);
                System.Console.Out.WriteLine("Total count   : " + smStatus.TotalCount);
                reader.GetLivelinessChangedStatus(ref lcStatus);
                System.Console.Out.WriteLine("Alive count   : " + lcStatus.AliveCount);
                result.Result = "wait should return 1 condition but didn't (1).";
                return result;
            }
            rc = reader.GetLivelinessChangedStatus(ref lcStatus);
            
            DDS.LivelinessChangedStatus status = lcStatus;
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "GetLivelinessChangedStatus call failed.";
                return result;
            }
            if (status.AliveCount != 1)
            {
                result.Result = "LivelinessChanged.AliveCount != 1. (" + status.AliveCount +
                    ").";
                return result;
            }
            if (status.AliveCountChange != 1)
            {
                result.Result = "LivelinessChanged.AliveCountChange != 1." + status.AliveCountChange
                     + ").";
                return result;
            }

            value = condition.GetTriggerValue();
            if (value)
            {
                result.Result = "GetTriggerValue returned true. " + reader.StatusChanges;
                return result;
            }
            statusMask = condition.GetEnabledStatuses();
            rc = condition.SetEnabledStatuses(statusMask);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "SetEnabledStatuses failed (1).";
                return result;
            }
            if (condition.GetEnabledStatuses() != statusMask)
            {
                result.Result = "GetEnabledStatuses does not match the applied one.";
                return result;
            }
            if (condition.GetEntity() != reader)
            {
                result.Result = "GetEntity does not return the correct entity.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
