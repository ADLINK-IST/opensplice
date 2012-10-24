namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Publisher1 : Test.Framework.TestCase
    {
        public Publisher1()
            : base("sacs_publisher_tc1", "sacs_publisher", "publisher", "Test if a Publisher qos can be resolved/set."
                , "Test if a Publisher qos can be resolved/set.", null)
        {
            this.AddPreItem(new test.sacs.PublisherItemInit());
            this.AddPostItem(new test.sacs.PublisherItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IPublisher publisher;
			DDS.PublisherQos qos = null;
			DDS.PublisherQos qos2 = null;
            string expResult = "PublisherQos test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            qos2 = (DDS.PublisherQos)this.ResolveObject("publisherQos");

            if (publisher.GetQos(ref qos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Get PublisherQos failed.";
                return result;
            }
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal.";
                return result;
            }
            qos.Partition.Name = new string[3];
            qos.Partition.Name[0] = "aap";
            qos.Partition.Name[1] = "noot";
            qos.Partition.Name[2] = "mies";
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set PublisherQos failed (2).";
                return result;
            }

            publisher.GetQos(ref qos2);
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal (2).";
                return result;
            }
            qos.GroupData.Value = new byte[2];
            qos.GroupData.Value[0] = 1;
            qos.GroupData.Value[1] = 2;
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set PublisherQos failed (3).";
                return result;
            }

            publisher.GetQos(ref qos2);
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal (3).";
                return result;
            }
            qos.Presentation.AccessScope = DDS.PresentationQosPolicyAccessScopeKind.InstancePresentationQos;
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set PublisherQos failed (4).";
                System.Console.Out.WriteLine("return code = " + rc);
                return result;
            }

            publisher.GetQos(ref qos2);
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal (4).";
                return result;
            }
            qos.Presentation.CoherentAccess = false;
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set PublisherQos failed (5).";
                return result;
            }
            publisher.GetQos(ref qos2);
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal (5).";
                return result;
            }
            qos.Presentation.OrderedAccess = false;
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set PublisherQos failed (6).";
                return result;
            }

            publisher.GetQos(ref qos2);
            if (!test.sacs.QosComparer.PublisherQosEquals(qos, qos2))
            {
                result.Result = "PublisherQosses not equal (6).";
                return result;
            }
            qos.Presentation.AccessScope = DDS.PresentationQosPolicyAccessScopeKind.TopicPresentationQos;
            rc = publisher.SetQos(qos);
            if (rc != DDS.ReturnCode.ImmutablePolicy)
            {
                result.Result = "Unexpected returncode " + rc + " when setting immutable QoS policy (7).";
                return result;
            }
            qos2.EntityFactory.AutoenableCreatedEntities = false;
            rc = publisher.SetQos(qos2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Unexpected returncode " + rc + " when setting invalid QoS policy (8).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
