namespace saj
{
    /// <date>May 23, 2005</date>
    public class testA : Test.Framework.TestCase
    {
        public testA()
            : base("sacs_defaultConstructor_tc1", "sacs_defaultConstructor", "testA"
                , "Test if struct A is initialized properly.", "Test if struct A is initialized properly."
                , null)
        {
        }

        public static string TestInit(testDefConstr.A sample)
        {
            if (sample.id != 0)
            {
                return "Expected: id == 0; Received: " + sample.id;
            }
            if (sample.Name == null)
            {
                return "name == null";
            }
            if (!sample.Name.Equals(string.Empty))
            {
                return "Expected: name == \"\"; Received: \"" + sample.Name + "\"";
            }
            if (sample.embedded == null)
            {
                return "embedded == null";
            }
            if (sample.embedded.x != 0)
            {
                return "Expected: embedded.x == 0; Received: " + sample.embedded.x;
            }
            if (sample.embedded.y != 0)
            {
                return "Expected: embedded.y == 0; Received: " + sample.embedded.y;
            }
            if (sample.embedded.z != 0)
            {
                return "Expected: embedded.z == 0; Received: " + sample.embedded.z;
            }
            if (sample.c != testDefConstr.Color.Red)
            {
                return "Expected: c == 0; Received: " + sample.c.Value();
            }
            if (sample.nums == null)
            {
                return "nums == null";
            }
            if (sample.nums.Length != testDefConstr.DIM_NUMS_1.Value)
            {
                return "Expected nums.length == " + testDefConstr.DIM_NUMS_1.Value + "; Received: "
                     + sample.nums.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_NUMS_1.Value; i++)
            {
                if (sample.nums[i].Length != testDefConstr.DIM_NUMS_2.Value)
                {
                    return "Expected nums[" + i + "].length == " + testDefConstr.DIM_NUMS_2.Value + "; Received: "
                         + sample.nums[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_NUMS_2.Value; j++)
                {
                    if (sample.nums[i][j] != 0)
                    {
                        return "Expected sample.nums[" + i + "][" + j + "] == 0" + "; Received: " + sample
                            .nums[i][j];
                    }
                }
            }
            if (sample.nums2 == null)
            {
                return "sample.nums2 == null";
            }
            if (sample.nums2.Length != 0)
            {
                return "Expected nums2.length == 0; Received: " + sample.nums2.Length;
            }
            if (sample.floatList == null)
            {
                return "floatList == null";
            }
            if (sample.floatList.Length != 0)
            {
                return "Expected floatList.length == 0; Received: " + sample.floatList.Length;
            }
            if (sample.la2Dim == null)
            {
                return "la2Dim == null";
            }
            if (sample.la2Dim.Length != testDefConstr.DIM_LA2DIM_1.Value)
            {
                return "Expected la2Dim.length == " + testDefConstr.DIM_LA2DIM_1.Value + "; Received: "
                     + sample.la2Dim.Length;
            }
            for (int i = 0; i < testDefConstr.DIM_LA2DIM_1.Value; i++)
            {
                if (sample.la2Dim[i].Length != testDefConstr.DIM_LONGARRAY10_1.Value)
                {
                    return "Expected la2Dim[" + i + "].length == " + testDefConstr.DIM_LONGARRAY10_1.
                        Value + "; Received: " + sample.la2Dim[i].Length;
                }
                for (int j = 0; j < testDefConstr.DIM_LONGARRAY10_1.Value; j++)
                {
                    if (sample.la2Dim[i][j] != 0)
                    {
                        return "Expected la2Dim[" + i + "][" + j + "] == 0" + "; Received: " + sample.la2Dim
                            [i][j];
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
			string expResult = "uninitialized struct A can be written/received without errors.";
			DDS.IDomainParticipant participant;
			testDefConstr.ATypeSupport aTS;
			string aTypeName;
			DDS.ITopic aTopic;
			DDS.IPublisher pub;
			DDS.ISubscriber sub;
			DDS.IDataWriter dw;
			testDefConstr.ADataWriter aDW;
			DDS.IDataReader dr;
			testDefConstr.ADataReader aDR;
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
			aTS = new testDefConstr.ATypeSupport();
			aTypeName = aTS.TypeName;
			status = aTS.RegisterType(participant, aTypeName);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Registering of A TypeSupport failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Topic.
			aTopic = participant.CreateTopic("structA", aTypeName, DDS.TOPIC_QOS_DEFAULT.Value
				, null, 0);
			if (aTopic == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of A Topic failed.", 
					expVerdict, Test.Framework.TestVerdict.Fail);
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
			dw = pub.CreateDataWriter(aTopic, DDS.DATAWRITER_QOS_USE_TOPIC_QOS.Value, null, 
				0);
			aDW = testDefConstr.ADataWriterHelper.Narrow(dw);
			if (aDW == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of ADataWriter failed."
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
			dr = sub.CreateDataReader(aTopic, DDS.DATAREADER_QOS_USE_TOPIC_QOS.Value, null, 
				0);
			aDR = testDefConstr.ADataReaderHelper.Narrow(dr);
			if (aDR == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of ADataReader failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Instantiate a sample of A and test if it is initialized properly.
			testDefConstr.A myA = new testDefConstr.A();
			resultMsg = TestInit(myA);
			if (resultMsg != null)
			{
				result = new Test.Framework.TestResult(expResult, "Default initialization of A not successful: "
					 + resultMsg, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Register the sample and write it into the system.
			handle = aDW.Register_instance(myA);
			if (handle == 0)
			{
				result = new Test.Framework.TestResult(expResult, "Registering an instance of type A failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			status = aDW.Write(myA, handle);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Writing an instance of type A failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Read the sample and check whether everything is initialized properly.
			testDefConstr.ASeqHolder msgSeq = new testDefConstr.ASeqHolder();
			DDS.SampleInfo[] infoSeq = new DDS.SampleInfo[]();
			status = aDR.Take(msgSeq, infoSeq, DDS.Length.Unlimited, DDS.NOT_READ_SAMPLE_STATE
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
				result = new Test.Framework.TestResult(expResult, "Received A Sample has a different state from its orginial: "
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
