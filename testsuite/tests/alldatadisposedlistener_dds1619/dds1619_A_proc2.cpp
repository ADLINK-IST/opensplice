#include "CPPProc2.h"

int main (int argc, char *argv[])
{
    int test_result = 0;

    // default the process name to exe name
    CPPProc2 dds1619_A_proc2 ("ccpp_alldatadisposedlistener_dds1619_proc2");

    dds1619_A_proc2.set_other_process_name ("ccpp_alldatadisposedlistener_dds1619_proc1");

    try
    {
        dds1619_A_proc2.init (argc, argv);

        test_result = dds1619_A_proc2.run ();
    }
    catch (...)
    {
        test_result = -1;
    }

    dds1619_A_proc2.terminate();

    return test_result;
}
