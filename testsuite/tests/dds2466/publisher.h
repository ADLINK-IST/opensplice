/** \file
 * \brief File that defines the subscriber.
 *
 */

#ifndef _DDS2466_PUBLISHER_H_
#define _DDS2466_PUBLISHER_H_

// How many messages the publisher will send:
#define MSG_COUNT    10
// Maximum data length:
#define MAX_DATA_LEN 50

#include <iostream>
#include <iostream>
#include <sstream>

#include "ccpp_dds_dcps.h"
#include "../testlibs/CPPTestProcess.h"
#include "ccpp_dds2466.h"

/**
* \class DDS2466Publisher
* \brief A publish process for the test.
* The publisher publishes a sample synchroniously with another using the same
* unique field value and different non-unique field value.
*/
class DDS2466Publisher: public virtual OSPLTestLib::CPPTestProcess
{
    // Public methods:
    public:
        //----------------------------------------------------------------------
        /**
         * \brief Constructs the object instance with the defined name.
         * \param[in] process_name - the name for this process.
         * Need for the process synchronization.
         */
        DDS2466Publisher(const char* process_name):
            CPPTestProcess(process_name),
            id(0),
            waiter(false)
        {}//DDS2466Publisher
        //----------------------------------------------------------------------
        /**
         * \brief Object destructor.
         */
        virtual ~DDS2466Publisher()
        {}//~DDS2466Publisher
        //----------------------------------------------------------------------
        /**
         * \brief Initialize the object instance with the defined name.
         * Sets publisher ID using system variable PUB_ID.
         * Setup if the publisher will wait for the synchronization or will "poke" another publisher.
         * \param[in] argc - arguments count.
         * \param[in] argv - arguments list.
         */
        virtual void init(int argc, char *argv[])
        {
            OSPLTestLib::CPPTestProcess::init(argc, argv);

            char* pub_id = getenv("PUB_ID");
            if (pub_id != NULL)
            {
                this->id = atoi(pub_id);
            }
            char* pub_waiter = getenv("PUB_WAITER");
            if (pub_waiter != NULL)
            {
                this->waiter = true;
            }
        }//init
        //----------------------------------------------------------------------
        /**
         * \brief Starts reading.
         * \return 0 - in case of success, -1 - in case of any failure.
         */
        virtual int run()
        {
            // Operation result:
            DDS::ReturnCode_t result;

            // Topic sample:
            Test2466::Sample sample;

            // Sample time stamp:
            DDS::Time_t time_stamp;
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

            ::DDS::Publisher_var publisher =
                domain_participant->create_publisher(
                    PUBLISHER_QOS_DEFAULT,
                    ::DDS::PublisherListener::_nil(),
                    0U);
            if (::DDS::is_nil(publisher.in()))
            {
                std::cerr << "FAIL: Can't create the publisher!" << std::endl;
                return -1;
            }

            DDS::DataWriterQos dw_qos = DATAWRITER_QOS_DEFAULT;
            dw_qos.durability.kind = ::DDS::PERSISTENT_DURABILITY_QOS;
            dw_qos.destination_order.kind = ::DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
            dw_qos.writer_data_lifecycle.autodispose_unregistered_instances = false;

            ::DDS::DataWriter_var generic_writer =
                publisher->create_datawriter(
                    topic,
                    dw_qos,
                    ::DDS::DataWriterListener::_nil(),
                    0U);
            Test2466::SampleDataWriter_var sample_writer =
                Test2466::SampleDataWriter::_narrow(generic_writer.in());
            if (::DDS::is_nil(sample_writer.in()))
            {
                std::cerr << "FAIL: Can't create the data writer!" << std::endl;
                return -1;
            }

            std::cout << "The publisher will write ["
                      << MSG_COUNT
                      << "] samples"
                      << std::endl;
            for (int i = 0; i < MSG_COUNT; i++)
            {
                std::stringstream msg(std::stringstream::in | std::stringstream::out);
                sample.id   = i;
                sample.publisher_id = this->id;
                msg << "Publisher #"
                    << this->id
                    << " sent id=["
                    << sample.id
                    << "].";
                sample.data = msg.str().c_str();

                time_stamp.sec     = i + 1;
                time_stamp.nanosec = 0;

                // Wait for sync. or send sync. ack. depending of the publisher role:
                if (this->waiter)
                {
                    std::cout << "Waiting for another publisher process to synchronize..." << std::endl;
                    this->process_controller_->wait_for_process_state(i + 1);
                }
                else
                {
                    std::cout << "Synchronizing with another publisher process..." << std::endl;
                    this->process_controller_->set_process_status(
                        i + 1,
                        this->other_process_name_.in());
                }
                std::cout << "Done." << std::endl;

                result = sample_writer->write_w_timestamp(
                    sample,
                    ::DDS::HANDLE_NIL,
                    time_stamp);

                if (result != ::DDS::RETCODE_OK)
                {
                    std::cerr << "FAIL: Can't write the data sample!" << std::endl;
                    return -1;
                }

                std::cout << "Publisher["
                            << this->id
                          << "] sent id ["
                          << sample.id
                          << "] data ["
                          << sample.data.in()
                          << "] with time stamp ["
                          << time_stamp.sec
                          << ":"
                          << time_stamp.nanosec
                          << "]"
                          << std::endl;
            }//for-i
            return 0;
        }//run
    //--------------------------------------------------------------------------
    // Private data:
    private:
        /**
         * \private
         * \brief The publisher ID.
         */
        // Publisher id:
        int id;
        /**
         * \private
         * \brief Will the publisher wait for another process or not.
         */
        // If the publisher instance requests synchronization
        // with another publisher, or responds to the request.
        bool waiter;
};//DDS2466Publisher

#endif // _DDS2466_PUBLISHER_H_
