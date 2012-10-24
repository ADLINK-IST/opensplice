#include "CPPProc1.h"

int main (int argc, char *argv[])
{
    int test_result = 0;

    // default the process name to exe name
    CPPProc1 dds1619_A_proc1 ("ccpp_alldatadisposedlistener_dds1619_proc1");

    dds1619_A_proc1.set_other_process_name ("ccpp_alldatadisposedlistener_dds1619_proc2");

    try
    {
        dds1619_A_proc1.init (argc, argv);

        test_result = dds1619_A_proc1.run ();
    }
    catch (...)
    {
        test_result = -1;
    }

    dds1619_A_proc1.wait_for_termination();

    return test_result;
}
