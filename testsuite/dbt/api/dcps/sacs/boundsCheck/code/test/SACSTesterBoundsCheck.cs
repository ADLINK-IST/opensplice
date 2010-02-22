namespace test
{
    public class BoundsCheckEntities
    {
        public DDS.DomainParticipantFactory factory;
        public DDS.IDomainParticipant participant;
        public DDS.IPublisher publisher;
        public mod.boundsTypeTypeSupport typeSupport;
        public DDS.ITopic topic;
        public mod.boundsTypeDataWriter datawriter;
        public mod.boundsType message;
    }
    
	public class SACSTesterBoundsCheck
	{
		public static void Main(string[] args)
		{
		    BoundsCheckEntities bce = new BoundsCheckEntities();
			Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
			suite.AddMilestone("Test start");
			suite.AddTest(new test.sacs.BoundsCheck0(bce));
			suite.AddTest(new test.sacs.BoundsCheck1(bce));
			suite.AddTest(new test.sacs.BoundsCheck2(bce));
			suite.AddTest(new test.sacs.BoundsCheck3(bce));
			suite.AddTest(new test.sacs.BoundsCheck4(bce));
			suite.AddTest(new test.sacs.BoundsCheck5(bce));
			suite.AddTest(new test.sacs.BoundsCheck6(bce));
			suite.AddTest(new test.sacs.BoundsCheck7(bce));
			suite.AddTest(new test.sacs.BoundsCheck8(bce));
			suite.AddTest(new test.sacs.BoundsCheck9(bce));
			suite.AddTest(new test.sacs.BoundsCheck999(bce));
			suite.AddMilestone("Test end");
			suite.RunTests();
			suite.PrintReport();
		}
	}
}
