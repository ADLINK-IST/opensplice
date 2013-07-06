#ifndef OSPL_TESTS_CPP_PUBLISHER_H
#define OSPL_TESTS_CPP_PUBLISHER_H

#include "testlibs/CPPTestProcess.h"

#include "ccpp_TestTopics.h"
#include "ccpp_dds_dcps.h"

/**
* @brief A very simple publish process for a very very simple
* publish / subscribe test.
* The publisher publishes a sample. The subscriber reads it.
*/
class CPPPublisherProcess
  : public virtual OSPLTestLib::CPPTestProcess
{
    public:
        CPPPublisherProcess (const char* the_name) :
          CPPTestProcess (the_name) {}

         virtual ~CPPPublisherProcess () {};
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
    * The test creates a simplest possible writer with all default QoS. 
    * Waits until notified to start by the other process.
    * Writes a sample and waits to be shutdown by the other process.
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
        
        ::TestModule::HelloTypeSupport hello_type_support;

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

        ::DDS::Publisher_var hello_publisher
                = domain_participant->create_publisher (PUBLISHER_QOS_DEFAULT,
                                                        ::DDS::PublisherListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (hello_publisher.in ())) 
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil publisher.\n", 
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataWriter_var generic_writer 
                = hello_publisher->create_datawriter (hello_topic,
                                                      DATAWRITER_QOS_DEFAULT, 
                                                      ::DDS::DataWriterListener::_nil (), 
                                                      0U);

        TestModule::HelloDataWriter_var hello_writer
                = TestModule::HelloDataWriter::_narrow (generic_writer.in ());                

        if (::DDS::is_nil (hello_writer.in ())) 
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil writer.\n", 
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Wait until the other process sets our status to 1 to indicate it's
        // ready for us to start writing
        this->process_controller_->wait_for_process_state (1);

        TestModule::Hello hello_sample;
        hello_sample.the_message = ::CORBA::string_dup ("Hello");

        result = hello_writer->write (hello_sample, 
                                      ::DDS::HANDLE_NIL);

        
        if (result != ::DDS::RETCODE_OK) 
        {
            fprintf (stderr, "ERROR %s (%s - %d): Write failed.\n", 
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Park the thread to wait for the publisher to shut us down after 
        // it has read the sample
        this->process_controller_->park_thread_to_do_work ();

        return 0;
    }


};

#endif /* OSPL_TESTS_CPP_PUBLISHER_H */
