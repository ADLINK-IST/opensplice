namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant1 : Test.Framework.TestCase
    {
        public DomainParticipant1()
            : base("sacs_domainParticipant_tc1", "sacs_domainParticipant"
                , "domainParticipant", "Test if a DomainParticipant qos can be resolved/set.", "Test if a DomainParticipant qos can be resolved/set."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "DomainParticipantQos get/set succeeded.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
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
            result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could be initialised."
                , expVerdict, Test.Framework.TestVerdict.Pass);

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
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
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "Creation of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            participant.GetQos(ref pqosHolder2);
            if (pqosHolder.UserData.Value.Length != pqosHolder2.UserData.Value.Length)
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
            returnCode = participant.SetQos(pqosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "set_qos on DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant.GetQos(ref pqosHolder2);
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
            if (pqosHolder.WatchdogScheduling.SchedulingPriorityKind.Kind != pqosHolder2.WatchdogScheduling.SchedulingPriorityKind.Kind)
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
            if (pqosHolder.ListenerScheduling.SchedulingPriorityKind.Kind != pqosHolder2.ListenerScheduling.SchedulingPriorityKind.Kind)
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
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deletion of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict);
            return result;
        }
    }
}
