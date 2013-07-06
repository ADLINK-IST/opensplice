namespace test.sacs
{
    /// <summary>Test if a DataReader qos can be resolved/set.</summary>
    /// <remarks>Test if a DataReader qos can be resolved/set.</remarks>
    public class Subscriber2 : Test.Framework.TestCase
    {
        /// <summary>Test if a DataReader qos can be resolved/set.</summary>
        /// <remarks>Test if a DataReader qos can be resolved/set.</remarks>
        public Subscriber2()
            : base("sacs_subscriber_tc2", "sacs_subscriber", "subscriber",
                "Test if a DataReader qos can be resolved/set.", "Test if a DataReader qos can be resolved/set."
                , null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.ISubscriber subscriber;
            DDS.ISubscriber subscriber2;
			DDS.SubscriberQos subscriberQos = null;
			DDS.DataReaderQos qos = null;
            string expResult = "DataReader qos tests succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            subscriberQos = (DDS.SubscriberQos)this.ResolveObject("subscriberQos");
            if (subscriber.GetDefaultDataReaderQos(ref qos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos (1).";
                return result;
            }
            if (!test.sacs.QosComparer.DataReaderQosEquals(qos, test.sacs.QosComparer.defaultDataReaderQos
                ))
            {
                result.Result = "DataReaderQos not equal to default DataReaderQos (2).";
                return result;
            }

            // TODO: JLS, do we really care about this test?
            //qos.Deadline.Period.Sec = DDS.Duration.InfiniteSec + 1;
            //rc = subscriber.SetDefaultDataReaderQos(ref qos);
            //if (rc != DDS.ReturnCode.BadParameter)
            //{
            //    result.Result = "Received RETCODE " + rc + " after setting invalid qos (3).";
            //    return result;
            //}
            qos = test.sacs.QosComparer.defaultDataReaderQos;
            subscriber2 = participant.CreateSubscriber(subscriberQos);//, null, 0);
            if (subscriber2 == null)
            {
                result.Result = "Could not create a subscriber (4).";
                return result;
            }
            participant2 = subscriber2.Participant;
            if (participant == null)
            {
                result.Result = "Could not get the participant (4).";
                return result;
            }
            if (participant2 != participant)
            {
                result.Result = "participant2 != participant (4).";
                return result;
            }
            rc = participant.DeleteSubscriber(subscriber2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Received RETCODE " + rc + " after deleting a subscriber (5).";
                return result;
            }
            rc = subscriber2.SetDefaultDataReaderQos(qos);
            if (rc != DDS.ReturnCode.AlreadyDeleted && rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "Received RETCODE " + rc + " after setting a qos on a already deleted Subscriber(5).";
                return result;
            }
            participant2 = subscriber2.Participant;
            if (participant2 != null)
            {
                result.Result = "Could get the participant of a deleted subscriber (6).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
