#include "CPPPublisherProcess.h"

int main (int argc, char *argv[])
{
    int test_result = 0;

    // default the process name to exe name
    CPPPublisherProcess publisher ("ccpp_net_compression_dds1578_pub");

    publisher.set_other_process_name ("ccpp_net_compression_dds1578_sub");

    try
    {
        publisher.init (argc, argv);

        test_result = publisher.run ();
    }
    catch (...)
    {
        test_result = -1;
    }
    return test_result;
}
