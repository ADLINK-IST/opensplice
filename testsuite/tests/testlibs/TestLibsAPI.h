#ifndef OSPL_TESTLIBS_API_H
#define OSPL_TESTLIBS_API_H

// For now I've only got the CORBA based impl so this is unconditional
#include "TestLibsAPIC.h"
#include "tao/TAOTestLibsAPIImpl.h"

namespace OSPLTestLib
{
    class ProcessControlImpl;

    /**
    * @brief The bootstrap factory entry points for the testlib API and some utils and stuff
    */
    class OSPL_TESTLIBS_API_Export TestLib
    {
    public:
        
        /**
        * @brief Create and configure a ProcessControl instance.
        * @param this_process_name A unique string identifying this test process.
        * @param argc Number of string args in argv
        * @param argv Arg strings hoding any necessary configuration for the PC
        */
        static ::OSPLTestLib::ProcessControlImpl*
            get_process_controller (const ProcessName this_process_name,
                                    int argc, 
                                    char *argv[]);
    };
}

#endif /* OSPL_TESTLIBS_API_H */
