/*
 * Documentation see osplo/testsuite/dbt/api/sac/tc_get_discovered_participants/code/tc_get_discovered_participants.c
 *
 */


using tc_get_discovered_xxx;

namespace test.sacs

{
    public class GetDiscoveredXxx : Test.Framework.TestCase
    {
        public GetDiscoveredXxx()
             : base (  "sacs_tc_get_discovered_xxx", 
                "sacs_tc_get_discovered_xxx", 
                "getDiscoveredXxx", 
                "This test checks the get_discovered_xxx API call functionality.", 
                "This test checks the get_discovered_xxx API call functionality.", 
                null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "get_discovered_xxx succeeded.";
            DDS.DomainParticipantFactory dpf;
            DDS.IDomainParticipant dp1;
            DDS.IDomainParticipant dp2;
            DDS.IDomainParticipant dp3;
            DDS.ITopic                                  topic1;
            DDS.ITopic                                  topic2;
            DDS.TopicQos                                tQos;
            tc_get_discovered_xxx.Type1TypeSupport                type1_ts;
            bool dp1res = false;
            bool dp2res = false;
            bool dp3res = false;
            bool t1res = false;
            bool t2res = false;

            DDS.ReturnCode returnCode;
                
            DDS.ParticipantBuiltinTopicData       participant_data = null;
            DDS.TopicBuiltinTopicData             topic_data = null;
	        DDS.SampleInfo[]                      sample_infos = null;
	        DDS.DomainParticipantQos              dpQos = null;
	        DDS.InstanceHandle[]                  sequence = null;
	        DDS.InstanceHandle[]                  sequence1 = null;
            
            System.Text.ASCIIEncoding  encoding = new System.Text.ASCIIEncoding();
            tQos = new DDS.TopicQos();

            dpf = DDS.DomainParticipantFactory.Instance;
            if (dpf == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could be initialised."
                , expVerdict, Test.Framework.TestVerdict.Pass);

            if (dpf.GetDefaultParticipantQos(ref dpQos) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
 
            
            dpQos.UserData.Value = encoding.GetBytes("dp1");
            
            dp1 = dpf.CreateParticipant (DDS.DomainId.Default, dpQos, null, 0);
	        if (dp1 == null) {
	              result = new Test.Framework.TestResult(expResult, "Test case failed. Failed to create dp1 participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
	        } 
	        
	        dpQos.UserData.Value = encoding.GetBytes("dp2");
            
            dp2 = dpf.CreateParticipant (DDS.DomainId.Default, dpQos, null, 0);
            if (dp2 == null) {
                  result = new Test.Framework.TestResult(expResult, "Test case failed. Failed to create dp2 participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            } 
            
            dpQos.UserData.Value = encoding.GetBytes("dp3");
            
            dp3 = dpf.CreateParticipant (DDS.DomainId.Default, dpQos, null, 0);
            if (dp3 == null) {
                  result = new Test.Framework.TestResult(expResult, "Test case failed. Failed to create dp3 participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            } 
            
            /*
             * Get default Topic Qos settings
             */
            dp1.GetDefaultTopicQos(ref tQos);

            /* Create Topic */
            type1_ts = new tc_get_discovered_xxx.Type1TypeSupport();
            type1_ts.RegisterType(dp1, "tc_get_discovered_xxx::Type1");
            topic1 = dp1.CreateTopic("Topic1", "tc_get_discovered_xxx::Type1", tQos);
            
            if (topic1 == null) {
                  result = new Test.Framework.TestResult(expResult, "Test case failed. Failed to create Topic1"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            } 

            topic2 = dp1.CreateTopic("Topic2", "tc_get_discovered_xxx::Type1", tQos);
            
            if (topic2 == null) {
                  result = new Test.Framework.TestResult(expResult, "Test case failed. Failed to create Topic2"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            } 
            
            returnCode = dp1.GetDiscoveredParticipants(ref sequence);

	        if (returnCode != DDS.ReturnCode.Ok) {
	            result = new Test.Framework.TestResult(expResult, "get_discovered_participants returned not OK."
	                    , expVerdict, Test.Framework.TestVerdict.Fail);
	                return result;
	
	
	        } else {
	            /* the test should find 5 participants:
	             * buildinParticipant
	             * spliced
	             * dp1
	             * dp2
	             * dp3
	             */
	
	            if (sequence.Length != 5) {
	                result = new Test.Framework.TestResult(expResult, "get_discovered_participants failed found: "+sequence.Length+" expected 5"
	                    , expVerdict, Test.Framework.TestVerdict.Fail);
	                return result;
	            }
	            
	            /* read from each handle the UserDataQos and check the result against the made participant names*/
	            for (int i=0; i<sequence.Length;i++) {
	
	                returnCode = dp1.GetDiscoveredParticipantData(ref participant_data,sequence[i]);
	                
	                if (returnCode == DDS.ReturnCode.Ok) {
	                    if (participant_data.UserData.Value.Length >0) {
	                        if ( encoding.GetString(participant_data.UserData.Value).CompareTo("dp1") == 0) {
	                            dp1res = true;
	                        } else if (encoding.GetString(participant_data.UserData.Value).CompareTo("dp2") == 0) {
	                            dp2res= true;
	                        }  else if (encoding.GetString(participant_data.UserData.Value).CompareTo("dp3") == 0) {
	                            dp3res = true;
	                        }
	                    }
	                   // returnCode = dataReader.ReturnLoan(ref data_values, ref sample_infos);
	                } else {
	                     result = new Test.Framework.TestResult(
	                            expResult, "read instance failed",
	                            expVerdict, Test.Framework.TestVerdict.Fail);
	                    return result;
	                }
	            }
	            if (!dp1res || !dp2res || !dp3res) {
	                 result = new Test.Framework.TestResult(
	                        expResult, "failed to discover all participants",
	                        expVerdict, Test.Framework.TestVerdict.Fail);
	                return result;
	            }
	        }
	        
	        returnCode = dp1.GetDiscoveredTopics(ref sequence1);

            if (returnCode != DDS.ReturnCode.Ok) {
                result = new Test.Framework.TestResult(expResult, "get_discovered_topics returned not OK."
                        , expVerdict, Test.Framework.TestVerdict.Fail);
                    return result;
    
    
            } else {
		        if (sequence1.Length != 10) {
	                result = new Test.Framework.TestResult(expResult, "get_discovered_topics failed found: "+sequence1.Length+" expected 10"
	                    , expVerdict, Test.Framework.TestVerdict.Fail);
	                return result;
	            }

	            /* read from each handle the name and check the result against the made topic names*/
	            for (int i=0; i<sequence1.Length;i++) {

	                returnCode = dp1.GetDiscoveredTopicData(ref topic_data,sequence1[i]);
	                
	                if (returnCode == DDS.ReturnCode.Ok) {
                        if ( topic_data.Name.CompareTo("Topic1") == 0) {
                            t1res = true;
                        } else if (topic_data.Name.CompareTo("Topic2") == 0) {
                            t2res= true;
                        } 
	                } else {
	                     result = new Test.Framework.TestResult(
	                            expResult, "get discovered topic data failed",
	                            expVerdict, Test.Framework.TestVerdict.Fail);
	                    return result;
	                }
	            }
	            if (!t1res || !t2res) {
	                 result = new Test.Framework.TestResult(
	                        expResult, "failed to discover all topics",
	                        expVerdict, Test.Framework.TestVerdict.Fail);
	                return result;
	            }
	            
	        result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict);
	        return result;   
          } 
      }
    }
}
