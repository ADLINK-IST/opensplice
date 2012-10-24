#include "TestLibsAPI.h"

// Only impl at the minute is the CORBA one
#include "tao/TAOTestLibsAPIImpl.h"

#include "ApplicationProcessBase.h"

namespace OSPLTestLib
{


::OSPLTestLib::ProcessControlImpl*
TestLib::get_process_controller (const ProcessName process_self,
                                 int argc,
                                 char *argv[])
{
    ::OSPLTestLib::ProcessControlImpl* the_process
        = new ProcessControlImpl (process_self);
    ::OSPLTestLib::ProcessControl_var safe_process (the_process);

    if (! the_process->init (argc, argv))
    {
        // Init failed - release
        safe_process = ::OSPLTestLib::ProcessControl::_nil ();
    }

    // @todo Needs fixing
    safe_process._retn ();
    return the_process;
}


} /* end of namespace OSPLTestLib */
