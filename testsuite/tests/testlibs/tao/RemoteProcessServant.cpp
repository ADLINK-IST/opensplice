// $Id:$

#include "RemoteProcessServant.h"

namespace OSPLTestLib {

    RemoteProcessServant::RemoteProcessServant (const char * process_name,
                                             ::CORBA::ORB_ptr an_orb)
        : own_name_ (CORBA::string_dup (process_name)),
        the_orb_ (::CORBA::ORB::_duplicate (an_orb)),
        process_status_ (0)
    {
    }

    RemoteProcessServant::~RemoteProcessServant (void)
    {
    }

    void 
    RemoteProcessServant::poke_process_with (
                const char * process_action,
                const char * process_target,
                const ::OSPLTestLib::DataSeq & poke_with,
                ::CORBA::ULong seq_no,
                const char * process_source)
    {
        // Notify the test application callback
        if (! CORBA::is_nil (this->process_self_.in ()))
        {
            this->process_self_.in ()->poke_process_with (process_action,
                                                          process_target,
                                                          poke_with,
                                                          seq_no,
                                                          process_source);
        }
    }

    ::OSPLTestLib::DataSeq *
    RemoteProcessServant::call_process_with (
                const char * process_action,
                const char * process_target,
                const ::OSPLTestLib::DataSeq & call_with,
                ::CORBA::ULong seq_no,
                const char * process_source)
    {
        // Notify the test application callback
        if (! CORBA::is_nil (this->process_self_.in ()))
        {
            return this->process_self_.in ()->call_process_with (process_action,
                                                          process_target,
                                                          call_with,
                                                          seq_no,
                                                          process_source);
        }
        return & ProcessControlImpl::NO_DATA_ARG; 
    }

    ::CORBA::Long
    RemoteProcessServant::get_process_status (
                const char * process_target)
    {
        return process_status_;
    }

    CORBA::Boolean 
    RemoteProcessServant::set_process_status (
                ::CORBA::Long process_status,
                const char * process_target)
    {
        // Set our status
        this->process_status_ = process_status;

        // Notify the test application callback
        if (! CORBA::is_nil (this->process_self_.in ()))
        {
            this->process_self_.in ()->set_process_status (process_status,
                                                          process_target);
        }
        return true;
    }

    void 
    RemoteProcessServant::shutdown (
                const char * process_target)
    {
        // First notify the test application callback
        if (! CORBA::is_nil (this->process_self_.in ()))
        {
            return this->process_self_.in ()->shutdown (process_target);
        }

        // Then shutdown
        if (! CORBA::is_nil (the_orb_.in ()))
        {
            cout << "Shutdown ...\n";
            the_orb_.in ()->shutdown (0);
        }
        else
        {
            fprintf (stderr, "ERROR %s (%d): No Can't ::shutdown "
                         "%s\n", __FILE__, __LINE__,
                                 process_target);
        }
    }

    void 
    RemoteProcessServant::set_process_self (
            ::OSPLTestLib::RemoteProcessServer_ptr process_self)
    {
        process_self_ = ::OSPLTestLib::RemoteProcessServer::_duplicate (process_self);
    }
                        
} /* End of namespace OSPLTestLib */
