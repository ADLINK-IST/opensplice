#include "CPPSubscriberProcess.h"

int main (int argc, char *argv[])
{
    int test_result = 0;

    // default the process name to exe name
    CPPSubscriberProcess subscriber ("ccpp_hello_sub");

    subscriber.set_other_process_name ("ccpp_hello_pub");

    try
    {
        subscriber.init (argc, argv);

        test_result = subscriber.run ();
    }
    catch (...)
    {
        test_result = -1;
    }
    return test_result;
}
