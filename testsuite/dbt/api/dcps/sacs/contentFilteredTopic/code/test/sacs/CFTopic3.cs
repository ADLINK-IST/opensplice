namespace test.sacs
{
    /// <summary>Basic test of all ContentFilteredTopic functions.</summary>
    /// <remarks>Basic test of all ContentFilteredTopic functions.</remarks>
    public class CFTopic3 : Test.Framework.TestCase
    {
        /// <summary>Basic test of all ContentFilteredTopic functions.</summary>
        /// <remarks>Basic test of all ContentFilteredTopic functions.</remarks>
        public CFTopic3()
            : base("sacs_content_filtered_topic_tc3", "sacs_content_filtered_topic"
                , "sacs_content_filtered_topic", "sacs_content_filtered_topic", "Test whether creation of ContentFilteredTopic succeeds when name shorter than topic name."
                , null)
        {
            this.AddPreItem(new test.sacs.CFTopicItem1Init());
            this.AddPostItem(new test.sacs.CFTopicItem1Deinit());
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "ContentFilteredTopic test succeeded";
            string filteredTypeName = "m";
            string filterExpression = "long_1 < 10";
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IContentFilteredTopic filteredTopic;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            filteredTopic = participant.CreateContentFilteredTopic(filteredTypeName, topic,
                filterExpression, null);
            if (filteredTopic == null)
            {
                result.Result = "participant.create_contentfilteredtopic failed (1).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
