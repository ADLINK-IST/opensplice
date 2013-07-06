
using DDS;
using Test.Framework;
using MyTopicModule;

/**
 * 
 * 
 * @date May 23, 2005 
 */
 namespace test.sacs
 {
    public class InvalidData {
        
        private TestFramework tfw;
        private DomainParticipantFactory factory;
        private IDomainParticipant participant;
        private ITopic topic;
        private ISubscriber subscriber;
        private IMyTopicDataReader reader;
        private IPublisher publisher;
        private IMyTopicDataWriter writer1;
        private IMyTopicDataWriter writer2;
        private Time time;
        private bool proceed = true;
        private ReturnCode result;
        private MyTopic[] sampleSeq = null;
        private SampleInfo[] infoSeq = null;
        
        static int timeCompare(Time t1, Time t2)
        {
            if (t1.Sec < t2.Sec) return 1;
            if (t1.Sec > t2.Sec) return -1;
            if (t1.NanoSec < t2.NanoSec) return 1;
            if (t1.NanoSec > t2.NanoSec) return -1;
            return 0;
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
    
        
        public InvalidData() 
        {
            tfw = new TestFramework();
            time.Sec = 0;
            time.NanoSec = 0;
        }
        
        private void init()
        {
            MyTopicTypeSupport typeSupport;
            DataWriterQos wQos = null;
            string msg = "Unknown error";
            
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_000
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
             * \arg \c The MyTopicModule::MyTopic type is registered
             * \arg \c A topic T1 of type MyTopicModule::MyTopic is created with default QoS settings
             * \arg \c A DataWriter W1 for T1 with default QoS settings, writer_data_lifecycle.autodispose_unregistered_instances = FALSE
             * \arg \c A DataWriter W2 for T1 with default QoS settings, writer_data_lifecycle.autodispose_unregistered_instances = FALSE
             * \arg \c A DataReader for T1 with default QoS settings
             *
             * \e Result
             * It is expected that all entities are created/initialized correctly and without any failures. \n
             * If a failure occurs at any of the above stages, the test fails, this is reported and no further testing is performed
             */
            /*- INITIALIZATION ---------------------------------------------------------*/
            tfw.TestStart ("sacs_invalid_data_000","invalid_data","initialization");
            tfw.TestTitle ("Test initialization.");
            tfw.TestPurpose ("Test initialization.");
    
            factory = DomainParticipantFactory.Instance;
            
            if(factory == null){
                msg = "DomainParticipantFactory could NOT be resolved";
                proceed = false;
            } else {
                participant = factory.CreateParticipant(DDS.DomainId.Default);
    
                if(participant == null){
                    msg = "DomainParticipant could NOT be created";
                    proceed = false;
                } else {
                    typeSupport = new MyTopicTypeSupport();
    
                    result = typeSupport.RegisterType(participant, "MyTopic");
                    if(result == ReturnCode.Ok){
                        topic = participant.CreateTopic("my_topic", "MyTopic");
    
                        if(topic != null){
                            subscriber = participant.CreateSubscriber();
    
                            if(subscriber != null){
                                reader = subscriber.CreateDataReader(topic) as MyTopicDataReader;
    
                                if(reader != null){
    
                                    publisher = participant.CreatePublisher();
    
                                    if(publisher != null){
                                        result = publisher.GetDefaultDataWriterQos(ref wQos);
                                        if(wQos != null && result == ReturnCode.Ok){
                                            wQos.WriterDataLifecycle.AutodisposeUnregisteredInstances = false;
                                            writer1 = publisher.CreateDataWriter(topic, wQos) as MyTopicDataWriter;
                                            if(writer1 != null){
                                                writer2 = publisher.CreateDataWriter(topic, wQos) as MyTopicDataWriter;
                                                if(writer2 == null){
                                                    msg = "DataWriter could NOT be created";
                                                    proceed = false;
                                                }
                                            } else {
                                                msg = "DataWriter could NOT be created";
                                                proceed = false;
                                            }
                                        } else {
                                            reportResultCode(result);
                                            msg = "Default DataWriterQos could NOT be retrieved";
                                            proceed = false;
                                        }
                                    } else {
                                        msg = "Publisher could NOT be created";
                                        proceed = false;
                                    }
    
                                } else {
                                    msg = "DataReader could NOT be created";
                                    proceed = false;
                                }
                            } else {
                                msg = "Subscriber could NOT be created";
                                proceed = false;
                            }
                        } else {
                            msg = "Topic could NOT be created";
                            proceed = false;
                        }
                    } else {
                        msg = "Typesupport NOT loaded into DomainParticipant";
                        proceed = false;
                    }
                }
            }
            
            if(proceed == true){
                tfw.TestResult("Initialization OK", "Initialization OK", TestVerdict.Pass, TestVerdict.Pass);
                tfw.TestFinish();
            } else {
                tfw.TestResult("Initialization OK", msg, TestVerdict.Pass, TestVerdict.Fail);
                tfw.TestFinish();
            }
            /*- END OF INITIALIZATION --------------------------------------------------*/
        }
        
        private void runTests()
        {
            MyTopic userData;
            InstanceHandle writer1ID = InstanceHandle.Nil;
            InstanceHandle writer2ID = InstanceHandle.Nil;
            
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_001
             *
             * \b Test \b Objectives:
             *
             * Initialization step for the main test purpose; in next testcase
             * the id of W1 is determined through this sent sample.
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * Write a data sample (with key 1) with writer W1 as an initialization
             * step of the main test goal. This write also registers the instance.
             *
             * \e Result
             * It is expected that the sample is successfully written. \n
             * If a failure occurs, the test fails, this is reported and no further testing is performed
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_001","invalid_data","write data");
            tfw.TestTitle  ("write a sample with writer W1, which immediately also registers the instance.");
            tfw.TestPurpose("Initialization step for the main test purpose, should not fail.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                result = ReturnCode.Ok;
                userData = new MyTopic();
    
                if(userData != null){
                    time.Sec++;
                    userData.long_1 = 1;
                    userData.str = "str";
                    userData.strArr = new string[3];
                    userData.strArr[0] = "strArr0";
                    userData.strArr[1] = "strArr1";
                    userData.strArr[2] = "strArr2";
                    userData.strUBSeq = new string[1];
                    userData.strUBSeq[0] = "strUBSeq0";
                    userData.strBSeq = new string[1];
                    userData.strBSeq[0] = "strBSeq0";
    
                    result = writer1.WriteWithTimestamp(userData, InstanceHandle.Nil, time);
    
                    if(result == ReturnCode.Ok){
                        tfw.TestResult("sample successfully written", "sample successfully written", TestVerdict.Pass, TestVerdict.Pass);
                    } else {
                        tfw.TestResult("sample successfully written", "sample NOT successfully written", TestVerdict.Pass, TestVerdict.Fail);
                        proceed = false;
                    }
                } else {
                    tfw.TestResult("sample successfully written", "could not allocate sample", TestVerdict.Pass, TestVerdict.Fail);
                    proceed = false;
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_002
             *
             * \b Test \b Objectives:
             *
             * Determine writer W1's ID.
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * The reader takes the sample to determine writer W1's ID.
             *
             * \e Result
             * If the take is successful, the sample is checked to see if it matches
             * the written sample. Then writer W1's ID is recorded for later matching purposes.
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_002","invalid_data","take data");
            tfw.TestTitle  ("take data with the reader.");
            tfw.TestPurpose("determine writer W1's ID.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                result = reader.Take(
                        ref sampleSeq,
                        ref infoSeq,
                        Length.Unlimited,
                        SampleStateKind.NotRead,
                        ViewStateKind.Any,
                        InstanceStateKind.Alive);
    
                if(result == ReturnCode.Ok){
                    if(sampleSeq.Length == 1){
                        if(infoSeq[0].ValidData == true){
                            if(sampleSeq[0].long_1 == 1 ){
                                writer1ID = infoSeq[0].PublicationHandle;
                                tfw.TestResult("take sample contains the correct data", "take sample contains the correct data", TestVerdict.Pass, TestVerdict.Pass);
                            } else {
                                tfw.TestResult("take sample success", "take sample returned OK, but the sample's contents were incorrect", TestVerdict.Pass, TestVerdict.Fail);
                            }
                        } else {
                            tfw.TestResult("take sample is valid", "take sample is invalid", TestVerdict.Pass, TestVerdict.Fail);
                        }
                    } else {
                        string msg;
                        if(sampleSeq.Length > 1){
                            msg = "more than 1 sample taken";
                        } else {
                            msg = "less than 1 sample taken";
                        }
                        tfw.TestResult("1 sample successfully taken", msg, TestVerdict.Pass, TestVerdict.Fail);
                    }
                    reader.ReturnLoan(ref sampleSeq, ref infoSeq);
    
                } else {
                    reportResultCode(result);
                    tfw.TestResult("take sample success", "take sample did not return OK", TestVerdict.Pass, TestVerdict.Fail);
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
    
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_003
             *
             * \b Test \b Objectives:
             *
             * Initialization step for the main test purpose; in next testcase
             * the id of W2 is determined through this sent sample.
             *
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * Write a data sample (with key 2) with writer W2 as an initialization
             * step of the main test goal. This write implicitly also registers the instance.
             *
             * \e Result
             * It is expected that the sample is successfully written. \n
             * If a failure occurs, the test fails, this is reported and no further testing is performed
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_003","invalid_data","write data");
            tfw.TestTitle  ("write a sample with writer W2, which immediately also registers the instance.");
            tfw.TestPurpose("Initialization step for the main test purpose, should not fail.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                result = ReturnCode.Ok;
                userData = new MyTopic();
    
                if(userData != null){
                    time.Sec++;
                    userData.long_1 = 2;
                    userData.str = "str";
                    userData.strArr[0] = "strArr0";
                    userData.strArr[1] = "strArr1";
                    userData.strArr[2] = "strArr2";
                    userData.strUBSeq = new string[0];
                    userData.strBSeq = new string[0];
    
                    result = writer2.WriteWithTimestamp(userData, InstanceHandle.Nil, time);
    
                    if(result == ReturnCode.Ok){
                        tfw.TestResult("sample successfully written", "sample successfully written", TestVerdict.Pass, TestVerdict.Pass);
                    } else {
                        tfw.TestResult("sample successfully written", "sample NOT successfully written", TestVerdict.Pass, TestVerdict.Fail);
                        proceed = false;
                    }
                } else {
                    tfw.TestResult("sample successfully written", "could not allocate sample", TestVerdict.Pass, TestVerdict.Fail);
                    proceed = false;
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
    
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_004
             *
             * \b Test \b Objectives:
             *
             * Determine writer W2's ID.
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * The reader takes the sample to determine writer W2's ID.
             *
             * \e Result
             * If the take is successful, the sample is checked to see if it matches
             * the written sample. Then writer W2's ID is recorded for later matching purposes.
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_004","invalid_data","take data");
            tfw.TestTitle  ("take data with the reader.");
            tfw.TestPurpose("determine writer W2's ID.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                result = reader.Take(
                        ref sampleSeq,
                        ref infoSeq,
                        Length.Unlimited,
                        SampleStateKind.NotRead,
                        ViewStateKind.Any,
                        InstanceStateKind.Any);
    
                if(result == ReturnCode.Ok){
                    if(sampleSeq.Length == 1){
                        if(infoSeq[0].ValidData == true){
                            if(sampleSeq[0].long_1 == 2 ){
                                writer2ID = infoSeq[0].PublicationHandle;
                                if(writer1ID != writer2ID){
                                    tfw.TestResult("take sample contains the correct data", "take sample contains the correct data", TestVerdict.Pass, TestVerdict.Pass);
                                } else {
                                    tfw.TestResult("writer2ID differs from writer1ID", "writer1ID and writer2ID are equal", TestVerdict.Pass, TestVerdict.Fail);
                                }
                            } else {
                                tfw.TestResult("take sample success", "take sample returned OK, but the sample's contents were incorrect", TestVerdict.Pass, TestVerdict.Fail);
                            }
                        } else {
                            tfw.TestResult("take sample is valid", "take sample is invalid", TestVerdict.Pass, TestVerdict.Fail);
                        }
                    } else {
                        string msg;
                        if(sampleSeq.Length > 1){
                            msg = "more than 1 sample taken";
                        } else {
                            msg = "less than 1 sample taken";
                        }
                        tfw.TestResult("1 sample successfully taken", msg, TestVerdict.Pass, TestVerdict.Fail);
                    }
    
                    reader.ReturnLoan(ref sampleSeq, ref infoSeq);
                } else {
                    reportResultCode(result);
                    tfw.TestResult("take sample success", "take sample did not return OK", TestVerdict.Pass, TestVerdict.Fail);
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
    
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_005
             *
             * \b Test \b Objectives:
             *
             * Check validity of sampleinfo data when a invalid sample is sent after a
             * dispose.
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * Dispose sample 1 with writer W2, check the invalid sample received
             * by the reader from which writer it came and its source_timestamp.
             *
             * \e Result
             * It is expected that the publication_handle of the dispose sample points to writer W2. \n
             * If a failure occurs, the test fails.
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_005","invalid_data","dispose data");
            tfw.TestTitle  ("dispose sample 1 with writer W2, check publication_handle and source timestamp");
            tfw.TestPurpose("The publication_handle and source timestamp should be that of writer W2.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                result = ReturnCode.Ok;
                userData = new MyTopic();
    
                if(userData != null){
                    time.Sec++;
                    int expectedSec = time.Sec;
                    userData.long_1 = 1;
                    userData.str = "str";
                    userData.strArr[0] = "strArr0";
                    userData.strArr[1] = "strArr1";
                    userData.strArr[2] = "strArr2";
                    userData.strUBSeq = new string[0];
                    userData.strBSeq = new string[0];
    
                    result = writer2.DisposeWithTimestamp(userData, InstanceHandle.Nil, time);
    
                    if(result == ReturnCode.Ok){
                        result = reader.Take(
                                ref sampleSeq,
                                ref infoSeq,
                                Length.Unlimited,
                                SampleStateKind.NotRead,
                                ViewStateKind.Any,
                                InstanceStateKind.Any);
    
                        if(result == ReturnCode.Ok){
                            if(sampleSeq.Length == 1){
                                if(sampleSeq[0].long_1 == 1){
                                    if(infoSeq[0].ValidData == false){
                                        if(infoSeq[0].InstanceState == InstanceStateKind.NotAliveDisposed){
                                            if(infoSeq[0].PublicationHandle == writer2ID){
                                                if(infoSeq[0].SourceTimestamp.Sec == expectedSec
                                                        && infoSeq[0].SourceTimestamp.NanoSec == 0){
                                                    tfw.TestResult("publication_handle and source time stamp are correct", "publication_handle and source time stamp are correct", TestVerdict.Pass, TestVerdict.Pass);
                                                } else {
                                                    tfw.TestResult("source time stamp is correct", "source time stamp is incorrect", TestVerdict.Pass, TestVerdict.Fail);
                                                }
                                            } else {
                                                tfw.TestResult("publication_handle is correct", "incorrect publication_handle", TestVerdict.Pass, TestVerdict.Fail);
                                            }
                                        } else {
                                            tfw.TestResult("instance_state is disposed", "instance_state is NOT disposed", TestVerdict.Pass, TestVerdict.Fail);
                                        }
                                    } else {
                                        tfw.TestResult("take sample success", "expected invalid data, received valid data", TestVerdict.Pass, TestVerdict.Fail);
                                    }
                                } else {
                                    tfw.TestResult("take sample success", "sample contains incorrect data", TestVerdict.Pass, TestVerdict.Fail);
                                }
                            } else {
                                tfw.TestResult("take sample success", "incorrect amount of data taken", TestVerdict.Pass, TestVerdict.Fail);
                            }
                            reader.ReturnLoan(ref sampleSeq, ref infoSeq);
                        } else {
                            reportResultCode(result);
                            tfw.TestResult("take sample success", "take did not return OK", TestVerdict.Pass, TestVerdict.Fail);
                        }
                    } else {
                        tfw.TestResult("sample successfully written", "sample NOT successfully written", TestVerdict.Pass, TestVerdict.Fail);
                        proceed = false;
                    }
                } else {
                    tfw.TestResult("sample successfully written", "could not allocate sample", TestVerdict.Pass, TestVerdict.Fail);
                    proceed = false;
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
    
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_006
             *
             * \b Test \b Objectives:
             *
             * Check validity of sampleinfo data when a invalid sample is sent after an
             * unregister.
             *
             * \b Test \b Procedure:
             *
             * \e Action
             *
             * First register (via write) and then unregister sample 2 with writer W1,
             * check the invalid sample received by the reader from which writer
             * it came and its source_timestamp. After the first register of W1, unregister
             * W2 on sample 2, so that only W1 is registered for this instance.
             *
             * \e Result
             * It is expected that the publication_handle of the unregister sample points to writer W2. \n
             * If a failure occurs, the test fails.
             */
            /*--------------------------------------------------------------------------*/
            tfw.TestStart  ("sacs_invalid_data_006","invalid_data","dispose data");
            tfw.TestTitle  ("unregister sample 2 with writer W1, check publication_handle and source timestamp");
            tfw.TestPurpose("The publication_handle and source timestamp should be that of writer W1.");
            /*--------------------------------------------------------------------------*/
    
            if(proceed == true){
                int expectedSec = -1;
                result = ReturnCode.Ok;
                userData = new MyTopic();
    
                if(userData != null){
                    time.Sec++;
    
                    userData.long_1 = 2;
                    userData.str = "str";
                    userData.strArr[0] = "strArr0";
                    userData.strArr[1] = "strArr1";
                    userData.strArr[2] = "strArr2";
                    userData.strUBSeq = new string[0];
                    userData.strBSeq = new string[0];
    
                    result = writer1.WriteWithTimestamp(userData, InstanceHandle.Nil, time);
    
                    if(result == ReturnCode.Ok){
                        time.Sec++;
    
                        result = writer2.UnregisterInstanceWithTimestamp(userData, InstanceHandle.Nil, time);
                        if(result == ReturnCode.Ok){
                            /* reader take */
                            result = reader.Take(
                                            ref sampleSeq, ref infoSeq, 
                                            Length.Unlimited,
                                            SampleStateKind.Any,
                                            ViewStateKind.Any,
                                            InstanceStateKind.Any);
                            if(result == ReturnCode.Ok){
                                time.Sec++;
                                expectedSec = time.Sec;
    
                                reader.ReturnLoan(ref sampleSeq, ref infoSeq);
    
                                userData.long_1 = 2;
                                result = writer1.UnregisterInstanceWithTimestamp(userData, InstanceHandle.Nil, time);
                                if(result != ReturnCode.Ok){
                                    reportResultCode(result);
                                    tfw.TestMessage(TestMessage.Error, "unregister did not return OK");
                                    proceed = false;
                                }
    
                            } else {
                                reportResultCode(result);
                                tfw.TestMessage(TestMessage.Error, "take did not return OK");
                                proceed = false;
                            }
                        } else {
                            reportResultCode(result);
                            tfw.TestMessage(TestMessage.Error, "unregister of sample 2 by writer 2 did not return OK");
                            proceed = false;
                        }
                    } else {
                        reportResultCode(result);
                        tfw.TestMessage(TestMessage.Error, "write did not return OK");
                        proceed = false;
                    }
    
                    if(proceed == true){
                        if(result == ReturnCode.Ok){
                            result = reader.Take(
                                    ref sampleSeq,
                                    ref infoSeq,
                                    Length.Unlimited,
                                    SampleStateKind.Any,
                                    ViewStateKind.Any,
                                    InstanceStateKind.Any);
    
                            if(result == ReturnCode.Ok){
                                if(sampleSeq.Length == 1){
                                    if(sampleSeq[0].long_1 == 2){
                                        if(infoSeq[0].ValidData == false){
                                            if(infoSeq[0].InstanceState == InstanceStateKind.NotAliveNoWriters){
                                                if(infoSeq[0].PublicationHandle == writer1ID){
                                                    if(expectedSec >= 0 && infoSeq[0].SourceTimestamp.Sec == expectedSec
                                                            && infoSeq[0].SourceTimestamp.NanoSec == 0){
                                                        tfw.TestResult("publication_handle and source time stamp are correct", "publication_handle and source time stamp are correct", TestVerdict.Pass, TestVerdict.Pass);
                                                    } else {
                                                        tfw.TestResult("source time stamp is correct", "source time stamp is incorrect", TestVerdict.Pass, TestVerdict.Fail);
                                                    }
                                                } else {
                                                    tfw.TestResult("publication_handle is correct", "incorrect publication_handle", TestVerdict.Pass, TestVerdict.Fail);
                                                }
                                            } else {
                                                tfw.TestResult("instance_state is disposed", "instance_state is NOT disposed", TestVerdict.Pass, TestVerdict.Fail);
                                            }
                                        } else {
                                            tfw.TestResult("take sample success", "expected invalid data, received valid data", TestVerdict.Pass, TestVerdict.Fail);
                                        }
                                    } else {
                                        tfw.TestResult("take sample success", "sample contains incorrect data", TestVerdict.Pass, TestVerdict.Fail);
                                    }
                                } else {
                                    tfw.TestResult("take sample success", "incorrect amount of data taken", TestVerdict.Pass, TestVerdict.Fail);
                                }
                                reader.ReturnLoan(ref sampleSeq, ref infoSeq);
                            } else {
                                reportResultCode(result);
                                tfw.TestResult("take sample success", "take did not return OK", TestVerdict.Pass, TestVerdict.Fail);
                            }
                        } else {
                            reportResultCode(result);
                            tfw.TestResult("sample successfully written", "sample NOT successfully written", TestVerdict.Pass, TestVerdict.Fail);
                            proceed = false;
                        }
                    } else {
                        reportResultCode(result);
                        tfw.TestResult("sample successfully written", "preliminary step failed", TestVerdict.Pass, TestVerdict.Fail);
                        proceed = false;
                    }
                } else {
                    tfw.TestResult("sample successfully written", "could not allocate sample", TestVerdict.Pass, TestVerdict.Fail);
                    proceed = false;
                }
            } else {
                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
            }
    
            tfw.TestFinish();
    
//            /**
//             * @addtogroup group_dds1290
//             *
//             * \b Test \b ID: \b sacs_invalid_data_007
//             *
//             * \b Test \b Objectives:
//             *
//             * Check validity of sampleinfo data when invalid samples are caused by a
//             * call to dispose_all_data.
//             *
//             * \b Test \b Procedure:
//             *
//             * \e Action
//             *
//             * First write sample 2 and 3 with writer W1, then check the invalid samples
//             * received by the reader, from which writer it came, and its source_timestamp.
//             *
//             * \e Result
//             * It is expected that the publication_handles of the dispose_all samples are NIL,
//             * and that the timestamp should be the bigger than the current time just before
//             * the dispose_all_data event was performed.\n
//             * If a failure occurs, the test fails.
//             */
//            /*--------------------------------------------------------------------------*/
//            tfw.TestStart  ("sacs_invalid_data_007","invalid_data","dispose_all_data");
//            tfw.TestTitle  ("dispose samples 2 and 3 with dispose_all_data, check publication_handle and source timestamp");
//            tfw.TestPurpose("The publication_handle be NIL, the timestamp >= current time just before dispose_all");
//            /*--------------------------------------------------------------------------*/
//    
//            if(proceed == true){
//                result = ReturnCode.Ok;
//                userData = new MyTopic();
//    
//                if(userData != null){
//                    time.Sec++;
//    
//                    userData.long_1 = 2;
//                    userData.str = "str";
//                    userData.strArr[0] = "strArr0";
//                    userData.strArr[1] = "strArr1";
//                    userData.strArr[2] = "strArr2";
//                    userData.strUBSeq = new string[0];
//                    userData.strBSeq = new string[0];
//    
//                    result = writer1.write_w_timestamp(userData, InstanceHandle.Nil, time);
//                    if(result == ReturnCode.Ok){
//                        userData.long_1 = 3;
//                        time.Sec++;
//                        result = writer1.write_w_timestamp(userData, InstanceHandle.Nil, time);
//        
//                        if(result == ReturnCode.Ok){
//                            /* reader take */
//                            result = reader.Take(
//                                            sampleSeq, 
//                                            infoSeq, 
//                                            LENGTH_UNLIMITED, 
//                                            ANY_SAMPLE_STATE, 
//                                            ANY_VIEW_STATE, 
//                                            ANY_INSTANCE_STATE);
//                            if(result == ReturnCode.Ok){
//                                Time_tHolder tHolder = new Time_tHolder();
//        
//                                if (sampleSeq.Length != 2 || sampleSeq[0].long_1 != 2 || sampleSeq[1].long_1 != 3)
//                                {
//                                    proceed = false;
//                                    tfw.TestMessage(TestMessage.Error, "Take for valid samples did not return the correct samples.");
//                                }
//                                reader.ReturnLoan(sampleSeq, infoSeq);
//        
//                                participant.get_current_time(tHolder);
//                                time = tHolder;
//                                result = topic.dispose_all_data();
//                                if(result != ReturnCode.Ok){
//                                    reportResultCode(result);
//                                    tfw.TestMessage(TestMessage.Error, "unregister did not return OK");
//                                    proceed = false;
//                                }
//        
//                            } else {
//                                reportResultCode(result);
//                                tfw.TestMessage(TestMessage.Error, "take did not return OK");
//                                proceed = false;
//                            }
//                        } else {
//                            reportResultCode(result);
//                            tfw.TestMessage(TestMessage.Error, "2nd write did not return OK");
//                            proceed = false;
//                        }
//                    } else {
//                        reportResultCode(result);
//                        tfw.TestMessage(TestMessage.Error, "1st write did not return OK");
//                        proceed = false;
//                    }
//    
//                    if(proceed == true){
//                        /* Sleep for 5 seconds to give spliced the time to dispose the samples. */
//                        Thread.sleep(5000);
//                        if(result == ReturnCode.Ok){
//                            result = reader.Take(
//                                    sampleSeq,
//                                    infoSeq,
//                                    LENGTH_UNLIMITED,
//                                    ANY_SAMPLE_STATE,
//                                    ANY_VIEW_STATE,
//                                    NOT_ALIVE_DISPOSED_INSTANCE_STATE);
//    
//                            if(result == ReturnCode.Ok){
//                                if(sampleSeq.Length == 2){
//                                    if(sampleSeq[0].long_1 == 2 && sampleSeq[1].long_1 == 3){
//                                        if(infoSeq[0].ValidData == false && infoSeq[1].ValidData == false){
//                                            if(infoSeq[0].InstanceState == NOT_ALIVE_DISPOSED_INSTANCE_STATE &&
//                                                    infoSeq[1].InstanceState == NOT_ALIVE_DISPOSED_INSTANCE_STATE){
//                                                if(infoSeq[0].PublicationHandle == InstanceHandle.Nil &&
//                                                        infoSeq[1].PublicationHandle == InstanceHandle.Nil){
//                                                    if(timeCompare(time, infoSeq[0].SourceTimestamp) > 0 &&
//                                                            infoSeq[0].SourceTimestamp.Sec == infoSeq[1].SourceTimestamp.Sec &&
//                                                            infoSeq[0].SourceTimestamp.NanoSec == infoSeq[1].SourceTimestamp.NanoSec) {
//                                                        tfw.TestResult("publication_handles and source time stamps are correct", "publication_handles and source time stamps are correct", TestVerdict.Pass, TestVerdict.Pass);
//                                                    } else {
//                                                        tfw.TestResult("source time stamps are correct", "source time stamps are incorrect", TestVerdict.Pass, TestVerdict.Fail);
//                                                    }
//                                                } else {
//                                                    tfw.TestResult("publication_handles are correct", "incorrect publication_handles", TestVerdict.Pass, TestVerdict.Fail);
//                                                }
//                                            } else {
//                                                tfw.TestResult("instance_state is disposed", "instance_state is NOT disposed", TestVerdict.Pass, TestVerdict.Fail);
//                                            }
//                                        } else {
//                                            tfw.TestResult("take sample success", "expected invalid data, received valid data", TestVerdict.Pass, TestVerdict.Fail);
//                                        }
//                                    } else {
//                                        tfw.TestResult("take sample success", "samples contain incorrect data", TestVerdict.Pass, TestVerdict.Fail);
//                                    }
//                                } else {
//                                    tfw.TestResult("take sample success", "incorrect amount of data taken", TestVerdict.Pass, TestVerdict.Fail);
//                                }
//                                reader.ReturnLoan(sampleSeq, infoSeq);
//                            } else {
//                                reportResultCode(result);
//                                tfw.TestResult("take sample success", "take did not return OK", TestVerdict.Pass, TestVerdict.Fail);
//                            }
//                        } else {
//                            reportResultCode(result);
//                            tfw.TestResult("sample successfully written", "sample NOT successfully written", TestVerdict.Pass, TestVerdict.Fail);
//                            proceed = false;
//                        }
//                    } else {
//                        reportResultCode(result);
//                        tfw.TestResult("sample successfully written", "preliminary step failed", TestVerdict.Pass, TestVerdict.Fail);
//                        proceed = false;
//                    }
//                } else {
//                    tfw.TestResult("sample successfully written", "could not allocate sample", TestVerdict.Pass, TestVerdict.Fail);
//                    proceed = false;
//                }
//            } else {
//                tfw.TestResult("OK", "Unresolved", TestVerdict.Pass, TestVerdict.Unresolved);
//            }
//    
//            tfw.TestFinish();
//    
        }
        
        private void deinit()
        {
            /**
             * @addtogroup group_dds1290
             *
             * \b Test \b ID: \b sacs_invalid_data_999
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
    
                    if(result == ReturnCode.Ok){
                       factory.DeleteParticipant(participant);
                       if(result == ReturnCode.Ok){
                           tfw.TestResult("Deinitialization OK", "Deinitialization OK", TestVerdict.Pass, TestVerdict.Pass);
                       } else {
                        reportResultCode(result);
                        tfw.TestResult("Deinitialization OK", "Deinitialization FAIL", TestVerdict.Pass, TestVerdict.Fail);
                       }
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