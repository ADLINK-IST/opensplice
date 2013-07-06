#ifndef OSPL_TESTS_CPP_PUBLISHER_H
#define OSPL_TESTS_CPP_PUBLISHER_H

#include "testlibs/CPPTestProcess.h"

#include "ccpp_TestTopics.h"
#include "ccpp_dds_dcps.h"

/**
* @brief A very simple subscriber process for a very very simple
* publish / subscribe test.
* The subscriber publishes a sample. This subscriber reads it.
*/
class CPPSubscriberProcess
  : public virtual OSPLTestLib::CPPTestProcess
{
    public:
        CPPSubscriberProcess (const char* the_name) :
          CPPTestProcess (the_name) {}

       virtual ~CPPSubscriberProcess () {};
    /**
    * @brief test process initialisation
    * Uncomment and doc *iff* required.
    */
    // virtual void init (int argc, char *argv[])
    //  {
    //    OSPLTestLib::CPPTestProcess::init (argc, argv);
    //
    //    // insert any initialisation you require here.
    //}

    /**
    * @brief the test procedure
    * The test creates a simplest possible reader with all default QoS.
    * It the notifies the other (subscriber) porcess by setting its status to 1.
    * It reads a sample from the reader and then tells the subscriber to shutdown
    * and shuts down itself.
    */
    virtual int run ()
    {
        DDS::ReturnCode_t result;

        ::DDS::DomainParticipantFactory_var factory
            = ::DDS::DomainParticipantFactory::get_instance ();

        ::DDS::DomainParticipant_var domain_participant
                = factory->create_participant (this->default_domain_id_,
                                              PARTICIPANT_QOS_DEFAULT,
                                              ::DDS::DomainParticipantListener::_nil (),
                                              0UL);


        if (::DDS::is_nil (domain_participant.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil domain participant - daemon probably not running.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        TestModule::HelloTypeSupport hello_type_support;

        result = hello_type_support.register_type (domain_participant.in (),
                                                   hello_type_support.get_type_name ());

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Register type failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::Topic_var hello_topic
                = domain_participant->create_topic ("HelloTopic",
                                                     hello_type_support.get_type_name (),
                                                     TOPIC_QOS_DEFAULT,
                                                     ::DDS::TopicListener::_nil (),
                                                     0U);

        if (::DDS::is_nil (hello_topic.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil topic.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::Subscriber_var hello_subscriber
                = domain_participant->create_subscriber (SUBSCRIBER_QOS_DEFAULT,
                                                        ::DDS::SubscriberListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (hello_subscriber.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil subscriber.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataReader_var generic_reader
                = hello_subscriber->create_datareader (hello_topic,
                                                      DATAREADER_QOS_DEFAULT,
                                                      ::DDS::DataReaderListener::_nil (),
                                                      0U);

        TestModule::HelloDataReader_var hello_reader
                = TestModule::HelloDataReader::_narrow (generic_reader.in ());

        if (::DDS::is_nil (hello_reader.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil reader.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Create a data available status condition fron the reader
        ::DDS::StatusCondition_var reader_status_condition 
                = hello_reader->get_statuscondition ();
        reader_status_condition->set_enabled_statuses (::DDS::DATA_AVAILABLE_STATUS);
        
        // Attach it to a wait set
        ::DDS::WaitSet wait_set;
        wait_set.attach_condition (reader_status_condition);

        ::DDS::ConditionSeq_var  condition_list_out = new ::DDS::ConditionSeq ();

        // Set the other process status to 1 to indicate it can start writing now
        this->process_controller_->set_process_status (1, this->other_process_name_.in ());

        result = wait_set.wait (condition_list_out.inout (), ::DDS::DURATION_INFINITE);

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Wait failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        if (condition_list_out->length () != 1
            || condition_list_out[0U].in () != reader_status_condition.in ())
        {
            fprintf (stderr, "ERROR %s (%s - %d): Unexpected status condition value.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        TestModule::HelloSeq hello_samples;
        ::DDS::SampleInfoSeq_var sample_info_seq = new ::DDS::SampleInfoSeq ();
        
        result = hello_reader->take (hello_samples,
                                     sample_info_seq,
                                     ::DDS::LENGTH_UNLIMITED, 
                                     ::DDS::ANY_SAMPLE_STATE, 
                                     ::DDS::ANY_VIEW_STATE, 
                                     ::DDS::ANY_INSTANCE_STATE);

        fprintf (stdout, "The publisher says: %s.\n", hello_samples[0].the_message.in ());
        
        hello_reader->return_loan (hello_samples, sample_info_seq);

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Read failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Shutdown the publisher and then exit
        this->process_controller_->shutdown (this->other_process_name_.in ());

        // Success
        return 0;
    }


};

#endif /* OSPL_TESTS_CPP_PUBLISHER_H */

