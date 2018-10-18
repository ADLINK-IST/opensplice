
/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "implementation.hpp"
#include "common/example_utilities.h"

#include <iostream>

#include "address.pbdds.hpp"

namespace examples { namespace protobuf { namespace isocpp  {

int publisher(int argc, char *argv[])
{
    int result = 0;
    (void) argc;
    (void) argv;
    try
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());

        /** A dds::topic::Topic is created for our protobuf type on the domain participant. */
        dds::topic::Topic<address::Person> topic(dp, "Person");

        /** A dds::pub::Publisher is created on the domain participant. */
        dds::pub::Publisher pub(dp);

        /** The dds::pub::qos::DataWriterQos is derived from the topic qos */
        dds::pub::qos::DataWriterQos dwqos;

        dwqos << dds::core::policy::Reliability::Reliable();
        /** A dds::pub::DataWriter is created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<address::Person> dw(pub, topic);

        /** Synchronize on subscriber availability. */
        std::cout << "Publisher: waiting for subscriber... " <<std::endl;
        unsigned long current = exampleTimevalToMicroseconds(exampleGetTime());
        unsigned long timeout = current + (30 * 1000 * 1000);
        bool stop = false;
        ::dds::core::status::PublicationMatchedStatus matched;

        do {
            matched = dw.publication_matched_status();

            if (exampleTimevalToMicroseconds(exampleGetTime()) > timeout) {
                stop = true;
            }
            if ((matched.current_count() == 0) && (!stop)) {
                exampleSleepMilliseconds(500);
            }
        } while ((matched.current_count() == 0) && (!stop));

        if (matched.current_count() != 0) {
            std::cout << "Publisher: Subscriber found" << std::endl;

            /** A sample is created and then written. */
            address::Person msgInstance;
            msgInstance.set_name("Jane Doe");
            msgInstance.set_email("jane.doe@somedomain.com");
            msgInstance.set_age(23);

            address::Person::PhoneNumber* phone = msgInstance.add_phone();
            phone->set_number("0123456789");
            address::Organisation* worksFor = msgInstance.mutable_worksfor();
            worksFor->set_name("Acme Corporation");
            worksFor->set_address("Wayne Manor, Gotham City");
            worksFor->mutable_phone()->set_number("9876543210");
            worksFor->mutable_phone()->set_type( ::address::Person_PhoneType_WORK);
            std::cout << "Publisher: publishing Person: " << msgInstance.name() << std::endl;

            ::dds::core::InstanceHandle handle = dw.register_instance(msgInstance);

            dw << msgInstance;

            std::cout << "Publisher: sleeping for 5 seconds..." << std::endl;

            exampleSleepMilliseconds(5000);

            std::cout << "Publisher: disposing Jane Doe..." << std::endl;

            /** Disposing the DDS instance associated with the name field of the
              * Protobuf data structure which is the key in DDS*/

            dw.dispose_instance(handle);
            exampleSleepMilliseconds(1000);
        } else {
            throw ::dds::core::PreconditionNotMetError("Subscriber NOT found, terminating...");
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "Publisher: ERROR: " << e.what() << std::endl;
        result = 1;
    }
    std::cout << "Publisher: terminating..." << std::endl;
    return result;
}

class ReadCondHandler
{
public:
    /**
     * @param dataState The dataState on which to filter the samples
     */
    ReadCondHandler(): updateCount(0) {}

