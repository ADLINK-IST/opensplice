    /** \defgroup group_dds2466 DDS2466 - eFDPfi_MW_DDS_15
     */

    /**
     * @addtogroup group_dds2466
     *
     * <b>Test code: osplo/testsuite/rbt/sac/dds2466/subscriber.cpp</b>
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
     * The subscriber application waits for \c MSG_COUNT data samples, prints all received samples and quits. After that manual check for received samples should be performed.
     * Each subscriber has the ID (0 - default). It can be set by \c SUB_ID system varibale.
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
     * \arg A \c Subscriber is created with default QoS
     * \arg A \c DataReader is created on the \c Subscriber, for the \c Topic with default QoS
     * \arg A \c StatusCondition created for the \c Subscriber with the \c DATA_AVAILABLE_STATUS status
     * \arg A \c WaitSet for the \c StatusCondition is created
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
     * The application waits until the \c StatusCondition is changed.
     *
     * \e Result
     *
     * The waiting is successfully passed.\n
     * If not the test verdict is set to \c FAIL, this is reported with the message \c "FAIL: Can't wait data!" and no further testing occurs.\n
     * If the \c WaitSet returns with not \c DATA_AVAILABLE_STATUS status condition then the test verdict is set to \c FAIL,  this is reported with the message \c "FAIL: Unexpected status condition value!" and no further testing occurs.
     *
     * <b>Step 2</b>
     *
     * \e Action
     *
     * The application reads data.
     *
     * \e Result
     *
     * Reading operation successfully completed.\n
     * If not the test verdict is set to \c FAIL, this is reported with the message \c "FAIL: Can't read data!" and no further testing occurs.
     */

#include "subscriber.h"

int main(int argc, char *argv[])
{
    std::cout << "DDS2466 subscriber test started." << std::endl;

    int test_result = 0;

    std::string sub_process_name = argv[0];
    if (getenv("SUB_ID") != NULL)
    {
        sub_process_name.append(getenv("SUB_ID"));
    }
    std::cout << "Creating subscriber [" << sub_process_name  << "]..." << std::endl;
    DDS2466Subscriber subscriber(sub_process_name.c_str());
    std::cout << "Done." << std::endl;

    try
    {
        subscriber.init(argc, argv);
        test_result = subscriber.run();
    }
    catch (...)
    {
        return -1;
    }

    return 0;
}
