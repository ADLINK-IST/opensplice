namespace test.sacs
{
    /// <summary>
    /// According to appendix B of the DDS spec the expression parameter list may
    /// not be greater than 100 elements.
    /// </summary>
    /// <remarks>
    /// According to appendix B of the DDS spec the expression parameter list may
    /// not be greater than 100 elements. It is not specified what will happen if
    /// the list is greater than 100 elements.
    /// This testcase tests if the ContentFilterTopic can be created for an
    /// expression parameter list of 101 elements.
    /// </remarks>
    public class DomainParticipant12 : Test.Framework.TestCase
    {
        /// <summary>Test the use of invalid expression parameters for a ContentFilteredTopic.
        /// 	</summary>
        /// <remarks>
        /// Test the use of invalid expression parameters for a ContentFilteredTopic.
        /// A ContentFilteredTopic is created which has a filter expression that
        /// references expression parameter #99 while the expression parameter list
        /// only contains 3 elements.
        /// It is assumed that the ContentFilteredTopic should not be created.
        /// </remarks>
        public DomainParticipant12()
            : base("sacs_domainParticipant_tc12", "sacs_domainParticipant"
                , "domainParticipant", "Test the use of invalid expression parameters for a ContentFilteredTopic"
                , "Test the use of invalid expression parameters for a ContentFilteredTopic", null
                )
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPreItem(new test.sacs.DomainParticipantItem1Init());
            this.AddPreItem(new test.sacs.DomainParticipantItem2Init());
            this.AddPostItem(new test.sacs.DomainParticipantItem2Deinit());
            this.AddPostItem(new test.sacs.DomainParticipantItem1Deinit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            string filterExpression = "long_1 < %99";
            string[] expressionParameters = new string[] { "1", "2", "3" };
            string expResult = "ContentFilteredTopic test succeeded";
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IContentFilteredTopic filteredTopic;
            Test.Framework.TestResult result;
            topic = (DDS.ITopic)this.ResolveObject("topic");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            filteredTopic = participant.CreateContentFilteredTopic("tc4_filtered_topic", topic
                , filterExpression, expressionParameters);
            if (filteredTopic != null)
            {
                System.Console.Out.WriteLine("NOTE\t\t: See STR/CP TH282");
                participant.DeleteContentFilteredTopic(filteredTopic);
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                result.Result = "could create a ContentFilteredTopic which refers to non existing param %99";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
