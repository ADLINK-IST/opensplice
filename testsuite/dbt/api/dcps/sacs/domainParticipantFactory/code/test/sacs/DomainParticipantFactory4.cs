namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipantFactory4 : Test.Framework.TestCase
    {
        public DomainParticipantFactory4()
            : base("sacs_domainParticipantFactory_tc4", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Test set default participant qos.", "Test set default participant qos."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Get/Set DomainParticipantQos succeeded.";
            DDS.DomainParticipantFactory factory;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.DomainParticipantQos pqosHolder2 = null;
            DDS.ReturnCode returnCode;
            byte[] ud;
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            returnCode = factory.GetDefaultParticipantQos(ref pqosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            pqosHolder.EntityFactory.AutoenableCreatedEntities = true;
            ud = new byte[1];
            ud[0] = System.Convert.ToByte("4");
            pqosHolder.UserData.Value = ud;
            pqosHolder.WatchdogScheduling.SchedulingClass.Kind = DDS.SchedulingClassQosPolicyKind.ScheduleRealtime;
            pqosHolder.WatchdogScheduling.SchedulingPriorityKind.Kind = DDS.SchedulingPriorityQosPolicyKind.PriorityAbsolute;
            pqosHolder.WatchdogScheduling.SchedulingPriority = 10;
            pqosHolder.ListenerScheduling.SchedulingClass.Kind = DDS.SchedulingClassQosPolicyKind.ScheduleTimesharing;
            pqosHolder.ListenerScheduling.SchedulingPriorityKind.Kind = DDS.SchedulingPriorityQosPolicyKind.PriorityAbsolute;
            pqosHolder.ListenerScheduling.SchedulingPriority = 20;
            returnCode = factory.SetDefaultParticipantQos(pqosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Set default participant qos failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            factory.GetDefaultParticipantQos(ref pqosHolder2);
            if (pqosHolder.EntityFactory.AutoenableCreatedEntities != pqosHolder2.EntityFactory.AutoenableCreatedEntities)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved entity_factory policy does not match the applied one."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.UserData.Value.Length != pqosHolder2.UserData.Value.
                Length)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved UserData policy does not match the applied one."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.UserData.Value[0] != pqosHolder2.UserData.Value[0])
            {
                result = new Test.Framework.TestResult(expResult, "Resolved UserData policy does not match the applied one (2)."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.WatchdogScheduling.SchedulingClass.Kind != pqosHolder2.WatchdogScheduling.SchedulingClass.Kind)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved watchdog_scheduling.scheduling_class policy does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.WatchdogScheduling.SchedulingPriorityKind.Kind != pqosHolder2
                .WatchdogScheduling.SchedulingPriorityKind.Kind)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved watchdog_scheduling.scheduling_priority_kind policy does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.WatchdogScheduling.SchedulingPriority != pqosHolder2.WatchdogScheduling.SchedulingPriority)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved watchdog_scheduling.SchedulingPriority does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.ListenerScheduling.SchedulingClass.Kind != pqosHolder2.ListenerScheduling.SchedulingClass.Kind)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved listener_scheduling.scheduling_class policy does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.ListenerScheduling.SchedulingPriorityKind.Kind != pqosHolder2
                .ListenerScheduling.SchedulingPriorityKind.Kind)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved listener_scheduling.scheduling_priority_kind policy does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.ListenerScheduling.SchedulingPriority != pqosHolder2.ListenerScheduling.SchedulingPriority)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved listener_scheduling.SchedulingPriority does not match the applied one"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
