
using DDS;
using Test.Framework;
using mod;
using System;
using System.Collections.Generic;

/**
 * 
 * 
 * @date May 04, 2012 
 */
 namespace test.sacs
 {
	public class SampleInfo1 {
	    
	    public const int MAX_DEPTH = 10;
	    public const int MAX_INSTANCE = 10;
	    
        public const int C_LT = -1;
        public const int C_EQ = 0;
        public const int C_GT = 1;
        
	    public class InstanceData {
	        public InstanceStateKind        instanceState;
	        public ViewStateKind            viewState;
	        public int                      DisposedGenerationCount;
	        public int                      NoWritersGenerationCount;
	        public int                      numSamples;
	        public LinkedList<SampleData>   samples;
	        public LinkedList<SampleData>   expected;
	    };
	    
	    public class SampleData {
	        public tst                 data;
	        public SampleStateKind     SampleState;
	        public ViewStateKind       ViewState;
	        public InstanceStateKind   InstanceState;
	        public int                 DisposedGenerationCount;
	        public int                 NoWritersGenerationCount;
	        public int                 SampleRank;
	        public int                 GenerationRank;
	        public int                 AbsoluteGenerationRank;
	        public bool                seen;
	    };
	    
	    public interface SampleSelect {
	        bool matches (SampleData sample, int arg);
	    }
	    
	    public class FieldValueGreaterOrEqual : SampleSelect {
	        public bool matches (SampleData sample, int arg)
	        {
	            bool match = false;
	
	            if ( sample.data.long_3 >= arg ) {
	                match = true;
	            }
	            return match;
	        }
	    }
	    
	    public class FieldValueLessOrEqual : SampleSelect {
	        public bool matches (SampleData sample, int arg)
	        {
	            bool match = false;
	
	            if ( sample.data.long_3 <= arg ) {
	                match = true;
	            }
	            return match;
	        }
	    }
	    
	    private TestFramework tfw;
	    private int domainId = 0;
	    private DomainParticipantFactory factory;
	    private IDomainParticipant participant;
	    private ITopic topic;
	    private ISubscriber subscriber;
	    private DataReaderQos drQos = new DataReaderQos();
	    private ItstDataReader reader;
	    private IPublisher publisher;
	    private DataWriterQos dwQos = new DataWriterQos();
	    private ItstDataWriter writer;
	    private bool proceed = true;
	    private ReturnCode result;
	    private tst[] dataList = null;
	    private SampleInfo[] infoList = null;
	    private FieldValueGreaterOrEqual fieldValueGreaterOrEqual = new FieldValueGreaterOrEqual();
	    private FieldValueLessOrEqual fieldValueLessOrEqual = new FieldValueLessOrEqual();
	    private InstanceData[] instanceDataList = new InstanceData[MAX_INSTANCE];
	    private tst[] testData = new tst[MAX_INSTANCE];
	        
	    public SampleInfo1() {
	        tfw = new TestFramework();
	    }
	    
	    static int timeCompare(Time t1, Time t2)
	    {
	        if (t1.Sec < t2.Sec) return C_LT;
	        if (t1.Sec > t2.Sec) return C_GT;
	        if (t1.NanoSec < t2.NanoSec) return C_LT;
	        if (t1.NanoSec > t2.NanoSec) return C_GT;
	        return C_EQ;
	    }
	    
	    void reportResultCode(ReturnCode code)
	    {
	        string msg;
	
	        switch ( code ) {
	            case ReturnCode.Ok:
	                msg = "result is OK";
	                break;
	            case ReturnCode.Error:
	                msg = "result is ERROR";
	                break;
	            case ReturnCode.Unsupported:
	                msg = "result is UNSUPPORTED";
	                break;
	            case ReturnCode.BadParameter:
	                msg = "result is BAD_PARAMETER";
	                break;
	            case ReturnCode.PreconditionNotMet:
	                msg = "result is PRECONDITION_NOT_MET";
	                break;
	            case ReturnCode.OutOfResources:
	                msg = "result is OUT_OF_RESOURCES";
	                break;
	            case ReturnCode.NotEnabled:
	                msg = "result is NOT_ENABLED";
	                break;
	            case ReturnCode.ImmutablePolicy:
	                msg = "result is IMMUTABLE_POLICY";
	                break;
	            case ReturnCode.InconsistentPolicy:
	                msg = "result is INCONSISTENT_POLICY";
	                break;
	            case ReturnCode.AlreadyDeleted:
	                msg = "result is ALREADY_DELETED";
	                break;
	            case ReturnCode.Timeout:
	                msg = "result is TIMEOUT";
	                break;
	            case ReturnCode.NoData:
	                msg = "result is NO_DATA";
	                break;
	            default:
	                msg = "result is UNKNOWN";
	                break;
	        }
	
	        tfw.TestMessage(TestMessage.Note, msg);
	    }
	
	    private bool checkSampleInfo (SampleInfo info, SampleData reference)
	    {
	        bool equal = true;
	
	        if ( info.SampleState != reference.SampleState ) {
	            Console.Write(string.Format("                : " + 
	                   "received info.SampleState = {0}   " +
	                   "expected reference.SampleState = {1}\n",
	                   info.SampleState,
	                   reference.SampleState));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: SampleState incorrect");
	            equal = false;
	        }
	        if ( info.ViewState != reference.ViewState ) {
	            Console.Write(string.Format("                : " +
	                   "received info.ViewState = {0}   " +
	                   "expected reference.ViewState = {1}\n",
	                   info.ViewState,
	                   reference.ViewState));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: ViewState incorrect");
	            equal = false;
	        }
	        if ( info.InstanceState != reference.InstanceState ) {
	            Console.Write(string.Format("                : " +
	                   "received info.InstanceState = {0}   " +
	                   "expected reference.InstanceState = {1}\n",
	                   info.InstanceState,
	                   reference.InstanceState));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: InstanceState incorrect");
	            equal = false;
	        }
	        if ( info.DisposedGenerationCount != reference.DisposedGenerationCount ) {
	            Console.Write(string.Format("                : " + 
	                   "received info.DisposedGenerationCount = {0} " +
	                   "expected reference.DisposedGenerationCount = {1}\n",
	                   info.DisposedGenerationCount,
	                   reference.DisposedGenerationCount));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: DisposedGenerationCount incorrect");
	            equal = false;
	        }
	        if ( info.NoWritersGenerationCount != reference.NoWritersGenerationCount ) {
	            Console.Write(string.Format("                : " +
	                   "received info.NoWritersGenerationCount = {0} " +
	                   "expected reference.NoWritersGenerationCount = {1}\n",
	                   info.NoWritersGenerationCount,
	                   reference.NoWritersGenerationCount));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: NoWritersGenerationCount incorrect");
	            equal = false;
	        }
	        if ( info.SampleRank != reference.SampleRank ) {
	            Console.Write(string.Format("                : " +
	                   "received info.SampleRank = {0}   " +
	                   "expected reference.SampleRank = {1}\n",
	                   info.SampleRank,
	                   reference.SampleRank));
	            tfw.TestMessage(TestMessage.Note, "SampleInfo: SampleRank incorrect");
	            equal = false;
	        }
	        if ( info.GenerationRank != reference.GenerationRank ) {
	            Console.Write(string.Format("                : " +
	                   "received info.GenerationRank = {0}   " +
	                   "expected reference.GenerationRank = {1}\n",
	                   info.GenerationRank,
	                   reference.GenerationRank));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: GenerationRank incorrect");
	            equal = false;
	        }
	        if ( info.AbsoluteGenerationRank != reference.AbsoluteGenerationRank ) {
	            Console.Write(string.Format("                : " +
	                   "received info.AbsoluteGenerationRank = {0} " +
	                   "expected reference.AbsoluteGenerationRank = {1}\n",
	                   info.AbsoluteGenerationRank,
	                   reference.AbsoluteGenerationRank));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: AbsoluteGenerationRank incorrect");
	            equal = false;
	        }
	        if (timeCompare(info.SourceTimestamp, info.ReceptionTimestamp) == C_GT) {
	            Console.Write(string.Format("                : " +
	                   "received info.SourceTimestamp = {0}.{1:D9} " +
	                   "received info.ReceptionTimestamp = {2}.{3:D9})\n",
	                   info.SourceTimestamp.Sec, info.SourceTimestamp.NanoSec,
	                   info.ReceptionTimestamp.Sec, info.ReceptionTimestamp.NanoSec));
	            tfw.TestMessage(TestMessage.Note,
	                         "SampleInfo: SourceTimestamp > ReceptionTimestamp");
	            equal = false;
	        }
	        
	        return equal;
	    }
	
	    
	    private void initialiseInstanceData ()
	    {
	        for ( int i = 0; i < MAX_INSTANCE; i++ ) {
	            instanceDataList[i] = new InstanceData();
	            instanceDataList[i].instanceState = InstanceStateKind.Alive;
	            instanceDataList[i].viewState = ViewStateKind.New;
	            instanceDataList[i].DisposedGenerationCount = 0;
	            instanceDataList[i].NoWritersGenerationCount = 0;
	            instanceDataList[i].samples = new LinkedList<SampleData>();
	            instanceDataList[i].expected = null;
	            instanceDataList[i].numSamples = 0;
	        }
	    } 
	       
	    private void addSample (tst data)
	    {
	        InstanceData instanceData;
	        SampleData   sampleData = new SampleData();
	
	        instanceData = instanceDataList[data.long_1];
	
	        sampleData.data = new tst();
	        sampleData.data.long_1 = data.long_1;
	        sampleData.data.long_2 = data.long_2; 
	        sampleData.data.long_3 = data.long_3;
	
	        sampleData.SampleState = SampleStateKind.NotRead;
	
	        if ( instanceData.samples.Count > 0 ) {
	            if ( instanceData.instanceState != InstanceStateKind.Alive ) {
	                instanceData.viewState = ViewStateKind.New;
	            } else {
	                instanceData.viewState = ViewStateKind.NotNew;
	            }
	        }
	
	        instanceData.instanceState = InstanceStateKind.Alive;
	
	        sampleData.DisposedGenerationCount   = instanceData.DisposedGenerationCount;
	        sampleData.NoWritersGenerationCount = instanceData.NoWritersGenerationCount;
	
	        instanceData.numSamples++;
	        if ( instanceData.numSamples > MAX_DEPTH ) {
	            instanceData.samples.RemoveLast();
	            instanceData.numSamples = MAX_DEPTH;
	        }
	        instanceData.samples.AddFirst(sampleData);
	    }
	    
	    private void markRead (tst[] data, int len)
	    {
	        for ( int i = 0; i < len; i++ ) {
	            InstanceData instanceData = null;
	            SampleData sampleData = null;
	
	            instanceData = instanceDataList[data[i].long_1];
	
	            instanceData.viewState = ViewStateKind.NotNew;
	
	            foreach (SampleData index in instanceData.samples) {
	                if (sampleCompare(index.data, data[i]) == C_EQ) {
	                    sampleData = index;
	                    break;
	                }
	            }
	            
	            if ( sampleData != null) {
	                sampleData.SampleState = SampleStateKind.Read;
	            }
	        }
	    }
	
	    private void markDisposedAction(SampleData sample)
	    {
	        sample.InstanceState = InstanceStateKind.NotAliveDisposed;
	    }
	
	    private void markDisposed (int i)
	    {
	        InstanceData instanceData;
	
	        instanceData = instanceDataList[i];
	
	        instanceData.instanceState = InstanceStateKind.NotAliveDisposed;
	        instanceData.DisposedGenerationCount++;
	
	        foreach (SampleData sampleData in instanceData.samples) {
	            markDisposedAction(sampleData);
	        }
	    }
	    
	    private void markNoWriterAction(SampleData sample)
	    {
	        sample.InstanceState = InstanceStateKind.NotAliveNoWriters;
	    }
	
	    private void markNoWriter (int i)
	    {
	        InstanceData instanceData;
	
	        instanceData = instanceDataList[i];
	
	        instanceData.instanceState = InstanceStateKind.NotAliveNoWriters;
	        instanceData.NoWritersGenerationCount++;
	
	        foreach (SampleData sampleData in instanceData.samples) {
	            markNoWriterAction(sampleData);
	        }
	    }
	
	    
	    private ReturnCode writeSample (ItstDataWriter writer, tst data)
	    {
	        ReturnCode result;
	
	        result = writer.Write(data, InstanceHandle.Nil);
	
	        if ( result == ReturnCode.Ok ) {
	            addSample(data);
	        }
	        return result;
	    }
	    
	    private ReturnCode disposeSample (ItstDataWriter writer, tst data)
	    {
	        ReturnCode result;
	
	        result = writer.Dispose(data, InstanceHandle.Nil);
	
	        if ( result == ReturnCode.Ok ) {
	            markDisposed(data.long_1);
	        }
	        return result;
	    }
	
	    
	    public class GenerationRankActionArg {
	        public int mrsCount;
	        public int mrsicCount;
	    };
	
	    
	    private void determineGenerationRankAction(SampleData sample, GenerationRankActionArg a)
	    {
	        int count;
	
	        if (sample != null) {
	            count = sample.DisposedGenerationCount + sample.NoWritersGenerationCount;
	            sample.GenerationRank = a.mrsicCount - count;
	            sample.AbsoluteGenerationRank = a.mrsCount - count;
	        }
	    }
	    
	    private void determineGenerationRank (InstanceData instanceData, SampleData mrs, SampleData mrsic)
	    {
	        GenerationRankActionArg arg = new GenerationRankActionArg();
	
	        if ((instanceData != null) && (mrs != null) && (mrsic != null)) {
	            arg.mrsCount   = mrs.DisposedGenerationCount + mrs.NoWritersGenerationCount;
	            arg.mrsicCount = mrsic.DisposedGenerationCount + mrsic.NoWritersGenerationCount;
	
	            foreach (SampleData sampleData in instanceData.expected) {
	                determineGenerationRankAction(sampleData, arg);
	            }
	        }
	    }
	
	    
	    public class SelectSampleActionArg {
	        public SampleSelect  matches;
	        public InstanceData instanceData;
	        public SampleData mrs;
	        public SampleData mrsic;
	        public int rank;
	        public int arg;
	    };
	    
	    private void selectSampleAction(SampleData sampleData, SelectSampleActionArg a)
	    {
	        if (sampleData != null) {
	            if ( a.mrs == null ) {
	                a.mrs = sampleData;
	            }
	            if ( a.matches.matches(sampleData, a.arg) ) {
	                if ( a.mrsic == null ) {
	                    a.mrsic = sampleData;
	                }
	                sampleData.ViewState     = a.instanceData.viewState;
	                sampleData.InstanceState = a.instanceData.instanceState;
	                sampleData.SampleRank    = a.rank++;
	                sampleData.GenerationRank = 0;
	                sampleData.AbsoluteGenerationRank = 0;
	                a.instanceData.expected.AddFirst(sampleData);
	                sampleData.seen = false;
	            }
	        }
	    }
	    
	    private void selectSamples (SampleSelect matches, int arg)
	    {
	        SelectSampleActionArg actionArg = new SelectSampleActionArg();
	
	        for ( int i = 0; i < MAX_INSTANCE; i++ ) {
	            actionArg.matches = matches;
	            actionArg.instanceData = instanceDataList[i];
	            actionArg.instanceData.expected = new LinkedList<SampleData>();
	            actionArg.mrs = null;
	            actionArg.mrsic = null;
	            actionArg.rank = 0;
	            actionArg.arg = arg;
	            
	            foreach (SampleData sampleData in instanceDataList[i].samples) {
	                selectSampleAction(sampleData, actionArg);
	            }
	
	            determineGenerationRank(actionArg.instanceData,
	                                    actionArg.mrs,
	                                    actionArg.mrsic);
	        }
	    }
	    
	    private bool
	    markSeenAction(SampleData sample, bool seen)
	    {
	        if (seen) {
	            seen = sample.seen;
	        }
	        return seen;
	    }
	
	    
	    private bool allSamplesReceived ()
	    {
	        bool seen = true;
	
	        for ( int i = 0; seen && (i < MAX_INSTANCE); i++ ) {
	            foreach (SampleData sampleData in instanceDataList[i].expected) {
	                seen = markSeenAction(sampleData, seen);
	            }
	        }
	        return seen;
	    }
	
	    
	    private bool checkReceivedInstanceSample (tst[] data, SampleInfo[] info, int len)
	    {
	        SampleData sampleData = null;
	        bool noError = true;
	
	        for ( int i = 0; noError && (i < len); i++ ) {
	            int x = data[i].long_1;
	
	            if ( x < MAX_INSTANCE ) {
	                if ( instanceDataList[x].expected != null) {
	                    foreach (SampleData index in instanceDataList[x].expected) {
	                        if (sampleCompare(index.data, data[i]) == C_EQ) {
	                            sampleData = index;
	                            break;
	                        }
	                    }
	                    if ( sampleData != null) {
	                        sampleData.seen = true;
	                        noError = checkSampleInfo(info[i], sampleData);
	                    } else {
	                        noError = false;
	                    }
	                } else {
	                    noError = false;
	                }
	            } else {
	                noError = false;
	            }
	        }
	
	        if ( noError ) {
	            noError = allSamplesReceived();
	        }
	
	        return noError;
	    }
	    
	    private int sampleCompare (tst sample1, tst sample2)
	    {
	        int result;
	
	        if ( (sample1.long_1 == sample2.long_1) &&
	             (sample1.long_2 == sample2.long_2) &&
	             (sample1.long_3 == sample2.long_3) ) {
	            result = C_EQ;
	        } else if ( sample1.long_1 != sample2.long_1 ) {
	            if ( sample1.long_1 < sample2.long_1 ) {
	                result = C_LT;
	            } else {
	                result = C_GT;
	            }
	        } else if ( sample1.long_2 != sample2.long_2 ) {
	            if ( sample1.long_2 < sample2.long_2 ) {
	                result = C_LT;
	            } else {
	                result = C_GT;
	            }
	        } else  {
	            if ( sample1.long_3 < sample2.long_3 ) {
	                result = C_LT;
	            } else {
	                result = C_GT;
	            }
	        }
	
	        return result;
	    }
	
	    
	    private void init()
	    {
	        tstTypeSupport typeSupport;
	        string errMsg = "Unknown error";
	        
	        for ( int i = 0; i < MAX_INSTANCE; i++ ) {
	            testData[i] = new tst();
	            testData[i].long_1 = i;
	            testData[i].long_2 = 0;
	            testData[i].long_3 = 0;
	        }
	        initialiseInstanceData();
	
	        
	        /**
	         * @addtogroup group_dds1290
	         *
	         * \b Test \b ID: \b saj_invalid_data_000
	         *
	         * \b Test \b Objectives:
	         *
	         * Create and initialise all required DDS entities
	         *
	         * \b Test \b Procedure:
	         *
	         * \e Action
	         *
	         * The following entities are obtained/created
	         * \arg \c DomainParticipantFactory
	         * \arg \c DomainParticipant with default QoS settings
	         * \arg \c Publisher with default QoS settings
	         * \arg \c Subscriber with default QoS settings
	         * \arg \c The mod::tst type is registered
	         * \arg \c A topic T1 of type tstModule::tst is created with default QoS settings
	         * \arg \c A DataWriter W for T1 with default QoS settings, writer_data_lifecycle.autodispose_unregistered_instances = FALSE
	         * \arg \c A DataReader for T1 with default QoS settings
	         *
	         * \e Result
	         * It is expected that all entities are created/initialized correctly and without any failures. \n
	         * If a failure occurs at any of the above stages, the test fails, this is reported and no further testing is performed
	         */
	        /*- INITIALIZATION ---------------------------------------------------------*/
	        tfw.TestStart ("sacs_sampleInfo_000","SampleInfo","initialization");
	        tfw.TestTitle ("Test SampleInfo initialization.");
	        tfw.TestPurpose ("Test SampleInfo initialization.");
	
	        factory = DomainParticipantFactory.Instance;
	        
	        if(factory == null){
	            errMsg = "DomainParticipantFactory could NOT be resolved";
	            proceed = false;
	        } else {
	            participant = factory.CreateParticipant(domainId);
	
	            if(participant == null){
	                errMsg = "DomainParticipant could NOT be created";
	                proceed = false;
	            } else {
	                typeSupport = new tstTypeSupport();
	
	                result = typeSupport.RegisterType(participant, "tst");
	                if(result == ReturnCode.Ok){
	                    topic = participant.CreateTopic("my_topic", "tst");
	
	                    if(topic != null){
	                        subscriber = participant.CreateSubscriber();
	
	                        if(subscriber != null){
	                            subscriber.GetDefaultDataReaderQos(ref drQos);
	                            
	                            if(drQos != null){
	                                drQos.History.Kind  = HistoryQosPolicyKind.KeepLastHistoryQos;
	                                drQos.History.Depth = MAX_DEPTH;
	                                reader = subscriber.CreateDataReader(topic, drQos) as tstDataReader;
	    
	                                if(reader != null){
	                                    publisher = participant.CreatePublisher();
	    
	                                    if(publisher != null){
	                                        result = publisher.GetDefaultDataWriterQos(ref dwQos);
	                                        if(dwQos != null && result == ReturnCode.Ok){
	                                            dwQos.WriterDataLifecycle.AutodisposeUnregisteredInstances = false;
	                                            writer = publisher.CreateDataWriter(topic, dwQos) as tstDataWriter;
	                                            if(writer == null){
	                                                errMsg = "DataWriter could NOT be created";
	                                                proceed = false;
	                                            }
	                                        } else {
	                                            reportResultCode(result);
	                                            errMsg = "Default DataWriterQos could NOT be retrieved";
	                                            proceed = false;
	                                        }
	                                    } else {
	                                        errMsg = "Publisher could NOT be created";
	                                        proceed = false;
	                                    }
	    
	                                } else {
	                                    errMsg = "DataReader could NOT be created";
	                                    proceed = false;
	                                }
	                            } else {
	                                errMsg = "Default DataReaderQos could not be resolved.";
	                                proceed = false;
	                            }
	                        } else {
	                            errMsg = "Subscriber could NOT be created";
	                            proceed = false;
	                        }
	                    } else {
	                        errMsg = "Topic could NOT be created";
	                        proceed = false;
	                    }
	                } else {
	                    errMsg = "Typesupport NOT loaded into DomainParticipant";
	                    proceed = false;
	                }
	            }
	        }
	        
	        if(proceed == true){
	            tfw.TestResult("Initialization OK", "Initialization OK", TestVerdict.Pass, TestVerdict.Pass);
	            tfw.TestFinish();
	        } else {
	            tfw.TestResult("Initialization OK", errMsg, TestVerdict.Pass, TestVerdict.Fail);
	            tfw.TestFinish();
	        }
	        /*- END OF INITIALIZATION --------------------------------------------------*/
	    }
	    
	    private void runTests() {
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_001","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when no samples available");
	        tfw.TestPurpose ("Check that the read returns NO_DATA");
	
	        if ( reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.NoData ) {
	                tfw.TestResult("result == NO_DATA",
	                                   "result == NO_DATA", TestVerdict.Pass,TestVerdict.Pass);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = writeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_002","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when one sample per instance is available");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        {
	            selectSamples(fieldValueGreaterOrEqual, 0);
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_003","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data second time when one sample per instance is available");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = writeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_004","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when two samples per instance are available");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int j = 0; proceed && (j < MAX_DEPTH); j++ ) {
	                for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                    result = writeSample(writer, testData[i]);
	                    if ( result == ReturnCode.Ok ) {
	                        testData[i].long_2++;
	                        testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                    } else {
	                        proceed = false;
	                    }
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_005","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when a number of samples per instance is available");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        {
	            selectSamples(fieldValueLessOrEqual, 5);
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_006","tc_sampleinfo","DataReader.read_w_condition");
	        tfw.TestTitle ("read a selection of the samples using a query condition");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            string[] prms = new string[0];
	            IQueryCondition query;
	
	            query = reader.CreateQueryCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any, "long_3 <= 5", prms);
	            if ( query != null) {
	                result = reader.ReadWithCondition(ref dataList, ref infoList, Length.Unlimited, query);
	                if ( result == ReturnCode.Ok ) {
	                    if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                        tfw.TestResult("received data and sampleinfo is correct",
	                                           "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                    } else {
	                        tfw.TestResult("received data and sampleinfo is correct",
	                                           "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                    }
	                    markRead(dataList, dataList.Length);
	                    reader.ReturnLoan(ref dataList, ref infoList);
	                } else {
	                    reportResultCode(result);
	                    tfw.TestResult("result == NO_DATA",
	                                       "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                reader.DeleteReadCondition(query);
	            } else {
	                tfw.TestResult("create query succeeded",
	                                   "create query failed", TestVerdict.Pass,TestVerdict.Fail);
	             }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = disposeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_007","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances are disposed");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        {
	            selectSamples(fieldValueLessOrEqual, 5);
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_008","tc_sampleinfo","DataReader.read_w_condition");
	        tfw.TestTitle ("read a selection of the samples using a query condition when the instances are not ALIVE");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            string[] prms = new string[0];
	            IQueryCondition query;
	
	            query = reader.CreateQueryCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any, "long_3 <= 5", prms);
	            if ( query != null) {
	                result = reader.ReadWithCondition(ref dataList, ref infoList, Length.Unlimited, query);
	                if ( result == ReturnCode.Ok ) {
	                    if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                        tfw.TestResult("received data and sampleinfo is correct",
	                                           "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                    } else {
	                        tfw.TestResult("received data and sampleinfo is correct",
	                                           "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                    }
	                    markRead(dataList, dataList.Length);
	                    reader.ReturnLoan(ref dataList, ref infoList);
	                } else {
	                    reportResultCode(result);
	                    tfw.TestResult("result == NO_DATA",
	                                       "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                reader.DeleteReadCondition(query);
	            } else {
	                tfw.TestResult("create query succeeded",
	                                   "create query failed", TestVerdict.Pass,TestVerdict.Fail);
	             }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = writeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_009","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances are alive again");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct",
	                                        TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct",
	                                        TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA",
	                                    TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",
	                                 TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        {
	            selectSamples(fieldValueLessOrEqual, 5);
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_010","tc_sampleinfo","DataReader.read_w_condition");
	        tfw.TestTitle ("read a selection of the samples using a query condition when the instances are alive");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            string[] prms = new string[0];
	            IQueryCondition query;
	
	            query = reader.CreateQueryCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any, "long_3 <= 5", prms);
	            if ( query != null) {
	                result = reader.ReadWithCondition(ref dataList, ref infoList, Length.Unlimited, query);
	                if ( result == ReturnCode.Ok ) {
	                    if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                        tfw.TestResult(
	                            "received data and sampleinfo is correct",
	                            "received data and sampleinfo is correct",
	                             TestVerdict.Pass,TestVerdict.Pass);
	                    } else {
	                        tfw.TestResult(
	                            "received data and sampleinfo is correct",
	                            "received data and sampleinfo is not correct",
	                             TestVerdict.Pass,TestVerdict.Fail);
	                    }
	                    markRead(dataList, dataList.Length);
	                    reader.ReturnLoan(ref dataList, ref infoList);
	                } else {
	                    reportResultCode(result);
	                    tfw.TestResult("result == NO_DATA",
	                                       "result != NO_DATA",
	                                        TestVerdict.Pass,TestVerdict.Fail);
	                }
	                reader.DeleteReadCondition(query);
	            } else {
	                tfw.TestResult("create query succeeded",
	                                   "create query failed",
	                                    TestVerdict.Pass,TestVerdict.Fail);
	             }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",
	                                 TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = disposeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_011","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances are alive again");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct",
	                                        TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct",
	                                        TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA",
	                                    TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",
	                                 TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = writeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_012","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances are alive again");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct",
	                                        TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct",
	                                        TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA",
	                                    TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",
	                                 TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( writer != null) {
	            result = publisher.DeleteDataWriter(writer);
	            writer = null;
	            if ( result == ReturnCode.Ok ) {
	                for ( int i = 0; i < MAX_INSTANCE; i++ ) {
	                    markNoWriter(i);
	                }
	            } else {
	                proceed = false;
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        } else {
	            proceed = false;
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_013","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances have become not_alive_no_writers again");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct",
	                                        TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct",
	                                        TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA",
	                                    TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",
	                                 TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	        /**************************************************************************/
	        if ( proceed ) {
	            writer = publisher.CreateDataWriter(topic, dwQos, null, 0) as tstDataWriter;
	            if ( writer == null) {
	                proceed = false;
	            }
	        }
	
	        if ( writer != null) {
	            for ( int i = 0; proceed && (i < MAX_INSTANCE); i++ ) {
	                result = writeSample(writer, testData[i]);
	                if ( result == ReturnCode.Ok ) {
	                    testData[i].long_2++;
	                    testData[i].long_3 = testData[i].long_2 % MAX_DEPTH;
	                } else {
	                    proceed = false;
	                }
	            }
	            if ( proceed ) {
	                selectSamples(fieldValueGreaterOrEqual, 0);
	            }
	        }
	
	        /**************************************************************************/
	        tfw.TestStart ("sacs_sampleinfo_014","tc_sampleinfo","DataReader.read");
	        tfw.TestTitle ("read data when the instances are alive again");
	        tfw.TestPurpose ("Check that the read returns the correct SampleInfo");
	
	        if ( proceed && reader != null) {
	            result = reader.Read(ref dataList, ref infoList, Length.Unlimited);
	            if ( result == ReturnCode.Ok ) {
	                if ( checkReceivedInstanceSample(dataList, infoList, dataList.Length) ) {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is correct", TestVerdict.Pass,TestVerdict.Pass);
	                } else {
	                    tfw.TestResult("received data and sampleinfo is correct",
	                                       "received data and sampleinfo is not correct", TestVerdict.Pass,TestVerdict.Fail);
	                }
	                markRead(dataList, dataList.Length);
	                reader.ReturnLoan(ref dataList, ref infoList);
	            } else {
	                reportResultCode(result);
	                tfw.TestResult("result == NO_DATA",
	                                   "result != NO_DATA", TestVerdict.Pass,TestVerdict.Fail);
	            }
	         } else {
	            tfw.TestResult ("precondition ok",
	                                "precondition failed",TestVerdict.Unresolved,TestVerdict.Unresolved);
	        }
	
	        tfw.TestFinish();
	
	    }
	    
	    private void deinit()
	    {
	        /**
	         * @addtogroup group_dds1290
	         *
	         * \b Test \b ID: \b saj_invalid_data_999
	         *
	         * \b Test \b Objectives:
	         *
	         * Deinitialize by deleting all contained entities of the \c DomainPariticipantFactory.
	         *
	         * \b Test \b Procedure:
	         *
	         * \e Action
	         *
	         * Call the method delete_contained_entities() on the DomainParticipantFactory.
	         *
	         * \e Result
	         * It is expected that call returns ok. \n
	         * If the call to delete_contained_entities is not successful, the test fails.
	         */
	        /*- DEINITIALIZATION -------------------------------------------------------*/
	        tfw.TestStart  ("sacs_invalid_data_999","invalid_data","deinitialization");
	        tfw.TestTitle  ("Test deinitialization.");
	        tfw.TestPurpose("Test deinitialization.");
	        /*--------------------------------------------------------------------------*/
	
	        if(proceed == true){
	            if(participant != null){
	
	                result = participant.DeleteContainedEntities();
	                if (result == ReturnCode.Ok){
	                   factory.DeleteParticipant(participant);
	                }
	
	                if(result == ReturnCode.Ok){
	                    tfw.TestResult("Deinitialization OK", "Deinitialization OK", TestVerdict.Pass, TestVerdict.Pass);
	                } else {
	                    reportResultCode(result);
	                    tfw.TestResult("Deinitialization OK", "Deinitialization FAIL", TestVerdict.Pass, TestVerdict.Fail);
	                }
	            } else {
	                tfw.TestResult("Deinitialization OK", "Domain participant is null.", TestVerdict.Pass, TestVerdict.Fail);
	            }
	        } else {
	            tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
	        }
	        tfw.TestFinish();
	
	        /*- DEINITIALIZATION -------------------------------------------------------*/
	    }
	    
	    public void runAll()
	    {
	        init();
	        runTests();
	        deinit();
	    }
	}
}
