#ifndef OSPL_TESTS_CPP_INFO_VALUES_CHECKER_H
#define OSPL_TESTS_CPP_INFO_VALUES_CHECKER_H

#include "testlibs/CPPTestProcess.h"

#include "ccpp_dds_dcps.h"
#include "ccpp_Space.h"

#include "ext_participant_listener_impl.h"

class CPPProc2
  : public virtual OSPLTestLib::CPPTestProcess
{
    public:
        CPPProc2(const char* the_name) :
          CPPTestProcess (the_name) {}

       virtual ~CPPProc2() {};

    // virtual void init (int argc, char *argv[])
    //  {
    //    OSPLTestLib::CPPTestProcess::init (argc, argv);
    //
    //    // insert any initialisation you require here.
    //}

    
    virtual int run ()
    {
        DDS::ReturnCode_t result;

        ExtParticipantListenerImpl* dpl = new  ExtParticipantListenerImpl();
        
        ::DDS::DomainParticipantFactory_var factory
            = ::DDS::DomainParticipantFactory::get_instance ();

        ::DDS::DomainParticipant_var domain_participant
                = factory->create_participant (this->default_domain_id_,
                                              PARTICIPANT_QOS_DEFAULT,
                                              dpl,
                                              DDS::STATUS_MASK_ANY_V1_2 | DDS::ALL_DATA_DISPOSED_TOPIC_STATUS);


        if (::DDS::is_nil (domain_participant.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil domain participant - daemon probably not running.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::Space::Type1TypeSupport type1_type_support;

        result = type1_type_support.register_type (domain_participant.in (),
                                                   type1_type_support.get_type_name ());

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Register type failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::Topic_var type1_topic
                = domain_participant->create_topic ("dds1619_multinode_A",
                                                     type1_type_support.get_type_name (),
                                                     TOPIC_QOS_DEFAULT,
                                                     ::DDS::TopicListener::_nil (),
                                                     0U);

        if (::DDS::is_nil (type1_topic.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil topic.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }
        
        
        
        ::DDS::Publisher_var publisher
                 = domain_participant->create_publisher (PUBLISHER_QOS_DEFAULT,
                                                        ::DDS::PublisherListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (publisher.in ())) 
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil publisher.\n", 
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataWriter_var generic_writer 
                = publisher->create_datawriter (type1_topic,
                                                      DATAWRITER_QOS_DEFAULT, 
                                                      ::DDS::DataWriterListener::_nil (), 
                                                      0U);

        Space::Type1DataWriter_var type1_writer
                = Space::Type1DataWriter::_narrow (generic_writer.in ());         
                

        if (::DDS::is_nil (type1_writer.in ())) 
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil writer.\n", 
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }
        

        ::DDS::Subscriber_var subscriber
                = domain_participant->create_subscriber (SUBSCRIBER_QOS_DEFAULT,
                                                        ::DDS::SubscriberListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (subscriber.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil subscriber.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataReader_var generic_reader
                = subscriber->create_datareader (type1_topic,
                                                      DATAREADER_QOS_DEFAULT,
                                                      ::DDS::DataReaderListener::_nil (),
                                                      0U);

        Space::Type1DataReader_var type1_reader
                = Space::Type1DataReader::_narrow (generic_reader.in ());

        if (::DDS::is_nil (type1_reader.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil reader.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }
        
        // Tell process 1 that we are initialized
        this->process_controller_->set_process_status (1, this->other_process_name_.in ());
 
        fprintf (stdout, "Test purpose : multiple calls to dispose_all_data from different nodes : \n-do we get correct number of callbacks? (Use case specification 2) \n");
        fprintf (stdout, "-is data correctly observed to be disposed on the local node? (Use case specification 3) \n");
        fprintf (stdout, "-is data correctly observed to be disposed on the remote node? (Use case specification 3) \n");
        
        
        for(int i = 2; i < 12; ++i)
        {
            if(i%2 == 1)
            {
                // My turn to write and dispose
                Space::Type1 type1_sample;
                type1_sample.long_1 = 1;
                type1_sample.long_2 = 2;
                type1_sample.long_3 = 3;
        
                result = type1_writer->write (type1_sample, 
                                              ::DDS::HANDLE_NIL);
                
                if (result != ::DDS::RETCODE_OK) 
                {
                    fprintf (stderr, "ERROR %s (%s - %d): Write failed.\n", 
                                      process_name_.in (), __FILE__, __LINE__);
                    return -1;
                }
                
                result = type1_topic->dispose_all_data();
                
                if (result != ::DDS::RETCODE_OK) 
                {
                    fprintf (stderr, "ERROR %s (%s - %d): Dispose_all_data failed.\n", 
                                      process_name_.in (), __FILE__, __LINE__);
                    return -1;
                }
            }
        
            DDS::Duration_t timeout;
            timeout.sec = 10;
            timeout.nanosec = 0;
            result = dpl->wait_for_on_all_data_disposed(timeout);
            if( result != ::DDS::RETCODE_OK ) {
                fprintf(stderr, "ERROR %s (%s - %d): on_all_data_disposed callback on participant listener has not been called before TIMEOUT.\n", 
                                      process_name_.in (), __FILE__, __LINE__);
                return -1;
            }
        
            Space::Type1Seq samples;
            ::DDS::SampleInfoSeq_var sample_info_seq = new ::DDS::SampleInfoSeq ();
            
            result = type1_reader->take (samples,
                                         sample_info_seq,
                                         ::DDS::LENGTH_UNLIMITED, 
                                         ::DDS::ANY_SAMPLE_STATE, 
                                         ::DDS::ANY_VIEW_STATE, 
                                         ::DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE);
    
            if (result != ::DDS::RETCODE_OK)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Take failed.\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }
            
            if(sample_info_seq->length() != 1)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Took %d samples instead of 1 expected.\n",
                                  process_name_.in (), __FILE__, __LINE__, sample_info_seq->length());
                return -1;
            }
    
            if(!sample_info_seq[0].valid_data)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Took invalid data (expected valid - reverse this test if dds751 is implemented).\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }
            
            dpl->reset();
            
            // tell process 1 that we received all_data_diposed call back or timeout.
            this->process_controller_->set_process_status (i, this->other_process_name_.in ());
        }

        fprintf (stdout, "OK \n");
        

        // Success
        return 0;
    }
    
    void terminate ()
    {
        // shutdown process1
        this->process_controller_->shutdown (this->other_process_name_.in ());
    }


};

#endif /* OSPL_TESTS_CPP_INFO_VALUES_CHECKER_H */

