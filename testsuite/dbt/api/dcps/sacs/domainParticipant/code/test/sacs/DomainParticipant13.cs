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
    public class DomainParticipant13 : Test.Framework.TestCase
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
        public DomainParticipant13()
            : base("sacs_domainParticipant_tc13", "sacs_domainParticipant"
                , "domainParticipant", "Test the get_current_time method", "Test the get_current_time method"
                , null)
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
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "get_current_time test succeeded";
            DDS.IDomainParticipant participant;
            DDS.ReturnCode returnCode;
            DDS.Time timeHolder;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            returnCode = participant.GetCurrentTime(out timeHolder);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, expResult, expVerdict, Test.Framework.TestVerdict.Pass);
            }
            else
            {
                result = new Test.Framework.TestResult(expResult, "get_current_time test failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            return result;
        }
    }
}
