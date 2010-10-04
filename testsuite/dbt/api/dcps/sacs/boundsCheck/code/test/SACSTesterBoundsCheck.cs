namespace test
{
    public class BoundsCheckEntities
    {
        public DDS.DomainParticipantFactory factory;
        public DDS.IDomainParticipant participant;
        public DDS.IPublisher publisher;
        public mod.boundsTypeTypeSupport typeSupport;
        public mod.embeddedStructTypeTypeSupport typeSupport2;
        public DDS.ITopic topic;
        public DDS.ITopic topic2;
        public mod.IboundsTypeDataWriter datawriter;
        public mod.IembeddedStructTypeDataWriter datawriter2;
        public mod.boundsType message;
        public mod.embeddedStructType message2;
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
            suite.AddTest(new test.sacs.BoundsCheck10(bce));
            suite.AddTest(new test.sacs.BoundsCheck11(bce));
            suite.AddTest(new test.sacs.BoundsCheck12(bce));
            suite.AddTest(new test.sacs.BoundsCheck13(bce));
            suite.AddTest(new test.sacs.BoundsCheck14(bce));
            suite.AddTest(new test.sacs.BoundsCheck15(bce));
            suite.AddTest(new test.sacs.BoundsCheck16(bce));
            suite.AddTest(new test.sacs.BoundsCheck17(bce));
            suite.AddTest(new test.sacs.BoundsCheck999(bce));
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
