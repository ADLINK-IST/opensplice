namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Subscriber1 : Test.Framework.TestCase
    {
        public Subscriber1()
            : base("sacs_subscriber_tc1", "sacs_subscriber", "subscriber",
                "Test if a Subscriber qos can be resolved/set.", "Test if a Subscriber qos can be resolved/set."
                , null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
            DDS.SubscriberQos qos;
            DDS.SubscriberQos qos2;
            string expResult = "SubscriberQos test succeeded";
            DDS.SubscriberQos subQosHolder;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            qos = (DDS.SubscriberQos)this.ResolveObject("subscriberQos");
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, test.sacs.QosComparer.
                defaultSubscriberQos))
            {
                result.Result = "SubscriberQos not equal to default SubscriberQos (1).";
                return result;
            }
            subQosHolder = new DDS.SubscriberQos();
            qos.Partition.Name = new string[3];
            qos.Partition.Name[0] = "aap";
            qos.Partition.Name[1] = "noot";
            qos.Partition.Name[2] = "mies";
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set SubsriberQos failed (2).";
                return result;
            }
            subscriber.GetQos(ref subQosHolder);
            qos2 = subQosHolder;
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, qos2))
            {
                result.Result = "SubscriberQosses not equal (2).";
                return result;
            }
            qos.GroupData.Value = new byte[2];
            qos.GroupData.Value[0] = 1;
            qos.GroupData.Value[1] = 2;
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set SubsriberQos failed (3).";
                return result;
            }
            subscriber.GetQos(ref subQosHolder);
            qos2 = subQosHolder;
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, qos2))
            {
                result.Result = "SubscriberQosses not equal (3).";
                return result;
            }
            qos.Presentation.AccessScope = DDS.PresentationQosPolicyAccessScopeKind.InstancePresentationQos;
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set SubsriberQos failed (4).";
                return result;
            }
            subscriber.GetQos(ref subQosHolder);
            qos2 = subQosHolder;
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, qos2))
            {
                result.Result = "SubscriberQosses not equal (4).";
                return result;
            }
            qos.Presentation.CoherentAccess = false;
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set SubsriberQos failed (5).";
                return result;
            }
            subscriber.GetQos(ref subQosHolder);
            qos2 = subQosHolder;
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, qos2))
            {
                result.Result = "SubscriberQosses not equal (5).";
                return result;
            }
            qos.Presentation.OrderedAccess = false;
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set SubsriberQos failed (6).";
                return result;
            }
            subscriber.GetQos(ref subQosHolder);
            qos2 = subQosHolder;
            if (!test.sacs.SubscriberQosComparer.SubscriberQosEqual(qos, qos2))
            {
                result.Result = "SubscriberQosses not equal (6).";
                return result;
            }
            qos = (DDS.SubscriberQos)this.ResolveObject("subscriberQos");
            qos.EntityFactory.AutoenableCreatedEntities = false;
            rc = subscriber.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Received returncode " + rc + " after setting an unsupported Qos (7).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
