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
    /// This testcase tests if the ContentFilterTopic can be created and used
    /// for a expression parameter list of 101 elements.
    /// </remarks>
    public class DomainParticipant11 : Test.Framework.TestCase
    {
        /// <summary>Tests the use of a ContentFilteredTopic.</summary>
        /// <remarks>Tests the use of a ContentFilteredTopic.</remarks>
        public DomainParticipant11()
            : base("sacs_domainParticipant_tc11", "sacs_domainParticipant"
                , "domainParticipant", "Test the use of expression parameters for a ContentFilteredTopic"
                , "Test the use of expression parameters for a ContentFilteredTopic", null)
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
            string expResult = "ContentFilteredTopic test succeeded";
            string[] expressionParameters;
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IContentFilteredTopic filteredTopic;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            topic = (DDS.ITopic)this.ResolveObject("topic");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            expressionParameters = new string[102];
            Utils.FillStringArray(ref expressionParameters, "10");
            if (topic == null || participant == null)
            {
                System.Console.Error.WriteLine("participant or topic = null");
                result.Result = "precondition not met";
                return result;
            }
            expressionParameters[101] = "5";
            filteredTopic = participant.CreateContentFilteredTopic("filtered_topic", topic,
                "long_1 > %101", expressionParameters);
            if (filteredTopic != null)
            {
                System.Console.Out.WriteLine("NOTE\t\t: See STR/CP TH281");
                participant.DeleteContentFilteredTopic(filteredTopic);
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                result.Result = "Could create a ContentFilteredTopic with an expression parameter %101 ";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
