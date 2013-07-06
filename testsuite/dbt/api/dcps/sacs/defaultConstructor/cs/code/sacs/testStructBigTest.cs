namespace saj
{
    /// <date>May 23, 2005</date>
    public class testStructBigTest : Test.Framework.TestCase
    {
        public testStructBigTest()
            : base("sacs_defaultConstructor_tc3", "sacs_defaultConstructor"
                , "testStructBigTest", "Test if struct StructBigTest is initialized properly.",
                "Test if struct StructBigTest is initialized properly.", null)
        {
        }

        public static string TestInit(testDefConstr.StructBigTest sample)
        {
            if (sample.id != 0)
            {
                return "Expected: id == 0; Received: " + sample.id;
            }
            if (sample.someLongArr == null)
            {
                return "someLongArr == null";
            }
            if (sample.someLongArr.Length != testDefConstr.DIM_LONG1ARR_1.Value)
            {
                return "Expected someLongArr.length == " + testDefConstr.DIM_LONG1ARR_1.Value + "; Received: "
                     + sample.someLongArr.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_LONG1ARR_1.Value; i++)
            {
                if (sample.someLongArr[i] != 0)
                {
                    return "Expected someLongArr[" + i + "] == 0; Received: " + sample.someLongArr[i];
                }
            }
            if (sample.someLong != 0)
            {
                return "Expected: someLong == 0; Received: " + sample.someLong;
            }
            if (sample.val == null)
            {
                return "val == null";
            }
            if (!sample.val.Equals(string.Empty))
            {
                return "Expected: val == \"\"; Received: \"" + sample.val + "\"";
            }
            if (sample.SomeOctet != 0)
            {
                return "Expected: SomeOctet == 0; Received: " + sample.SomeOctet;
            }
            if (sample.mySize.Value() != 0)
            {
                return "Expected mySize.Value() == 0; Received: " + sample.mySize.Value();
            }
            if (sample.someSeq == null)
            {
                return "someSeq == null";
            }
            if (sample.someSeq.Length != testDefConstr.DIM_AAPSEQARR12_1.Value)
            {
                return "Expected someSeq.length == " + testDefConstr.DIM_AAPSEQARR12_1.Value + "; Received: "
                     + sample.someSeq.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_AAPSEQARR12_1.Value; i++)
            {
                if (sample.someSeq[i] == null)
                {
                    return "someSeq[" + i + "] == null";
                }
                if (sample.someSeq[i].Length != 0)
                {
                    return "Expected someSeq[" + i + "].length == 0; Received: " + sample.someSeq[i].
                        Length;
                }
            }
            if (sample.someUnion == null)
            {
                return "someUnion == null";
            }
            if (sample.someUnion.Discriminator() != 0)
            {
                return "Expected: someUnion.discriminator() == 0; Received: " + sample.someUnion.
                    Discriminator();
            }
            if (sample.someUnion.Case1() != '\0')
            {
                return "someUnion.case1() != '\\0'";
            }
            if (sample.someSeq2 == null)
            {
                return "someSeq2 == null";
            }
            if (sample.someSeq2.Length != 0)
            {
                return "Expected someSeq2.length == 0; Received: " + sample.someSeq2.Length;
            }
            if (sample.mySize2 == null)
            {
                return "mySize2 == null";
            }
            if (sample.mySize2.Length != 0)
            {
                return "Expected mySize2.length == 0; Received: " + sample.mySize2.Length;
            }
            if (sample.someUnion3 == null)
            {
                return "someUnion3 == null";
            }
            if (sample.someUnion3.Length != testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_1.Value)
            {
                return "Expected someUnion3.length == " + testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_1
                    .Value + "; Received: " + sample.someUnion3.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_1.Value; i++)
            {
                if (sample.someUnion3[i] == null)
                {
                    return "someUnion3[" + i + "] == null";
                }
                if (sample.someUnion3[i].Length != testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_2.Value)
                {
                    return "Expected someUnion3[" + i + "].length == " + testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_2
                        .Value + "; Received: " + sample.someUnion3[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_BIGUNIONSEQSEQARR1ARR2_2.Value; j++)
                {
                    if (sample.someUnion3[i][j] == null)
                    {
                        return "someUnion3[" + i + "][" + j + "] == null";
                    }
                    if (sample.someUnion3[i][j].Length != 0)
                    {
                        return "Expected someUnion3[" + i + "][" + j + "].length == 0" + "; Received: " +
                             sample.someUnion3[i][j].Length;
                    }
                }
            }
            if (sample.arrTest == null)
            {
                return "arrTest == null";
            }
            if (sample.arrTest.Length != testDefConstr.DIM_LONG1ARR43ARR2ARR_1.Value)
            {
                return "Expected someUnion3.length == " + testDefConstr.DIM_LONG1ARR43ARR2ARR_1.Value
                     + "; Received: " + sample.arrTest.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_LONG1ARR43ARR2ARR_1.Value; i++)
            {
                if (sample.arrTest[i] != 0)
                {
                    return "Expected: arrTest[" + i + "] == 0; Received: " + sample.arrTest[i];
                }
            }
            if (sample.erik == null)
            {
                return "erik == null";
            }
            if (sample.erik.Length != testDefConstr.DIM_ERIK_1.Value)
            {
                return "Expected erik.length == " + testDefConstr.DIM_ERIK_1.Value + "; Received: "
                     + sample.erik.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_ERIK_1.Value; i++)
            {
                if (sample.erik[i] == null)
                {
                    return "erik[" + i + "] == null";
                }
                if (sample.erik[i].Length != testDefConstr.DIM_ERIK_2.Value)
                {
                    return "Expected erik[" + i + "].length == " + testDefConstr.DIM_ERIK_2.Value + "; Received: "
                         + sample.erik[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_ERIK_2.Value; j++)
                {
                    if (sample.erik[i][j] == null)
                    {
                        return "erik[" + i + "][" + j + "] == null";
                    }
                    if (sample.erik[i][j].Length != testDefConstr.DIM_ARRAYA_1.Value)
                    {
                        return "Expected erik[" + i + "][" + j + "].length == " + testDefConstr.DIM_ARRAYA_1
                            .Value + "; Received: " + sample.erik[i][j].Length;
                    }
                    for (int k = 0; k < testDefConstr.DIM_ARRAYA_1.Value; k++)
                    {
                        if (sample.erik[i][j][k] == null)
                        {
                            return "erik[" + i + "][" + j + "][" + k + "] == null";
                        }
                        string message = saj.testA.TestInit(sample.erik[i][j][k]);
                        if (message != null)
                        {
                            return "In message erik[" + i + "][" + j + "][" + k + "]: " + message;
                        }
                    }
                }
            }
            return null;
        }

        public override Test.Framework.TestResult Run()
		{
			Test.Framework.TestResult result;
			int status;
			Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
			string expResult = "uninitialized struct StructBigTest can be written/received without errors.";
			DDS.IDomainParticipant participant;
			testDefConstr.StructBigTestTypeSupport sbtTS;
			string sbtTypeName;
			DDS.ITopic sbtTopic;
			DDS.IPublisher pub;
			DDS.ISubscriber sub;
			DDS.IDataWriter dw;
			testDefConstr.StructBigTestDataWriter sbtDW;
			DDS.IDataReader dr;
			testDefConstr.StructBigTestDataReader sbtDR;
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
			sbtTS = new testDefConstr.StructBigTestTypeSupport();
			sbtTypeName = sbtTS.TypeName;
			status = sbtTS.RegisterType(participant, sbtTypeName);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Registering of StructBigTest TypeSupport failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Topic.
			sbtTopic = participant.CreateTopic("StructBigTest", sbtTypeName, DDS.TOPIC_QOS_DEFAULT
				.Value, null, 0);
			if (sbtTopic == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of StructBigTest Topic failed."
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
			dw = pub.CreateDataWriter(sbtTopic, DDS.DATAWRITER_QOS_USE_TOPIC_QOS.Value, null
				, 0);
			sbtDW = testDefConstr.StructBigTestDataWriterHelper.Narrow(dw);
			if (sbtDW == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of StructBigTestDataWriter failed."
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
			dr = sub.CreateDataReader(sbtTopic, DDS.DATAREADER_QOS_USE_TOPIC_QOS.Value, null
				, 0);
			sbtDR = testDefConstr.StructBigTestDataReaderHelper.Narrow(dr);
			if (sbtDR == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of TestUnionStartsDataReader failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Instantiate a sample of A and test if it is initialized properly.
			testDefConstr.StructBigTest sbt = new testDefConstr.StructBigTest();
			resultMsg = TestInit(sbt);
			if (resultMsg != null)
			{
				result = new Test.Framework.TestResult(expResult, "Default initialization of StructBigTest not successful: "
					 + resultMsg, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Register the sample and write it into the system.
			handle = sbtDW.Register_instance(sbt);
			if (handle == 0)
			{
				result = new Test.Framework.TestResult(expResult, "Registering an instance of type StructBigTest failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			status = sbtDW.Write(sbt, handle);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Writing an instance of type StructBigTest failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Read the sample and check whether everything is initialized properly.
			testDefConstr.StructBigTestSeqHolder msgSeq = new testDefConstr.StructBigTestSeqHolder
				();
			DDS.SampleInfo[] infoSeq = new DDS.SampleInfo[]();
			status = sbtDR.Take(msgSeq, infoSeq, DDS.Length.Unlimited, DDS.NOT_READ_SAMPLE_STATE
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
				result = new Test.Framework.TestResult(expResult, "Received StructBigTest Sample has a different state from its orginial: "
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