    void operator() (dds::sub::cond::ReadCondition c)
    {
        std::string states, sampleState, viewState, instanceState;
        dds::sub::DataReader<address::Person> dr = c.data_reader();
        dds::sub::LoanedSamples<address::Person> samples = dr.select().state(c.state_filter()).take();

        for (dds::sub::LoanedSamples<address::Person>::const_iterator sample = samples.begin();
            sample < samples.end(); ++sample)
        {
            updateCount++;

            if(sample->info().state().sample_state() == dds::sub::status::SampleState::read()){
                sampleState = "READ";
            } else {
                sampleState = "NOT_READ";
            }
            if(sample->info().state().view_state() == dds::sub::status::ViewState::new_view()){
                viewState = "NEW";
            } else {
                viewState = "NOT_NEW";
            }
            if(sample->info().state().instance_state() == dds::sub::status::InstanceState::alive()){
                instanceState = "ALIVE";
            } else if(sample->info().state().instance_state() == dds::sub::status::InstanceState::not_alive_disposed()){
                instanceState = "NOT_ALIVE_DISPOSED";
            } else {
                instanceState = "NOT_ALIVE_NO_WRITERS";
            }
            states = "(" + sampleState + ", " + viewState + ", " + instanceState + ")";

            if(sample->info().valid())
            {
                std::cout << "Subscriber: reading sample " <<  states << ":" << std::endl;
                printPerson(sample->data(), "");
            } else {
                std::cout << "Subscriber: reading invalid sample " << states << ":" << std::endl;
                std::cout << "- Name     = " << sample->data().name() << std::endl;
                std::cout << "- Company  = " << std::endl;
                std::cout << "   - Name  = " << sample->data().worksfor().name() << std::endl;
            }
        }
    }

    int getUpdateCount(){
        return updateCount;
    }

private:
    int updateCount;

    void printPhone(const ::address::Person_PhoneNumber phone, std::string tabs){
        std::string type;

        switch(phone.type()){
        case ::address::Person_PhoneType_MOBILE:
            type = "MOBILE";
            break;
        case ::address::Person_PhoneType_HOME:
            type = "HOME";
            break;
        case ::address::Person_PhoneType_WORK:
            type = "WORK";
            break;
        default:
            type = "UNKNOWN";
            break;
        }
        std::cout << tabs << "- Phone      = " << phone.number() <<
                " (" + type << ")" << std::endl;

    }

    void printPerson(address::Person person, std::string tabs){
        std::cout << tabs << "- Name       = " << person.name() << std::endl;
        std::cout << tabs << "- Age        = " << person.age() << std::endl;
        std::cout << tabs << "- Email      = " << person.email() << std::endl;

        for (int i=0; i< person.phone_size(); i++) {
            printPhone(person.phone(i), tabs);
        }
        std::cout << tabs << "- Company    = " << std::endl;
        std::cout << tabs << "   - Name       = " << person.worksfor().name() << std::endl;
        std::cout << tabs << "   - Address    = " << person.worksfor().address() << std::endl;

        if(person.worksfor().has_phone()){
            printPhone(person.worksfor().phone(), tabs + "   ");
        } else {
            std::cout << tabs << "   - Phone   = NONE" << std::endl;
        }
    }
};


/**
 * Runs the subscriber role of this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int subscriber(int argc, char *argv[])
{
    int result = 0;
    (void) argc;
    (void) argv;
    try
    {
        int expectedUpdates = 2;
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());

        /** A dds::topic::Topic is created for our protobuf type on the domain participant. */
        dds::topic::Topic<address::Person> topic(dp, "Person");

        /** A dds::pub::Subscriber is created on the domain participant. */
        dds::sub::Subscriber sub(dp);

        /** The dds::pub::qos::DataWriterQos is derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos;

        drqos << dds::core::policy::Reliability::Reliable();
        /** A dds::pub::Reader is created on the Subscriber & Topic with the modififed Qos. */
        dds::sub::DataReader<address::Person> dr = dds::sub::DataReader<address::Person>(sub, topic, drqos);

        /** any sample, view and instance state */
        dds::sub::cond::ReadCondition readCond(dr, dds::sub::status::DataState::any());
        ReadCondHandler personHandler;
        readCond.handler(personHandler);

        /** A WaitSet is created and the four conditions created above are attached to it */
        dds::core::cond::WaitSet waitSet;
        waitSet += readCond;

        dds::core::Duration waitTimeout(30, 0);

        /** Wait until the condition in the WaitSet triggers and dispatch the corresponding functor*/
        do {
            waitSet.dispatch(waitTimeout);
        } while(personHandler.getUpdateCount() < expectedUpdates);
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "Subscriber: ERROR: " << e.what() << std::endl;
        result = 1;
    }
    std::cout << "Subscriber: terminating..." << std::endl;

    return result;
}

}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Protobuf_publisher, examples::protobuf::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Protobuf_subscriber, examples::protobuf::isocpp::subscriber)
