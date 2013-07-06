namespace test.sacs
{
    /// <summary>Tests the ability to attach and to lookup conditions.</summary>
    /// <remarks>Tests the ability to attach and to lookup conditions.</remarks>
    public class Waitset2 : Test.Framework.TestCase
    {
        public Waitset2()
            : base("sacs_waitset_tc2", "waitset", "waitset", "test the attach_condition and the get_conditions functions"
                , "conditions", null)
        {
            AddPreItem(new test.sacs.CreateConditionItems());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Waitset StatusCondition succeeded.";
            DDS.ICondition[] holder;
            DDS.ReturnCode rc;
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos qosHolder = null;
            DDS.IStatusCondition condition;
            DDS.IStatusCondition condition2;
            DDS.IPublisher publisher;
			DDS.PublisherQos pubQosHolder = null;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            holder = new DDS.ICondition[0];
            DDS.WaitSet waitset = new DDS.WaitSet();
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "Factory get_instance failed.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref qosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_participant_qos failed.";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, qosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "create_participant failed.";
                return result;
            }

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_publisher_qos failed.";
                return result;
            }
            publisher = participant.CreatePublisher(pubQosHolder);//, null, DDS.StatusKind.Any);
            if (publisher == null)
            {
                result.Result = "create_publisher failed.";
                return result;
            }
            condition = participant.StatusCondition;
            if (condition == null)
            {
                result.Result = "get_status_condition failed.";
                return result;
            }
            condition2 = publisher.StatusCondition;
            if (condition2 == null)
            {
                result.Result = "get_status_condition failed.(2)";
                return result;
            }
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "attach_condition returned RETCODE: " + rc;
                return result;
            }
            rc = waitset.GetConditions(ref holder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions returned RETCODE: " + rc;
                return result;
            }
            if (holder.Length != 1)
            {
                result.Result = "get_conditions returned " + holder.Length + " conditions.";
                return result;
            }
            if (holder[0] != condition)
            {
                result.Result = "get_conditions returned wrong condition.";
                return result;
            }
            rc = waitset.DetachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "detach_condition returned RETCODE: " + rc + " (1)";
                return result;
            }
            rc = waitset.GetConditions(ref holder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions returned RETCODE: " + rc + " (1)";
                return result;
            }
            if (holder.Length != 0)
            {
                result.Result = "get_conditions returned " + holder.Length + " conditions (1).";
                return result;
            }
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "attach_condition returned RETCODE: " + rc;
                return result;
            }
            rc = waitset.AttachCondition(condition2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "attach_condition returned RETCODE: " + rc;
                return result;
            }
            rc = waitset.GetConditions(ref holder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "get_conditions returned RETCODE: " + rc;
                return result;
            }
            if (holder.Length != 2)
            {
                result.Result = "get_conditions returned " + holder.Length + " conditions.";
                return result;
            }
            if ((holder[0] != condition) && (holder[1] != condition))
            {
                result.Result = "get_conditions returned wrong conditions(1).";
                return result;
            }
            if ((holder[0] != condition2) && (holder[1] != condition2))
            {
                result.Result = "get_conditions returned wrong conditions(2).";
                return result;
            }
            rc = waitset.DetachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "detach_condition returned RETCODE: " + rc + " (2)";
                return result;
            }
            rc = waitset.DetachCondition(condition2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "detach_condition returned RETCODE: " + rc + " (3)";
                return result;
            }
            rc = participant.DeleteContainedEntities();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_contained_entities returned RETCODE: " + rc;
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "delete_participant returned RETCODE: " + rc;
                return result;
            }
            result.Result = expResult;
            result.Verdict = expVerdict;
            return result;
        }
    }
}
