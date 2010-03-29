namespace test.sacs
{
    /// <summary>Basic test of all ContentFilteredTopic functions.</summary>
    /// <remarks>Basic test of all ContentFilteredTopic functions.</remarks>
    public class CFTopic1 : Test.Framework.TestCase
    {
        /// <summary>Basic test of all ContentFilteredTopic functions.</summary>
        /// <remarks>Basic test of all ContentFilteredTopic functions.</remarks>
        public CFTopic1()
            : base("sacs_content_filtered_topic_tc1", "sacs_content_filtered_topic"
                , "sacs_content_filtered_topic", "sacs_content_filtered_topic", "Basic test of all ContentFilteredTopic functions."
                , null)
        {
            this.AddPreItem(new test.sacs.CFTopicItem1Init());
            this.AddPostItem(new test.sacs.CFTopicItem1Deinit());
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "ContentFilteredTopic test succeeded";
            string filteredTypeName = "my_filtered_topic";
            string filterExpression = "long_1 < %0";
            //        final String expressionParameters[] = {"1", "2", "3"}; 
            string[] expressionParameters = new string[] { "1" };
            string[] retrievedExpressionParameters;
            string retrievedFilterExpression;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant retrievedParticipant;
            string name;
            DDS.ITopic topic;
            DDS.ITopic retrievedTopic;
            DDS.IContentFilteredTopic filteredTopic;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string[] ssHolder = null;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            filteredTopic = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression, expressionParameters);
            if (filteredTopic == null)
            {
                result.Result = "participant.create_contentfilteredtopic failed (1).";
                return result;
            }
            name = filteredTopic.TypeName;
            if (name == null)
            {
                result.Result = "get_type_name failed (1).";
                return result;
            }
            if (!name.Equals("my_type"))
            {
                result.Result = "get_type_name returned \"" + name + "\" instead of \"my_type\" (1).";
                return result;
            }
            name = filteredTopic.Name;
            if (name == null)
            {
                result.Result = "operation get_name failed (2).";
                return result;
            }
            if (!name.Equals(filteredTypeName))
            {
                result.Result = "get_name returned " + name + " instead of \"" + filteredTypeName
                     + "\" (2).";
                return result;
            }
            retrievedParticipant = filteredTopic.Participant;
            if (retrievedParticipant == null)
            {
                result.Result = "operation get_participant failed (3).";
                return result;
            }
            if (retrievedParticipant != participant)
            {
                result.Result = "get_participant returned a different participant (3).";
                return result;
            }
            retrievedTopic = filteredTopic.RelatedTopic;
            if (retrievedTopic == null)
            {
                result.Result = "operation get_related_topic failed (4).";
                return result;
            }
            if (retrievedTopic != topic)
            {
                result.Result = "get_related_topic returned a different Topic (4).";
                return result;
            }
            retrievedFilterExpression = filteredTopic.GetFilterExpression();
            if (retrievedFilterExpression == null)
            {
                result.Result = "operation get_filter_expression failed (4).";
                return result;
            }
            if (!retrievedFilterExpression.Equals(filterExpression))
            {
                result.Result = "unexpected expression (\"" + retrievedFilterExpression + "\") after calling get_filter_expression (4).";
                return result;
            }
            rc = filteredTopic.GetExpressionParameters(ref ssHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "operation get_expression_parameters returned " + rc;
                return result;
            }
            
            retrievedExpressionParameters = ssHolder;
            if (retrievedExpressionParameters == null)
            {
                result.Result = "operation get_expression_parameters failed (5).";
                return result;
            }
            //LF, System.Array.Equals tests instance equality - this is not what we are looking for here...
            /*if (!System.Array.Equals(retrievedExpressionParameters, expressionParameters))
            {
                result.Result = "unexpected parameters after calling get_expression_parameters (5)." + retrievedExpressionParameters[0] + " : " + expressionParameters[0] ;
                return result;
            }*/

            if (retrievedExpressionParameters.Length == expressionParameters.Length)
            {
                for (int i = 0; i < retrievedExpressionParameters.Length; i++ )
                {
                    if (retrievedExpressionParameters[i] != expressionParameters[i])
                    {
                        result.Result = "unexpected parameters after calling get_expression_parameters (5)." + retrievedExpressionParameters[i] + " : " + expressionParameters[i];
                        return result;
                    }
                }
            }

            // TODO: JLS, Verify that the hexstring is correct.
            string[] newExpressionParameters = new string[] { int.MaxValue.ToString("X"), 
				"9", int.MinValue.ToString("X") };
            rc = filteredTopic.SetExpressionParameters(newExpressionParameters);
            if (rc != DDS.ReturnCode.Unsupported)
            {
                result.Result = "Recieved return code " + rc + " after calling set_expression_parameters (6).";
                return result;
            }
            rc = filteredTopic.GetExpressionParameters(ref ssHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "operation get_expression_parameters returned " + rc;
                return result;
            }
            retrievedExpressionParameters = ssHolder;
            //if (!System.Array.equals(retrievedExpressionParameters, newExpressionParameters)){
            /*if (!System.Array.Equals(retrievedExpressionParameters, expressionParameters))
            {
                result.Result = "unexpected parameters after calling get_expression_parameters (6).";
                return result;
            }*/
            if (retrievedExpressionParameters.Length == expressionParameters.Length)
            {
                for (int i = 0; i < retrievedExpressionParameters.Length; i++)
                {
                    if (retrievedExpressionParameters[i] != expressionParameters[i])
                    {
                        result.Result = "unexpected parameters after calling get_expression_parameters (5)." + retrievedExpressionParameters[i] + " : " + expressionParameters[i];
                        return result;
                    }
                }
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
