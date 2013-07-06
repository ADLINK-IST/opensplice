namespace saj
{
    /// <date>May 23, 2005</date>
    public class testTestUnionStarts : Test.Framework.TestCase
    {
        public testTestUnionStarts()
            : base("sacs_defaultConstructor_tc2", "sacs_defaultConstructor"
                , "testTestUnionStarts", "Test if struct TestUnionStarts is initialized properly."
                , "Test if struct TestUnionStarts is initialized properly.", null)
        {
        }

        public static string TestInit(testDefConstr.TestUnionStarts sample)
        {
            if (sample.id != 0)
            {
                return "Expected: id == 0; Received: " + sample.id;
            }
            if (sample.usstring == null)
            {
                return "usstring == null";
            }
            if (sample.usstring.Discriminator() != testDefConstr.Size.LARGE)
            {
                return "Expected: usstring.discriminator() == " + testDefConstr.Size.LARGE.Value(
                    ) + "; Received: " + sample.usstring.Discriminator().Value();
            }
            if (sample.usstring.Name() == null)
            {
                return "usstring.Name() == null";
            }
            if (!sample.usstring.Name().Equals(string.Empty))
            {
                return "Expected: usstring.Name() == \"\"; Received: \"" + sample.usstring.Name()
                     + "\"";
            }
            if (sample.usmds == null)
            {
                return "usmds == null";
            }
            if (sample.usmds.Discriminator() != testDefConstr.Size.MEDIUM)
            {
                return "Expected: usmds.discriminator() == " + testDefConstr.Size.MEDIUM.Value()
                    + "; Received: " + sample.usmds.Discriminator().Value();
            }
            if (sample.usmds.ThreeDimA() == null)
            {
                return "usmds.threeDimA() == null";
            }
            if (sample.usmds.ThreeDimA().Length != testDefConstr.DIM_THREEDIMA_1.Value)
            {
                return "Expected usmds.threeDimA().length == " + testDefConstr.DIM_THREEDIMA_1.Value
                     + "; Received: " + sample.usmds.ThreeDimA().Length;
            }
            for (int i = 0; i < testDefConstr.DIM_THREEDIMA_1.Value; i++)
            {
                if (sample.usmds.ThreeDimA()[i].Length != testDefConstr.DIM_THREEDIMA_2.Value)
                {
                    return "Expected usmds.threeDimA()[" + i + "].length == " + testDefConstr.DIM_THREEDIMA_2
                        .Value + "; Received: " + sample.usmds.ThreeDimA()[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_THREEDIMA_2.Value; j++)
                {
                    if (sample.usmds.ThreeDimA()[i][j].Length != testDefConstr.DIM_ARRAYA_1.Value)
                    {
                        return "Expected usmds.threeDimA()[" + i + "][" + j + "].length == " + testDefConstr.DIM_ARRAYA_1
                            .Value + "; Received: " + sample.usmds.ThreeDimA()[i][j].Length;
                    }
                    for (int k = 0; k < testDefConstr.DIM_ARRAYA_1.Value; k++)
                    {
                        if (sample.usmds.ThreeDimA()[i][j][k] == null)
                        {
                            return "usmds.threeDimA()[" + i + "][" + j + "][" + k + "] == null";
                        }
                        string message = saj.testA.TestInit(sample.usmds.ThreeDimA()[i][j][k]);
                        if (message != null)
                        {
                            return "In message usmds.threeDimA()[" + i + "][" + j + "][" + k + "]: " + message;
                        }
                    }
                }
            }
            if (sample.usmdp == null)
            {
                return "usmdp == null";
            }
            if (sample.usmdp.Discriminator() != testDefConstr.Size.MEDIUM)
            {
                return "Expected: usmdp.discriminator() == " + testDefConstr.Size.MEDIUM.Value()
                    + "; Received: " + sample.usmdp.Discriminator().Value();
            }
            if (sample.usmdp.TwoDimLong() == null)
            {
                return "usmdp.twoDimLong() == null";
            }
            if (sample.usmdp.TwoDimLong().Length != testDefConstr.DIM_TWODIMLONG_1.Value)
            {
                return "Expected usmdp.twoDimLong().length == " + testDefConstr.DIM_TWODIMLONG_1.
                    Value + "; Received: " + sample.usmdp.TwoDimLong().Length;
            }
            for (int i = 0; i < testDefConstr.DIM_TWODIMLONG_1.Value; i++)
            {
                if (sample.usmdp.TwoDimLong()[i].Length != testDefConstr.DIM_LONGARRAY10_1.Value)
                {
                    return "Expected usmdp.twoDimLong()[" + i + "].length == " + testDefConstr.DIM_LONGARRAY10_1
                        .Value + "; Received: " + sample.usmdp.TwoDimLong()[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_LONGARRAY10_1.Value; j++)
                {
                    if (sample.usmdp.TwoDimLong()[i][j] != 0)
                    {
                        return "Expected usmdp.twoDimLong()[" + i + "][" + j + "] == 0" + "; Received: "
                            + sample.usmdp.TwoDimLong()[i][j];
                    }
                }
            }
            if (sample.use == null)
            {
                return "use == null";
            }
            if (sample.use.Discriminator() != testDefConstr.Color.Red)
            {
                return "Expected: use.discriminator() == " + testDefConstr.Color.Red.Value() + "; Received: "
                     + sample.use.Discriminator().Value();
            }
            if (sample.use.MySize() != testDefConstr.Size.LARGE)
            {
                return "Expected: use.mySize() == " + testDefConstr.Size.LARGE.Value() + "; Received: "
                     + sample.use.MySize().Value();
            }
            if (sample.usstruct == null)
            {
                return "usstruct == null";
            }
            if (sample.usstruct.Discriminator() != testDefConstr.Color.Red)
            {
                return "Expected: usstruct.discriminator() == " + testDefConstr.Color.Red.Value()
                     + "; Received: " + sample.usstruct.Discriminator().Value();
            }
            if (sample.usstruct.MyB() == null)
            {
                return "usstruct.myB() == null";
            }
            if (sample.usstruct.MyB().x != 0)
            {
                return "Expected: usstruct.myB().x == 0; Received: " + sample.usstruct.MyB().x;
            }
            if (sample.usstruct.MyB().y != 0)
            {
                return "Expected: usstruct.myB().y == 0; Received: " + sample.usstruct.MyB().y;
            }
            if (sample.usstruct.MyB().z != 0)
            {
                return "Expected: usstruct.myB().z == 0; Received: " + sample.usstruct.MyB().z;
            }
            if (sample.usseq == null)
            {
                return "usseq == null";
            }
            if (sample.usseq.Discriminator() != testDefConstr.Color.Red)
            {
                return "Expected: usseq.discriminator() == " + testDefConstr.Color.Red.Value() +
                    "; Received: " + sample.usseq.Discriminator().Value();
            }
            if (sample.usseq.NoInitRequired() == null)
            {
                return "usseq.noInitRequired() == null";
            }
            if (sample.usseq.NoInitRequired().Length != 0)
            {
                return "Expected usseq.noInitRequired().length == 0; Received: " + sample.usseq.NoInitRequired
                    ().Length;
            }
            if (sample.uspd == null)
            {
                return "uspd == null";
            }
            if (sample.uspd.Discriminator() != testDefConstr.Color.Red)
            {
                return "Expected: uspd.discriminator() == " + testDefConstr.Color.Red.Value() + "; Received: "
                     + sample.uspd.Discriminator().Value();
            }
            if (sample.uspd.D() != 0)
            {
                return "Expected: uspd.d() == 0; Received: " + sample.uspd.D();
            }
            if (sample.uspc == null)
            {
                return "uspc == null";
            }
            if (sample.uspc.Discriminator() != testDefConstr.Color.Red)
            {
                return "Expected: uspc.discriminator() == " + testDefConstr.Color.Red.Value() + "; Received: "
                     + sample.uspc.Discriminator().Value();
            }
            if (sample.uspc.C() != '\0')
            {
                return "Expected: uspc.c() == '\\0'; Received: " + sample.uspc.C();
            }
            return null;
        }

        public override Test.Framework.TestResult Run()
		{
			Test.Framework.TestResult result;
			int status;
			Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
			string expResult = "uninitialized struct TestUnionStarts can be written/received without errors.";
			DDS.IDomainParticipant participant;
			testDefConstr.TestUnionStartsTypeSupport tusTS;
			string tusTypeName;
			DDS.ITopic tusTopic;
			DDS.IPublisher pub;
			DDS.ISubscriber sub;
			DDS.IDataWriter dw;
			testDefConstr.TestUnionStartsDataWriter tusDW;
			DDS.IDataReader dr;
			testDefConstr.TestUnionStartsDataReader tusDR;
			long handle;
			string resultMsg;
			// Create Participant.
			participant = DDS.TheParticipantFactory.Value.CreateParticipant(DDS.DomainId.Default, DDS.PARTICIPANT_QOS_DEFAULT
				.Value, null, 0);
			if (participant == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of DomainParticipant failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Register TypeSupport.
			tusTS = new testDefConstr.TestUnionStartsTypeSupport();
			tusTypeName = tusTS.TypeName;
			status = tusTS.RegisterType(participant, tusTypeName);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Registering of TestUnionStarts TypeSupport failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Topic.
			tusTopic = participant.CreateTopic("TestUnionStarts", tusTypeName, DDS.TOPIC_QOS_DEFAULT
				.Value, null, 0);
			if (tusTopic == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of TestUnionStarts Topic failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Publisher.
			pub = participant.CreatePublisher(DDS.PUBLISHER_QOS_DEFAULT.Value, null, 0);
			if (pub == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of Publisher failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create ADataWriter.
			dw = pub.CreateDataWriter(tusTopic, DDS.DATAWRITER_QOS_USE_TOPIC_QOS.Value, null
				, 0);
			tusDW = testDefConstr.TestUnionStartsDataWriterHelper.Narrow(dw);
			if (tusDW == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of TestUnionStartsDataWriter failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Subscriber.
			sub = participant.CreateSubscriber(DDS.SUBSCRIBER_QOS_DEFAULT.Value, null, 0);
			if (pub == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of Subscriber failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create ADataReader.
			dr = sub.CreateDataReader(tusTopic, DDS.DATAREADER_QOS_USE_TOPIC_QOS.Value, null
				, 0);
			tusDR = testDefConstr.TestUnionStartsDataReaderHelper.Narrow(dr);
			if (tusDR == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of TestUnionStartsDataReader failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Instantiate a sample of A and test if it is initialized properly.
			testDefConstr.TestUnionStarts tus = new testDefConstr.TestUnionStarts();
			resultMsg = TestInit(tus);
			if (resultMsg != null)
			{
				result = new Test.Framework.TestResult(expResult, "Default initialization of TestUnionStarts not successful: "
					 + resultMsg, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Register the sample and write it into the system.
			handle = tusDW.Register_instance(tus);
			if (handle == 0)
			{
				result = new Test.Framework.TestResult(expResult, "Registering an instance of type TestUnionStarts failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			status = tusDW.Write(tus, handle);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Writing an instance of type TestUnionStarts failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Read the sample and check whether everything is initialized properly.
			testDefConstr.TestUnionStartsSeqHolder msgSeq = new testDefConstr.TestUnionStartsSeqHolder
				();
			DDS.SampleInfo[] infoSeq = new DDS.SampleInfo[]();
			status = tusDR.Take(msgSeq, infoSeq, DDS.Length.Unlimited, DDS.NOT_READ_SAMPLE_STATE
				.Value, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
			if (msgSeq.Value.Length != 1)
			{
				result = new Test.Framework.TestResult(expResult, "Wrong number of samples received."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			resultMsg = TestInit(msgSeq.Value[0]);
			if (resultMsg != null)
			{
				result = new Test.Framework.TestResult(expResult, "Received TestUnionStarts Sample has a different state from its orginial: "
					 + resultMsg, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Delete all entities.
			status = participant.DeleteContainedEntities();
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Deleting contained entities failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			status = DDS.TheParticipantFactory.Value.DeleteParticipant(participant);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Deleting participant failed.", 
					expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
				);
			return result;
		}
    }
}
