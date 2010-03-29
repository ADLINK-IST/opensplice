namespace test.sacs
{
    /// <summary>Tests the ability to attach and to lookup conditions.</summary>
    /// <remarks>Tests the ability to attach and to lookup conditions.</remarks>
    public class Waitset1 : Test.Framework.TestCase
    {
        public Waitset1()
            : base("sacs_waitset_tc1", "waitset", "waitset", "test the attach_condition and the get_conditions functions"
                , "conditions", null)
        {
            AddPreItem(new test.sacs.CreateConditionItems());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Resolving GuardCondition succeeded.";
			DDS.ICondition[] holder = null;
            DDS.ReturnCode[] resultCode = new DDS.ReturnCode[] { DDS.ReturnCode.Error, DDS.ReturnCode.Error, DDS.ReturnCode.Error };
            DDS.ReturnCode ddsReturnCode;
            bool continueTesting = true;
            DDS.GuardCondition condition1 = (DDS.GuardCondition)ResolveObject("condition1");
            DDS.GuardCondition condition2 = (DDS.GuardCondition)ResolveObject("condition2");
            DDS.GuardCondition condition3 = (DDS.GuardCondition)ResolveObject("condition3");
            DDS.WaitSet waitset = new DDS.WaitSet();
            resultCode[0] = waitset.AttachCondition(condition1);
            resultCode[1] = waitset.AttachCondition(condition2);
            resultCode[2] = waitset.AttachCondition(condition3);

            if (resultCode[0] != 0 || resultCode[1] != 0 || resultCode[2] != 0)
            {
                result = new Test.Framework.TestResult("attached guardconditions to a waitset", "attach_condition returned RETCODE: "
                     + resultCode[0] + " " + resultCode[1] + " " + resultCode[2], expVerdict, Test.Framework.TestVerdict.Fail);
                continueTesting = false;
            }
            else
            {
                result = new Test.Framework.TestResult(expResult, "get_conditions returned RETCODE_OK"
                    , expVerdict, Test.Framework.TestVerdict.Pass);
            }
            if (continueTesting)
            {
                ddsReturnCode = waitset.GetConditions(ref holder);
                if (ddsReturnCode != DDS.ReturnCode.Ok)
                {
                    result = new Test.Framework.TestResult(expResult, "get_conditions returned RETCODE: "
                         + ddsReturnCode, expVerdict, Test.Framework.TestVerdict.Fail);
                }
                else
                {
                    result = new Test.Framework.TestResult(expResult, "get_conditions returned RETCODE_OK"
                        , expVerdict, Test.Framework.TestVerdict.Pass);
                }
                if (holder.Length != 3)
                {
                    result = new Test.Framework.TestResult(expResult, "Did not resolve 3 GuardCondition objects"
                        , expVerdict, Test.Framework.TestVerdict.Fail);
                }
                else
                {
                    if ((holder[0] != condition1) && (holder[1] != condition1) && (holder[2] != condition1))
                    {
                        result = new Test.Framework.TestResult(expResult, "Resolved GuardCondition objects not OK (1)"
                            , expVerdict, Test.Framework.TestVerdict.Fail);
                        return result;
                    }
                    if ((holder[0] != condition2) && (holder[1] != condition2) && (holder[2] != condition2))
                    {
                        result = new Test.Framework.TestResult(expResult, "Resolved GuardCondition objects not OK (2)"
                            , expVerdict, Test.Framework.TestVerdict.Fail);
                        return result;
                    }
                    if ((holder[0] != condition3) && (holder[1] != condition3) && (holder[2] != condition3))
                    {
                        result = new Test.Framework.TestResult(expResult, "Resolved GuardCondition objects not OK (3)"
                            , expVerdict, Test.Framework.TestVerdict.Fail);
                        return result;
                    }
                    result = new Test.Framework.TestResult(expResult, "Resolved 3 GuardCondition objects"
                        , expVerdict, Test.Framework.TestVerdict.Pass);
                }
            }
            return result;
        }
    }
}
