/** \file
 * \brief File that defines the subscriber.
 *
 */

#ifndef _DDS2466_SUBSCRIBER_H_
#define _DDS2466_SUBSCRIBER_H_

// How many messages the subscriber will wait for:
#define MSG_COUNT 20
#define INS_COUNT 10

#include "ccpp_dds_dcps.h"
#include "../testlibs/CPPTestProcess.h"
#include "ccpp_dds2466.h"

/**
* \class DDS2466Subscriber
* \brief A subscriber process for the test.
* The subscriber just receives defined amount of samples.
*/
class DDS2466Subscriber: public virtual OSPLTestLib::CPPTestProcess
{
    // Public methods:
    public:
        //----------------------------------------------------------------------
        /**
         * \brief Constructs the object instance with the defined name.
         * \param[in] process_name - the name for this process.
         * Need for the process synchronization.
         */
        DDS2466Subscriber(const char* process_name):
            CPPTestProcess(process_name),
            id(0)
        {}//DDS2466Subscriber
        //----------------------------------------------------------------------
        /**
         * \brief Object destructor.
         */
        virtual ~DDS2466Subscriber()
        {}//~DDS2466Subscriber
        //----------------------------------------------------------------------
        /**
         * \brief Initialize the object instance with the defined name.
         * Sets subscriber ID using system variable SUB_ID.
         * \param[in] argc - arguments count.
         * \param[in] argv - arguments list.
         */
        virtual void init(int argc, char *argv[])
        {
            OSPLTestLib::CPPTestProcess::init(argc, argv);

            char* sub_id = getenv("SUB_ID");
            if (sub_id != NULL)
            {
                this->id = atoi(sub_id);
            }
        }//init
        //----------------------------------------------------------------------
        /**
         * \brief Starts writing.
         * \return 0 - in case of success, -1 - in case of any failure.
         */
        virtual int run()
        {
            // Operation result:
            DDS::ReturnCode_t result;
            DDS::TopicQos topic_qos;

            ::DDS::DomainParticipantFactory_var factory =
                ::DDS::DomainParticipantFactory::get_instance();

            ::DDS::DomainParticipant_var domain_participant =
                factory->create_participant(
                    this->default_domain_id_,
                    PARTICIPANT_QOS_DEFAULT,
                    ::DDS::DomainParticipantListener::_nil(),
                    0UL);
            if (::DDS::is_nil(domain_participant.in()))
            {
                std::cerr << "FAIL: Can't create domain participant instance!" << std::endl;
                return -1;
            }

            ::Test2466::SampleTypeSupport type_support;
            result = type_support.register_type(domain_participant.in(),
                                                type_support.get_type_name());
            if (result != ::DDS::RETCODE_OK)
            {
                std::cerr << "FAIL: Can't register the topic type!" << std::endl;
                return -1;
            }
            result = domain_participant->get_default_topic_qos(topic_qos);
            if (result != ::DDS::RETCODE_OK)
            {
                std::cerr << "FAIL: Can't get default topic qos!" << std::endl;
                return -1;
            }
            topic_qos.destination_order.kind = DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
            ::DDS::Topic_var topic = domain_participant->create_topic(
                "TEST_DDS2466_TOPIC",
                type_support.get_type_name(),
                topic_qos,
                ::DDS::TopicListener::_nil(),
                0U);
            if (::DDS::is_nil(topic.in()))
            {
                std::cerr << "FAIL: Can't create the topic!" << std::endl;
                return -1;
            }

            ::DDS::Subscriber_var subscriber =
                domain_participant->create_subscriber(
                    SUBSCRIBER_QOS_DEFAULT,
                    ::DDS::SubscriberListener::_nil(),
                    0U);
            if (::DDS::is_nil(subscriber.in()))
            {
                std::cerr << "FAIL: Can't create the publisher!" << std::endl;
                return -1;
            }

            DDS::DataReaderQos dr_qos = DATAREADER_QOS_DEFAULT;
            dr_qos.durability.kind = ::DDS::PERSISTENT_DURABILITY_QOS;
            dr_qos.destination_order.kind = ::DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
            dr_qos.history.depth = 2;

            ::DDS::DataReader_var generic_reader =
                subscriber->create_datareader(
                    topic,
                    dr_qos,
                    ::DDS::DataReaderListener::_nil(),
                    0U);
            Test2466::SampleDataReader_var sample_reader =
                Test2466::SampleDataReader::_narrow(generic_reader.in());
            if (::DDS::is_nil(sample_reader.in()))
            {
                std::cerr << "FAIL: Can't create the data reader!" << std::endl;
                return -1;
            }

            ::DDS::DataReader_var generic_reader2 =
                subscriber->create_datareader(
                    topic,
                    dr_qos,
                    ::DDS::DataReaderListener::_nil(),
                    0U);

            Test2466::SampleDataReader_var sample_reader2 =
                Test2466::SampleDataReader::_narrow(generic_reader2.in());
            if (::DDS::is_nil(sample_reader.in()))
            {
                std::cerr << "FAIL: Can't create the data reader!" << std::endl;
                return -1;
            }

            ::DDS::StatusCondition_var reader_status_condition =
                sample_reader->get_statuscondition();
            reader_status_condition->set_enabled_statuses(::DDS::DATA_AVAILABLE_STATUS);

            ::DDS::ReadCondition_var reader_condition =
                sample_reader2->create_readcondition(::DDS::NOT_READ_SAMPLE_STATE,
                                                     ::DDS::ANY_VIEW_STATE,
                                                     ::DDS::ANY_INSTANCE_STATE);

            ::DDS::WaitSet wait_set;
            wait_set.attach_condition(reader_condition);

            ::DDS::ConditionSeq_var condition_list_out = new ::DDS::ConditionSeq();

            int read_samples = 0;
            int received_samples = 0;
            DDS::Duration_t timeout = ::DDS::DURATION_INFINITE;

            Test2466::SampleSeq result_sample_seq;
            result_sample_seq.length(INS_COUNT);
            ::DDS::SampleInfoSeq result_info_seq;
            result_info_seq.length(INS_COUNT);

            while (read_samples < MSG_COUNT)
            {
                result = wait_set.wait(condition_list_out.inout(),
                                       timeout);

                timeout.sec     = 5;
                timeout.nanosec = 0;

                if (result != ::DDS::RETCODE_OK)
                {
                    std::cerr << "FAIL: Waitset wait timed out!" << std::endl;
                    return -1;
                }

                if (condition_list_out->length() != 1 ||
                    condition_list_out[0U].in()  != reader_condition.in())
                {
                    std::cerr << "FAIL: Unexpected read condition value!" << std::endl;
                    return -1;
                }

                Test2466::SampleSeq sample_seq;
                ::DDS::SampleInfoSeq_var info_seq = new ::DDS::SampleInfoSeq();

                result = sample_reader->take(sample_seq,
                                             info_seq,
                                             ::DDS::LENGTH_UNLIMITED,
                                             ::DDS::ANY_SAMPLE_STATE,
                                             ::DDS::ANY_VIEW_STATE,
                                             ::DDS::ANY_INSTANCE_STATE);

                if (result == ::DDS::RETCODE_OK)
                {
                    for (unsigned int i = 0; i < sample_seq.length(); i++)
                    {
                        if (info_seq[i].valid_data == true)
                        {
                            unsigned int n = sample_seq[i].id;

                            result_sample_seq[n] = sample_seq[i];
                            result_info_seq[n]   = info_seq[i];

                            std::cout << "Subscriber["
                                      << this->id
                                      << "] received sample with id ["
                                      << sample_seq[i].id
                                      << "] data ["
                                      << sample_seq[i].data.in()
                                      << "] with time stamp ["
                                      << info_seq[i].source_timestamp.sec
                                      << ":"
                                      << info_seq[i].source_timestamp.nanosec
                                      << "] from Publisher["
                                      << sample_seq[i].publisher_id
                                      << "]"
                                      << std::endl;

                            received_samples++;
                        }
                    }

                    sample_reader->return_loan(sample_seq, info_seq);
                }
                else if (result != ::DDS::RETCODE_NO_DATA)
                {
                    std::cerr << "FAIL: Can't read data!" << std::endl;
                    return -1;
                }

                result = sample_reader2->read_w_condition(sample_seq,
                                                          info_seq,
                                                          ::DDS::LENGTH_UNLIMITED,
                                                          reader_condition.in());

                if (result != ::DDS::RETCODE_OK)
                {
                    std::cerr << "FAIL: Can't read data!" << std::endl;
                    return -1;
                }

                for (unsigned int i = 0; i < sample_seq.length(); i++)
                {
                    if (info_seq[i].valid_data == true)
                    {
                        read_samples++;
                    }
                }

                sample_reader->return_loan(sample_seq, info_seq);
            }

            std::cout << std::endl
                      << "------------------------------------------------"
                      << std::endl
                      << "Last sample of each instance"
                      << std::endl
                      << std::endl;

            for (unsigned int i = 0; i < result_sample_seq.length(); i++) {
               std::cout << "Subscriber["
                          << this->id
                          << "] received sample with id ["
                          << result_sample_seq[i].id
                          << "] data ["
                          << result_sample_seq[i].data.in()
                          << "] with time stamp ["
                          << result_info_seq[i].source_timestamp.sec
                          << ":"
                          << result_info_seq[i].source_timestamp.nanosec
                          << "] from Publisher["
                          << result_sample_seq[i].publisher_id
                          << "]"
                          << std::endl;
            }

            ::DDS::SampleLostStatus lost_status;
            result = sample_reader->get_sample_lost_status(lost_status);
            if (result != ::DDS::RETCODE_OK)
            {
                std::cerr << "FAIL: Can't read sample lost status!" << std::endl;
                return -1;
            }

            std::cout << std::endl
                      << "received: "
                      << received_samples
                      << " lost: "
                      << lost_status.total_count
                      << " expected total: "
                      << MSG_COUNT
                      << " total: "
                      << received_samples + lost_status.total_count
                      << std::endl;

            if (received_samples + lost_status.total_count != MSG_COUNT)
            {
                std::cout << std::endl
                          << "FAIL: total number of samples is incorrect"
                          << std::endl;
            }

            return 0;
        }//run
    //--------------------------------------------------------------------------
    // Private data:
    private:
        /**
         * \private
         * \brief The subscriber ID.
         */
        // Subscriber id:
        int id;
    //--------------------------------------------------------------------------
};//DDS2466Subscriber

#endif // _DDS2466_SUBSCRIBER_H_
