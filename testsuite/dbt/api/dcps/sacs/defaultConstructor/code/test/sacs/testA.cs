namespace saj
{
    /// <date>May 23, 2005</date>
    public class testA : Test.Framework.TestCase
    {
        public testA()
            : base("sacs_defaultConstructor_tc1", "sacs_defaultConstructor"
                , "testA"
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
            if (sample.name == null)
            {
                return "name == null";
            }
            if (!sample.name.Equals(string.Empty))
            {
                return "Expected: name == \"\"; Received: \"" + sample.name + "\"";
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
                return "Expected: c == 0; Received: " + sample.c;
            }
            if (sample.nums == null)
            {
                return "nums == null";
            }
            if (sample.nums.GetLength(0) != testDefConstr.DIM_NUMS_1.value)
            {
                return "Expected nums.length (1st dimension) == " + testDefConstr.DIM_NUMS_1.value + "; Received: "
                     + sample.nums.GetLength(0);
            }
            if (sample.nums.GetLength(1) != testDefConstr.DIM_NUMS_2.value)
            {
                return "Expected nums.length (2nd dimension) == " + testDefConstr.DIM_NUMS_2.value + "; Received: "
                     + sample.nums.GetLength(1);
            }
            for (int i = 0; i < testDefConstr.DIM_NUMS_1.value; i++)
            {
                for (int j = 0; j < testDefConstr.DIM_NUMS_2.value; j++)
                {
                    if (sample.nums[i, j] != 0)
                    {
                        return "Expected sample.nums[" + i + ", " + j + "] == 0" + "; Received: " + sample
                            .nums[i, j];
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
            if (sample.stringList == null)
            {
                return "stringList == null";
            }
            if (sample.stringList.Length != 0)
            {
                return "Expected stringList.length == 0; Received: " + sample.stringList.Length;
            }
            if (sample.la2Dim == null)
            {
                return "la2Dim == null";
            }
            if (sample.la2Dim.GetLength(0) != testDefConstr.DIM_LA2DIM_1.value)
            {
                return "Expected la2Dim.length (1st dimension) == " + testDefConstr.DIM_LA2DIM_1.value + "; Received: "
                     + sample.la2Dim.GetLength(0);
            }
            if (sample.la2Dim.GetLength(1) != testDefConstr.DIM_LONGARRAY10_1.value)
            {
                return "Expected la2Dim.length (2nd dimension) == " + testDefConstr.DIM_LONGARRAY10_1.
                    value + "; Received: " + sample.la2Dim.GetLength(1);
            }
            for (int i = 0; i < testDefConstr.DIM_LA2DIM_1.value; i++)
            {
                for (int j = 0; j < testDefConstr.DIM_LONGARRAY10_1.value; j++)
                {
                    if (sample.la2Dim[i, j] != 0)
                    {
                        return "Expected la2Dim[" + i + ", " + j + "] == 0" + "; Received: " + sample.la2Dim
                            [i, j];
                    }
                }
            }
            return null;
        }

        public static string compareSamples(testDefConstr.A sample1, testDefConstr.A sample2)
        {
            if (sample1.id != sample2.id)
            {
                return "Expected: id == " + sample1.id + "; Received: " + sample2.id;
            }
            if ((sample1.name != null && sample2.name != null && !sample1.name.Equals(sample2.name)) || (sample1.name != sample2.name && (sample1.name == null || sample2.name == null)))
            {
                return "Expected: name == " + sample1.name + "; Received: \"" + sample2.name + "\"";
            }
            if (sample1.embedded == null || sample2.embedded == null)
            {
                return "embedded == null";
            }
            if (sample1.embedded.x != sample2.embedded.x)
            {
                return "Expected: embedded.x == " + sample1.embedded.x + "; Received: " + sample2.embedded.x;
            }
            if (sample1.embedded.y != sample2.embedded.y)
            {
                return "Expected: embedded.y == " + sample1.embedded.y + "; Received: " + sample2.embedded.y;
            }
            if (sample1.embedded.z != sample2.embedded.z)
            {
                return "Expected: embedded.z == " + sample1.embedded.z + "; Received: " + sample2.embedded.z;
            }
            if (sample1.c != sample2.c)
            {
                return "Expected: c == " + sample1.c + "; Received: " + sample2.c;
            }
            if (sample1.nums == null || sample2.nums == null)
            {
                return "nums == null";
            }
            if (sample1.nums.GetLength(0) != sample2.nums.GetLength(0))
            {
                return "Expected nums.length (1st dimension) == " + sample1.nums.GetLength(0) + "; Received: "
                     + sample2.nums.GetLength(0);
            }
            if (sample1.nums.GetLength(1) != sample2.nums.GetLength(1))
            {
                return "Expected nums.length (2nd dimension) == " + sample1.nums.GetLength(1) + "; Received: "
                     + sample2.nums.GetLength(1);
            }
            for (int i = 0; i < sample1.nums.GetLength(0); i++)
            {
                for (int j = 0; j < sample1.nums.GetLength(1); j++)
                {
                    if (sample1.nums[i, j] != sample2.nums[i, j])
                    {
                        return "Expected sample1.nums[" + i + ", " + j + "] == " + sample1.nums[i, j] + "; Received: " + sample2.nums[i, j];
                    }
                }
            }
            if (sample1.nums2 == null || sample2.nums2 == null)
            {
                return "sample.nums2 == null";
            }
            if (sample1.nums2.Length != sample2.nums2.Length)
            {
                return "Expected nums2.length == " + sample1.nums2.Length + "; Received: " + sample2.nums2.Length;
            }
            for (int i = 0; i < sample1.nums2.Length; i++)
            {
                if (sample1.nums2[i] == null || sample2.nums2[i] == null)
                {
                    return "sample.nums2[ " + i + "] == null";
                }
                if (sample1.nums2[i].Length != sample2.nums2[i].Length)
                {
                    return "Expected nums2.length == " + sample1.nums2[i].Length + "; Received: " + sample2.nums2[i].Length;
                }
                for (int j = 0; j < sample1.nums2[i].Length; j++)
                {
                    if (sample1.nums2[i][j] != sample2.nums2[i][j])
                    {
                        return "Expected nums2[" + i + "][" + j + "] == " + sample1.nums2[i][j] + "; Received: " + sample2.nums2[i][j];
                    }
                }
            }
            if (sample1.floatList == null || sample2.floatList == null)
            {
                return "floatList == null";
            }
            if (sample1.floatList.Length != sample2.floatList.Length)
            {
                return "Expected floatList.length == " + sample1.floatList.Length + "; Received: " + sample2.floatList.Length;
            }
            for (int i = 0; i < sample1.floatList.Length; i++)
            {
                if (sample1.floatList[i] == null || sample2.floatList[i] == null)
                {
                    return "sample.floatList[ " + i + "] == null";
                }
                if (sample1.floatList[i].Length != sample2.floatList[i].Length)
                {
                    return "Expected floatList.length == " + sample1.floatList[i].Length + "; Received: " + sample2.floatList[i].Length;
                }
                for (int j = 0; j < sample1.floatList[i].Length; j++)
                {
                    if (sample1.floatList[i][j] != sample2.floatList[i][j])
                    {
                        return "Expected floatList[" + i + "][" + j + "] == " + sample1.floatList[i][j] + "; Received: " + sample2.floatList[i][j];
                    }
                }
            }
            if (sample1.stringList == null || sample2.stringList == null)
            {
                return "stringList == null";
            }
            if (sample1.stringList.Length != sample2.stringList.Length)
            {
                return "Expected stringList.length == " + sample1.stringList.Length + "; Received: " + sample2.stringList.Length;
            }
            for (int i = 0; i < sample1.stringList.Length; i++)
            {
                if (sample1.stringList[i] == null || sample2.stringList[i] == null)
                {
                    return "sample.stringList[ " + i + "] == null";
                }
                if (sample1.stringList[i].Length != sample2.stringList[i].Length)
                {
                    return "Expected stringList.length == " + sample1.stringList[i].Length + "; Received: " + sample2.stringList[i].Length;
                }
                for (int j = 0; j < sample1.stringList[i].Length; j++)
                {
                    if (sample1.stringList[i][j] != sample2.stringList[i][j])
                    {
                        return "Expected stringList[" + i + "][" + j + "] == " + sample1.stringList[i][j] + "; Received: " + sample2.stringList[i][j];
                    }
                }
            }
            if (sample1.la2Dim == null || sample2.la2Dim == null)
            {
                return "la2Dim == null";
            }
            if (sample1.la2Dim.GetLength(0) != sample2.la2Dim.GetLength(0))
            {
                return "Expected la2Dim.length (1st dimension) == " + sample1.la2Dim.GetLength(0) + "; Received: "
                     + sample2.la2Dim.GetLength(0);
            }
            if (sample1.la2Dim.GetLength(1) != sample2.la2Dim.GetLength(1))
            {
                return "Expected la2Dim.length (2nd dimension) == " + sample1.la2Dim.GetLength(1)
                    + "; Received: " + sample2.la2Dim.GetLength(1);
            }
            for (int i = 0; i < sample1.la2Dim.GetLength(0); i++)
            {
                for (int j = 0; j < sample1.la2Dim.GetLength(1); j++)
                {
                    if (sample1.la2Dim[i, j] != sample2.la2Dim[i, j])
                    {
                        return "Expected la2Dim[" + i + ", " + j + "] == " + sample1.la2Dim[i, j] + "; Received: " + sample2.la2Dim[i, j];
                    }
                }
            }
            return null;
        }

        public override Test.Framework.TestResult Run()
		{
			Test.Framework.TestResult result;
			DDS.ReturnCode status;
			Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
			string expResult = "Both uninitialized and initialized struct A can be written/received without errors.";
			DDS.IDomainParticipant participant;
			testDefConstr.ATypeSupport aTS;
			string aTypeName;
			DDS.ITopic aTopic;
			DDS.IPublisher pub;
			DDS.ISubscriber sub;
			testDefConstr.ADataWriter aDW;
			testDefConstr.ADataReader aDR;
			long handle;
			string resultMsg;
			// Create Participant.
			participant = DDS.DomainParticipantFactory.Instance.CreateParticipant(DDS.DomainId.Default);
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
			aTopic = participant.CreateTopic("structA", aTypeName);
			if (aTopic == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of A Topic failed.",
					expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Publisher.
			pub = participant.CreatePublisher();
			if (pub == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of Publisher failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create ADataWriter.
			aDW = pub.CreateDataWriter(aTopic) as testDefConstr.ADataWriter;;
			if (aDW == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of ADataWriter failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create Subscriber.
			sub = participant.CreateSubscriber();
			if (pub == null)
			{
				result = new Test.Framework.TestResult(expResult, "Creation of Subscriber failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Create ADataReader.
			aDR = sub.CreateDataReader(aTopic) as testDefConstr.ADataReader;
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
			handle = aDW.RegisterInstance(myA);
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
			testDefConstr.A[] msgSeq = null;
			DDS.SampleInfo[] infoSeq = null;
			status = aDR.Take(ref msgSeq, ref infoSeq, DDS.Length.Unlimited, DDS.SampleStateKind.NotRead,
				DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
			if (msgSeq.Length != 1)
			{
				result = new Test.Framework.TestResult(expResult, "Wrong number of samples received."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			resultMsg = TestInit(msgSeq[0]);
			if (resultMsg != null)
			{
				result = new Test.Framework.TestResult(expResult, "Received A Sample has a different state from its orginial: "
					 + resultMsg, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}

                        //Now explicitly initlaize sample to some preset values.
                        myA.id = 5;
                        myA.name = "Mary";
                        myA.embedded.x = 1;
                        myA.embedded.y = 2;
                        myA.embedded.z = 3;
                        myA.c = testDefConstr.Color.Blue;
                        for (int i = 0; i < testDefConstr.DIM_NUMS_1.value; i++)
                        {
                            for (int j = 0; j < testDefConstr.DIM_NUMS_2.value; j++)
                            {
                                myA.nums[i, j] = 10 + i + j;
                            }
                        }
                        myA.nums2 = new int[2][];
                        myA.nums2[0] = new int[1];
                        myA.nums2[1] = new int[1];
                        myA.nums2[0][0] = 8;
                        myA.nums2[1][0] = 9;

                        myA.floatList = new float[1][];
                        myA.floatList[0] = new float[2];
                        myA.floatList[0][0] = 0.1f;
                        myA.floatList[0][1] = 0.2f;

                        myA.stringList = new string[3] { "Erik", "Maurtis", "Hans"};

                        for (int i = 0; i < testDefConstr.DIM_LA2DIM_1.value; i++)
                        {
                            for (int j = 0; j < testDefConstr.DIM_LONGARRAY10_1.value; j++)
                            {
                                myA.la2Dim[i, j] = testDefConstr.DIM_LA2DIM_1.value * i + j;
                            }
                        }

			// Register the sample and write it into the system.
			handle = aDW.RegisterInstance(myA);
			if (handle == 0)
			{
				result = new Test.Framework.TestResult(expResult, "Registering an initialized instance of type A failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			status = aDW.Write(myA, handle);
			if (status != DDS.ReturnCode.Ok)
			{
				result = new Test.Framework.TestResult(expResult, "Writing an initialized instance of type A failed."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			// Read the sample and check whether everything is initialized properly.
			status = aDR.Take(ref msgSeq, ref infoSeq, DDS.Length.Unlimited, DDS.SampleStateKind.NotRead,
				DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
			if (msgSeq.Length != 1)
			{
				result = new Test.Framework.TestResult(expResult, "Wrong number of samples received."
					, expVerdict, Test.Framework.TestVerdict.Fail);
				return result;
			}
			resultMsg = compareSamples(myA, msgSeq[0]);
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
			status = DDS.DomainParticipantFactory.Instance.DeleteParticipant(participant);
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
