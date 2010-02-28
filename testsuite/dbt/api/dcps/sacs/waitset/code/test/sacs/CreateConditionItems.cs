namespace test.sacs
{
    /// <summary>Creates 3 new GuardCondition objects.</summary>
    /// <remarks>
    /// Creates 3 new GuardCondition objects. The conditions are stored in the
    /// preItems list as:
    /// <ul>
    /// <li>condition1</li>
    /// <li>condition2</li>
    /// <li>condition3</li>
    /// </ul>
    /// </remarks>
    public class CreateConditionItems : Test.Framework.TestItem
    {
        public CreateConditionItems()
            : base("create three new GuardConditions")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            Test.Framework.TestResult result = null;
            //DDS.DomainParticipantFactory.Instance;
            DDS.GuardCondition condition1 = new DDS.GuardCondition();
            DDS.GuardCondition condition2 = new DDS.GuardCondition();
            DDS.GuardCondition condition3 = new DDS.GuardCondition();
            testCase.RegisterObject("condition1", condition1);
            testCase.RegisterObject("condition2", condition2);
            testCase.RegisterObject("condition3", condition3);

            // TODO: JLS, should one of these TestVerdicts be Fail?
            result = new Test.Framework.TestResult("creation of 3 GuardCondions is succesfull"
                , "succesfully created 3 GuardConditions", Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Pass);

            return result;
        }
    }
}
