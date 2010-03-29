namespace test.sacs
{
    /// <summary>
    /// Tests the create_contentfilteredtopic and the delete_contentfilteredtopic
    /// functions.
    /// </summary>
    /// <remarks>
    /// Tests the create_contentfilteredtopic and the delete_contentfilteredtopic
    /// functions.
    /// </remarks>
    public class DomainParticipant10 : Test.Framework.TestCase
    {
        /// <summary>
        /// Tests the create_contentfilteredtopic and the
        /// delete_contentfilteredtopic functions.
        /// </summary>
        /// <remarks>
        /// Tests the create_contentfilteredtopic and the
        /// delete_contentfilteredtopic functions.
        /// </remarks>
        public DomainParticipant10()
            : base("sacs_domainParticipant_tc10", "sacs_domainParticipant"
                , "domainParticipant", "create- and delete_contentfilteredtopic test.", "create- and delete_contentfilteredtopic test."
                , null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPreItem(new test.sacs.DomainParticipantItem1Init());
            this.AddPostItem(new test.sacs.DomainParticipantItem1Deinit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        private string CreateParameterExpression(string fieldname, int num)
        {
            string expression = fieldname + " < %0";
            int i;
            for (i = 1; i < num; i++)
            {
                expression = expression + " AND " + fieldname + " < %" + i;
            }
            return expression;
        }

        public override Test.Framework.TestResult Run()
        {
            string filteredTypeName = "my_filtered_topic";
            string filterExpression = "long_1 < 1";
            string filterExpression2 = "long_1 > " + System.Convert.ToString(((ulong)long.MaxValue) + 1);
            //        final String filterExpression3 = "long_1 < %99";
            //        final String filterExpression4 = "long_1 < %100";
            //        final String filterExpression5 = "long_1 < %101";
            string filterExpression3 = CreateParameterExpression("long_1", 99);
            string filterExpression4 = CreateParameterExpression("long_1", 101);
            string filterExpression5 = CreateParameterExpression("long_1", 102);
            string[] expressionParameters = new string[] { "1", "2", "3" };
            string[] expressionParameters3 = new string[99];
            string[] expressionParameters4 = new string[101];
            string[] expressionParameters5 = new string[102];
            string expResult = "create- and delete_contentfilteredtopic test succeeded.";
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IContentFilteredTopic filteredTopic1;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            if (participant == null || topic == null)
            {
                System.Console.Error.WriteLine("DomainParticipant10: participant or topic = null"
                    );
            }
            Utils.FillStringArray(ref expressionParameters3, "10");
            Utils.FillStringArray(ref expressionParameters4, "10");
            Utils.FillStringArray(ref expressionParameters5, "10");

            filteredTopic1 = participant.CreateContentFilteredTopic(null, topic, filterExpression
                , expressionParameters);
            if (filteredTopic1 != null)
            {
                result.Result = "could create a ContentFilteredTopic without a topic name (1).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, null,
                filterExpression, expressionParameters);
            if (filteredTopic1 != null)
            {
                result.Result = "could create a ContentFilteredTopic without a related topic (2).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                null, expressionParameters);
            if (filteredTopic1 != null)
            {
                result.Result = "could create a ContentFilteredTopic without a filter expression (3).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression, null);
            if (filteredTopic1 == null)
            {
                result.Result = "could not create a ContentFilteredTopic without expression parameters (4).";
                return result;
            }
            rc = participant.DeleteContentFilteredTopic(filteredTopic1);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a ContentFilteredTopic (5).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression2, null);
            if (filteredTopic1 == null)
            {
                result.Result = "could not create a ContentFilteredTopic with filter \"FIELDNAME > (MAX_VALUE + 1)\"(6).";
                return result;
            }
            rc = participant.DeleteContentFilteredTopic(filteredTopic1);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a ContentFilteredTopic (7).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression3, expressionParameters3);
            if (filteredTopic1 == null)
            {
                result.Result = "could not create a ContentFilteredTopic with 99 expression parameters (8).";
                return result;
            }
            rc = participant.DeleteContentFilteredTopic(filteredTopic1);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a ContentFilteredTopic (9).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression4, expressionParameters4);
            if (filteredTopic1 != null)
            {
                participant.DeleteContentFilteredTopic(filteredTopic1);
                result.Result = "could create a ContentFilteredTopic with 100 expression parameters (10).";
                return result;
            }
            filteredTopic1 = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression5, expressionParameters5);
            if (filteredTopic1 != null)
            {
                participant.DeleteContentFilteredTopic(filteredTopic1);
                result.Result = "could create a ContentFilteredTopic with 101 expression parameters (12).";
                return result;
            }
            //        participant.delete_contentfilteredtopic(filteredTopic1);
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
