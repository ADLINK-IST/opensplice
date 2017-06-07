namespace test.sacs
{
    /// <date>Sep 5, 2009</date>
    public class Builtin2 : Test.Framework.TestCase
    {
        public Builtin2()
            : base("sacs_builtin_tc2", "sacs_builtin", "sacs_builtin", "test builtin subscriber"
                , "test builtin subscriber", null)
        {
            this.AddPreItem(new test.sacs.BuiltinInit());
            this.AddPostItem(new test.sacs.BuiltinDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ITopicDescription topic;
            DDS.ITopicDescription topic2;
            DDS.IDataReader reader;
            DDS.ISubscriber subscriber;
            Test.Framework.TestResult result;
            string topicName;
            string expResult = "Builtin subscriber test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");

            subscriber = participant.BuiltInSubscriber;
            if (subscriber == null)
            {
                result.Result = "Builtin subscriber could not be resolved.";
                return result;
            }

            /************************************************
             * DCPSTopic
             ************************************************/
            topicName = "DCPSTopic";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                return result;
            }
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ITopicBuiltinTopicDataDataReader dcpsTopReader = reader as DDS.ITopicBuiltinTopicDataDataReader;
                DDS.TopicBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsTopReader.Read(
                        ref dataSeq, 
                        ref infoSeq, 
                        DDS.Length.Unlimited, 
                        DDS.SampleStateKind.NotRead, 
                        DDS.ViewStateKind.Any, 
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    for (int i = 0; i < dataSeq.Length; i++)
                    {
                        System.Console.WriteLine("Found: topic " + dataSeq[i].Name); 
                    }
                    retcode = dcpsTopReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * DCPSParticipant
             ************************************************/
            topicName = "DCPSParticipant";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.IParticipantBuiltinTopicDataDataReader dcpsDPReader = reader as DDS.IParticipantBuiltinTopicDataDataReader;
                DDS.ParticipantBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsDPReader.Read(
                        ref dataSeq, 
                        ref infoSeq, 
                        DDS.Length.Unlimited, 
                        DDS.SampleStateKind.NotRead, 
                        DDS.ViewStateKind.Any, 
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    for (int i = 0; i < dataSeq.Length; i++)
                    {
                        System.Console.WriteLine("Found: Participant for node " + dataSeq[i].Key[0]); 
                    }
                    retcode = dcpsDPReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * DCPSPublication
             ************************************************/
            topicName = "DCPSPublication";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.IPublicationBuiltinTopicDataDataReader dcpsPubReader = reader as DDS.IPublicationBuiltinTopicDataDataReader;
                DDS.PublicationBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsPubReader.Read(
                        ref dataSeq, 
                        ref infoSeq, 
                        DDS.Length.Unlimited, 
                        DDS.SampleStateKind.NotRead, 
                        DDS.ViewStateKind.Any, 
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    for (int i = 0; i < dataSeq.Length; i++)
                    {
                        System.Console.WriteLine("Found: Writer for topic " + dataSeq[i].TopicName);
                    }
                    retcode = dcpsPubReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * DCPSSubscription
             ************************************************/
            topicName = "DCPSSubscription";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ISubscriptionBuiltinTopicDataDataReader dcpsSubReader = reader as DDS.ISubscriptionBuiltinTopicDataDataReader;
                DDS.SubscriptionBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsSubReader.Read(
                        ref dataSeq, 
                        ref infoSeq, 
                        DDS.Length.Unlimited, 
                        DDS.SampleStateKind.NotRead, 
                        DDS.ViewStateKind.Any, 
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    for (int i = 0; i < dataSeq.Length; i++)
                    {
                        System.Console.WriteLine("Found: Reader for topic " + dataSeq[i].TopicName); 
                    }
                    retcode = dcpsSubReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * CMParticipant
             ************************************************/
            topicName = "CMParticipant";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ICMParticipantBuiltinTopicDataDataReader dcpsCmpReader = reader as DDS.ICMParticipantBuiltinTopicDataDataReader;
                DDS.CMParticipantBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsCmpReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsCmpReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * CMPublisher
             ************************************************/
            topicName = "CMPublisher";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ICMPublisherBuiltinTopicDataDataReader dcpsCmpReader = reader as DDS.ICMPublisherBuiltinTopicDataDataReader;
                DDS.CMPublisherBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsCmpReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsCmpReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * CMSubscriber
             ************************************************/
            topicName = "CMSubscriber";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ICMSubscriberBuiltinTopicDataDataReader dcpsCmsReader = reader as DDS.ICMSubscriberBuiltinTopicDataDataReader;
                DDS.CMSubscriberBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsCmsReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsCmsReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * CMDataWriter
             ************************************************/
            topicName = "CMDataWriter";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ICMDataWriterBuiltinTopicDataDataReader dcpsCmwReader = reader as DDS.ICMDataWriterBuiltinTopicDataDataReader;
                DDS.CMDataWriterBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsCmwReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsCmwReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * CMDataReader
             ************************************************/
            topicName = "CMDataReader";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ICMDataReaderBuiltinTopicDataDataReader dcpsCmrReader = reader as DDS.ICMDataReaderBuiltinTopicDataDataReader;
                DDS.CMDataReaderBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsCmrReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsCmrReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            /************************************************
             * DCPSType
             ************************************************/
            topicName = "DCPSType";
            System.Console.WriteLine("-- Builtin Topic: " + topicName);
            reader = subscriber.LookupDataReader(topicName);
            if (reader == null)
            {
                result.Result = "Builtin datareader for " + topicName + " could not be resolved.";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            else
            {
                DDS.ITypeBuiltinTopicDataDataReader dcpsTypeReader = reader as DDS.ITypeBuiltinTopicDataDataReader;
                DDS.TypeBuiltinTopicData[] dataSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                DDS.ReturnCode retcode = dcpsTypeReader.Read(
                        ref dataSeq,
                        ref infoSeq,
                        DDS.Length.Unlimited,
                        DDS.SampleStateKind.NotRead,
                        DDS.ViewStateKind.Any,
                        DDS.InstanceStateKind.Any);
                if (retcode == DDS.ReturnCode.Ok)
                {
                    System.Console.WriteLine("TODO: Print info ["+ dataSeq.Length +"]");
                    retcode = dcpsTypeReader.ReturnLoan(ref dataSeq, ref infoSeq);
                    if (retcode != DDS.ReturnCode.Ok)
                    {
                       result.Result = "ReturnLoan Returncode not Ok, instead ReturnCode = " + retcode;
                       return result;
                    }
                }
                else if (retcode == DDS.ReturnCode.NoData) {
                    System.Console.WriteLine("TODO: Write data");
                }
                else
                {
                    result.Result = "Read Returncode not Ok, instead ReturnCode = " + retcode;
                    return result;
                }
            }
            topic = reader.GetTopicDescription();
            if (topic == null)
            {
                result.Result = "Builtin topic for datareader of " + topicName + " could not be resolved.";
                return result;
            }
            topic2 = participant.LookupTopicDescription(topicName);
            if (topic2 == null)
            {
                result.Result = "Builtin topic " + topicName + " could not be resolved.";
                return result;
            }
            if (topic != topic2)
            {
                result.Result = "Builtin topics of " + topicName + " do not match.";
                return result;
            }

            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
