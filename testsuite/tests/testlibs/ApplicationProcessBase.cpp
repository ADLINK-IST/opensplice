// $Id:$

#include "ApplicationProcessBase.h"

OSPLTestLib::ApplicationProcessBase::ApplicationProcessBase ()
    : 
{
}

OSPLTestLib::ApplicationProcessBase::~ApplicationProcessBase (void)
{
}

CORBA::Boolean
OSPLTestLib::ApplicationProcessBase::init (int argc, char *argv[])
{
    // @todo arg parsing and whatnot
    return true;
}

void 
OSPLTestLib::ApplicationProcessBase::poke_process_with (
            const char * process_action,
            const char * process_target,
            const ::OSPLTestLib::DataSeq & poke_with,
            ::CORBA::ULong seq_no,
            const char * process_source)
{
    if (! seq_no)
        seq_no = ++this->sequence_no_;

    if (process_source == "")
        process_source = own_name_.in ();

    

       
}