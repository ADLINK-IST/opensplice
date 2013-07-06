    /** \defgroup group_dds2466 DDS2466 - eFDPfi_MW_DDS_15
     */

    /**
     * @addtogroup group_dds2466
     *
     * <b>Test code: osplo/testsuite/rbt/sac/dds2466/publisher.cpp</b>
     * 
     * This is the test descriptions for the eFDPfi_MW_DDS_15 CoFlight test.
     *
     * <b>Prerequisites, assumptions, constraints</b>
     *
     * \arg The test is run on a multi node and are platform independent.
     * \arg The ospl daemon must be started before the test begin on each nodes.
     * \arg The test uses OSPL test lib CORBA-based framework for process synchronization (ospli/testsuite/tests/testlib).
     *
     * <b>Test ID: eFDPfi_MW_DDS_15</b>
     *
     * <b>Test objectives</b>
     *
     * The objective of this test is to check does DDS deliver a consistent end result even in case of simultaneous updates on the same instance.
     *
     * <b>Test description</b>
     *
     * The publisher application sends \c MSG_COUNT data samples, prints all received samples and quits. After that manual check for sent samples should be performed.
     * To write data samples simultaneously with another publisher the application synchronizes with another one using the process state mechanism (\c wait_for_process_state/\c set_process_status).
     * When the synchronization point is reached publishers writes the data sample with the same key value but different non-key value.\n
     * Each publisher has the ID (0 - default). It can be set by \c PUB_ID system varibale. To define which publisher will wait for the synchronization point the \c PUB_WAITER system varibale is used. If it is defined then the publisher will wait for another.
     *
     * <b>Test procedure</b>
     *
     * \e Action
     *
     * The following entities are obtained/created
     *
     * \arg \c DomainParticipantFactory
     * \arg \c DomainParticipant with the default QoS
     * \arg The single type of the topic \c TEST_DDS2466_TOPIC is registered with the \c DomainParticipant
     * \arg A \c Topic (\c TEST_DDS2466_TOPIC) is created on the DomainParticipant with default QoS
     * \arg A \c Publisher is created with default QoS
     * \arg A \c DataWriter is created on the \c Subscriber, for the \c Topic with default QoS
     *
     * \e Result
     *
     * It is expected that all entities are created/initialized correctly and without any failures.\n
     * If a failure occurs at any of the above stages, the test fails, this is reported and no further testing is performed.
     *
     * <b>Step 1</b>
     *
     * \e Action
     *
     * The publisher waits for another publisher or tries to poke another publisher (depending on the publisher role).
     *
     * \e Result
     *
     * The waiting is successfully passed or poking another publisher successfully perfrormed (depending on the publisher role).\n
     * If not the test verdict is set to \c FAIL, and no further testing occurs.\n
     *
     * <b>Step 2</b>
     *
     * \e Action
     *
     * The application sends data.
     *
     * \e Result
     *
     * Writing operation successfully completed.\n
     * If not the test verdict is set to \c FAIL, this is reported with the message \c "FAIL: Can't write the data sample!" and no further testing occurs.
     */

#include <iostream>
#include <string>
#include "publisher.h"

int main(int argc, char *argv[])
{
    std::cout << "DDS2466 publisher test started." << std::endl;

    int test_result = 0;

    std::string pub_process_name = getenv("PUB_EXEC");
    if (getenv("PUB_ID") != NULL)
    {
        pub_process_name.append(getenv("PUB_ID"));
    }
    std::cout << "Creating publisher [" << pub_process_name  << "]..." << std::endl;
    DDS2466Publisher publisher(pub_process_name.c_str());
    std::cout << "Done." << std::endl;

    std::string another_pub_process_name = getenv("PUB_EXEC");
    if (getenv("ANOTHER_PUB_ID") != NULL)
    {
        another_pub_process_name.append(getenv("ANOTHER_PUB_ID"));
    }
    std::cout << "Set another publisher process to ["
              << another_pub_process_name
              << "]."
              << std::endl;
    publisher.set_other_process_name(another_pub_process_name.c_str());

    try
    {
        publisher.init(argc, argv);
        test_result = publisher.run();
    }
    catch (...)
    {
        return -1;
    }

    return 0;
}
