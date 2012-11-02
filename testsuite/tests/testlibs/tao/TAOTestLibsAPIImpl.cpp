// $Id:$

#include "TAOTestLibsAPIImpl.h"
#include <tao/IORTable/IORTable.h>
#include "os_time.h"

 

namespace OSPLTestLib {

    ::OSPLTestLib::DataSeq ProcessControlImpl::NO_DATA_ARG = ::OSPLTestLib::DataSeq (0);

    ProcessControlImpl::ProcessControlImpl (const char * process_name)
        : default_timeout_ (OSPLTestLib::FOREVER),
        sequence_no_ (0),
        own_name_ (CORBA::string_dup (process_name)),
        the_orb_ (::CORBA::ORB::_nil ()),
        servant_ (0)
    {
    }

    ProcessControlImpl::~ProcessControlImpl (void)
    {
    }

    ::CORBA::Boolean
    ProcessControlImpl::init (int argc, char *argv[])
    {
        try
        {
            // @todo Pure CORBAism - split out 
            this->the_orb_ = ::CORBA::ORB_init (argc, argv);

            ::CORBA::Object_var poa_object =
                the_orb_->resolve_initial_references ("RootPOA");

            ::PortableServer::POA_var root_poa =
                ::PortableServer::POA::_narrow (poa_object.in ());

            if (CORBA::is_nil (root_poa.in ()))
                ACE_ERROR_RETURN ((LM_ERROR,
                                 "ERROR (%P|%t) Catastrophic failure accessing root POA\n"),
                                 0);

            ::PortableServer::POAManager_var poa_manager = root_poa->the_POAManager ();
            
            // We want persistent references
            ::CORBA::PolicyList policies (2);
            policies.length (2);
            policies[0] = root_poa->create_id_assignment_policy (PortableServer::USER_ID);
            policies[1] = root_poa->create_lifespan_policy (PortableServer::PERSISTENT);

            ::PortableServer::POA_var servant_poa = root_poa->create_POA (this->own_name_.in (),
                                                                          poa_manager.in (),
                                                                          policies);

            // Creation of the new POA is over, so destroy the Policy_ptr's.
            for (::CORBA::ULong i = 0; i < policies.length (); ++i)
            {
                ::CORBA::Policy_ptr policy = policies[i];
                policy->destroy ();
            }

            this->servant_ = new RemoteProcessServant (this->own_name_.in (),
                                                       this->the_orb_.in ());

            ::PortableServer::ObjectId_var server_id =
                ::PortableServer::string_to_ObjectId (this->own_name_.in ());

            servant_poa->activate_object_with_id (server_id.in (),
                                                        this->servant_);

            CORBA::Object_var obj = servant_poa->id_to_reference (server_id.in ());

            if (CORBA::is_nil(obj.in ()))
            {
                fprintf (stderr, "ERROR %s (%d): Nil obj returned from id_to_reference in ::init\n", 
                              __FILE__, __LINE__);
                throw "";
            }

            this->the_orb_->register_initial_reference (this->own_name_.in(), obj.in ());

            CORBA::String_var str = this->the_orb_->object_to_string (obj.in ());
            
            CORBA::Object_var tmp = this->the_orb_->resolve_initial_references ("IORTable");
            IORTable::Table_var iorTable = IORTable::Table::_narrow(tmp.in ());
            
            if (! CORBA::is_nil(iorTable.in ())) 
            {
		        iorTable->bind(this->own_name_.in(), str.in());
            }
            
            this->process_self_ = ::OSPLTestLib::RemoteProcessServer::_narrow (obj.in ());

            if (CORBA::is_nil(this->process_self_.in ()))
            {
                fprintf (stderr, "ERROR %s (%d): Nil process self reference in ::init\n", 
                              __FILE__, __LINE__);
                throw "";
            }

            poa_manager.in ()->activate ();

            return true;
        }
    #ifdef ACE_VERSION // TAO
        catch (const CORBA::Exception& ex)
        {
            fprintf (stderr, "ERROR %s (%d): CORBA Exception  ::init\n", 
                              __FILE__, __LINE__);
            ex._tao_print_exception ("");
            throw;
        }
    #endif
        catch (...)
        {
            fprintf (stderr, "ERROR %s (%d): Unknown exception in ::init\n", 
                              __FILE__, __LINE__);
            throw;
        }
        return false;
    }

    void 
    ProcessControlImpl::poke_process_with (
                const char * process_action,
                const char * process_target,
                const ::OSPLTestLib::DataSeq & poke_with,
                ::CORBA::ULong seq_no,
                const char * process_source)
    {
        if (! seq_no)
            seq_no = ++this->sequence_no_;

        if (process_source == 0)
            process_source = own_name_.in ();

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            target = this->get_named_process_target (process_target);
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::poke_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
            throw "";
            // should be return false ?
        }
        else
        {
            try
            {
                this->process_self_.in ()->poke_process_with (process_action,
                                                            process_target,
                                                            poke_with,
                                                            seq_no,
                                                            process_source);
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::poke_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
                ex._tao_print_exception ("");
                throw;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::poke_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
                throw;
            }
        }
        
        /*for (CORBA::ULong i = 0; (all || all_other || ! found) 
                                && i < this->other_processes_.in ().length () ; ++i)
        {
            if (all || all_other || strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
            {
                try
                {
                    this->other_processes_.in ()[i]->poke_process_with (process_action,
                                                                    process_target,
                                                                    poke_with,
                                                                    seq_no,
                                                                    process_source);
                }
    #ifdef ACE_VERSION // TAO
                catch (const CORBA::Exception& ex)
                {
                    fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::poke_process_with from %s"
                        " to %s (%s) with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                     process_target, this->other_process_names_.in ()[i], process_action, seq_no);
                    ex._tao_print_exception ("");
                    throw;
                }
    #endif
                catch (...)
                {
                    fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::poke_process_with from %s"
                        " to %s (%s) with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                     process_target, this->other_process_names_.in ()[i], process_action, seq_no);
                    throw;
                }
                found = true;
            }
        }

        if (!found)
        {
            fprintf (stderr, "ERROR %s (%d): No target found for ::poke_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
            throw "";
        }*/
    }

    ::OSPLTestLib::DataSeq *
    ProcessControlImpl::call_process_with (
                const char * process_action,
                const char * process_target,
                const ::OSPLTestLib::DataSeq & call_with,
                ::CORBA::ULong seq_no,
                const char * process_source)
    {
        if (! seq_no)
            seq_no = ++this->sequence_no_;

        if (process_source == 0)
            process_source = own_name_.in ();

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            target = this->get_named_process_target (process_target);
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::call_process_with from %s"
                         " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
            throw "";
            // should be return false ?
        }
        else
        {
            try
            {
                return this->process_self_.in ()->call_process_with (process_action,
                                                            process_target,
                                                            call_with,
                                                            seq_no,
                                                            process_source);
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::call_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
                ex._tao_print_exception ("");
                throw;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::call_process_with from %s"
                    " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
                throw;
            }
        }
        
        /*for (CORBA::ULong i = 0; i < this->other_processes_.in ().length () ; ++i)
        {
            if (strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
            {
                try
                {
                    return this->other_processes_.in ()[i]->call_process_with (process_action,
                                                                    process_target,
                                                                    call_with,
                                                                    seq_no,
                                                                    process_source);
                }
    #ifdef ACE_VERSION // TAO
                catch (const CORBA::Exception& ex)
                {
                    fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::call_process_with from %s"
                                     " to %s with action %s\n", __FILE__, __LINE__, process_source,
                                     process_target, process_action);
                    ex._tao_print_exception ("");
                    throw;
                }
    #endif
                catch (...)
                {
                    fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::call_process_with from %s"
                                     " to %s with action %s\n", __FILE__, __LINE__, process_source,
                                     process_target, process_action);
                    throw;
                }
            }
        }

        fprintf (stderr, "ERROR %s (%d): No target found for ::call_process_with from %s"
                         " to %s with action %s (#%d)\n", __FILE__, __LINE__, process_source,
                                 process_target, process_action, seq_no);
        throw "";*/
    }

    ::CORBA::Long
    ProcessControlImpl::get_process_status (
                const char * process_target)
    {
        if (process_target == 0)
            process_target = this->own_name_.in ();

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            target = this->get_named_process_target (process_target);
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::get_process_status"
                         " of %s\n", __FILE__, __LINE__,
                                 process_target);
            throw "";
            // should be return false ?
        }
        else
        {
            try
            {
                return this->process_self_.in ()->get_process_status (process_target);
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::get_process_status"
                    " of %s\n", __FILE__, __LINE__, 
                                 process_target);
                ex._tao_print_exception ("");
                return -1;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::get_process_status"
                    " of %s\n", __FILE__, __LINE__, 
                                 process_target);
                return -1;
            }
        }
        
        /*for (CORBA::ULong i = 0; i < this->other_processes_.in ().length () ; ++i)
        {
            if (strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
            {
                try
                {
                    return this->other_processes_.in ()[i]->get_process_status (process_target);
                }
    #ifdef ACE_VERSION // TAO
                //catch (const CORBA::Exception& /*ex ) // acceptable if the remote is just not ready
                //{
                //    fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::get_process_status"
                //                     " of process %s\n", __FILE__, __LINE__,
                //                     process_target);
                //    ex._tao_print_exception ("");
               /*     return -1;
                //}
    #endif
                catch (...)
                {
                    fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::get_process_status"
                                     " of %s\n", __FILE__, __LINE__,
                                     process_target);
                    return -1;
                }
            }
        }

        fprintf (stderr, "ERROR %s (%d): No target found for ::get_process_status"
                         " of %s\n", __FILE__, __LINE__,
                                 process_target);
        throw "";*/
    }

    CORBA::Boolean 
    ProcessControlImpl::set_process_status (
                ::CORBA::Long process_status,
                const char * process_target)
    {
        if (process_target == 0)
            process_target = own_name_.in ();

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            target = this->get_named_process_target (process_target);
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::set_process_status"
                         " of %s to %d\n", __FILE__, __LINE__,
                                 process_target, process_status);
            throw "";
            // should be return false ?
        }
        else
        {
            try
            {
                return target.in ()->set_process_status (process_status, process_target);
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::set_process_status"
                    " of %s to %d\n", __FILE__, __LINE__, 
                                 process_target, process_status);
                ex._tao_print_exception ("");
                throw;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::set_process_status"
                    " of %s to %d\n", __FILE__, __LINE__, 
                                 process_target, process_status);
                throw;
            }
        }
        
        /*for (CORBA::ULong i = 0; i < this->other_processes_.in ().length (); ++i)
        {
            if (strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
            {
                try
                {
                    return this->other_processes_.in ()[i]->set_process_status (process_status, process_target);
                }
    #ifdef ACE_VERSION // TAO
                catch (const CORBA::Exception& ex)
                {
                    fprintf (stderr, "ERROR %s (%d): CORBA Exception trying ::set_process_status"
                                     " of process %s to %d\n", __FILE__, __LINE__,
                                     process_target, process_status);
                    ex._tao_print_exception ("");
                    throw;
                }
    #endif
                catch (...)
                {
                    fprintf (stderr, "ERROR %s (%d): Unknown exception trying ::set_process_status"
                                     " of %s to %d\n", __FILE__, __LINE__,
                                     process_target, process_status);
                    throw;
                }
            }
        }

        fprintf (stderr, "ERROR %s (%d): No target found for ::set_process_status"
                         " of %s to %d\n", __FILE__, __LINE__,
                                 process_target, process_status);*/
        return false;
    }

    void 
    ProcessControlImpl::shutdown (
                const char * process_target)
    {
        if (process_target == 0)
            process_target = own_name_.in ();

        /*CORBA::Boolean all = false;
        CORBA::Boolean all_other = false;
        CORBA::Boolean found = false;
        if (strcmp (process_target, ALL_PROCESSES) == 0)
        {
            all = true;
        }
        else if (strcmp (process_target, ALL_OTHER_PROCESSES) == 0)
        {
            all_other = false;
        }

        for (CORBA::ULong i = 0; (all || all_other || ! found) 
                                && i < this->other_processes_.in ().length () ; ++i)
        {
            if (all || all_other || strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
            {
                try
                {
                    this->other_processes_.in ()[i]->shutdown (this->other_process_names_.in ()[i]);
                }
    #ifdef ACE_VERSION // TAO
                catch (const CORBA::Exception& ex)
                {
                    fprintf (stderr, "ERROR %s (%d): CORBA Exception trying to ::shutdown %s"
                                      "\n", __FILE__, __LINE__, this->other_process_names_.in ()[i]);
                    ex._tao_print_exception ("");
                    throw;
                }
    #endif
                catch (...)
                {
                    fprintf (stderr, "ERROR %s (%d): Unknown exception trying to ::shutdown %s"
                                      "\n", __FILE__, __LINE__, this->other_process_names_.in ()[i]);
                    throw;
                }
                found = true;
            }
        }
        */

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            target = this->get_named_process_target (process_target);
            
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::shutdown of %s"
                    "\n", __FILE__, __LINE__,
                                 process_target);
            throw "";
            // should be return false ?
        }
        else
        {

            try
            {
                CORBA::PolicyList_var unused;
                target.in ()->_validate_connection (unused);
                target.in ()->shutdown (this->own_name_.in ());
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                fprintf (stderr, "ERROR %s (%d): CORBA Exception trying to ::shutdown %s"
                    "\n", __FILE__, __LINE__, this->own_name_.in ());
                ex._tao_print_exception ("");
                throw;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying to ::shutdown %s"
                    "\n", __FILE__, __LINE__, this->own_name_.in ());
                throw;
            }
        }

        /*if (!found)
        {
            fprintf (stderr, "ERROR %s (%d): No target found for ::shutdown of %s"
                    "\n", __FILE__, __LINE__,
                                 process_target);
            throw "";
        }*/
    }

    ::CORBA::Boolean
    ProcessControlImpl::wait_for_process_state (
                                            ::CORBA::Long process_status,
                                            const char * process_target,
                                            ::CORBA::ULongLong time_t_timeout)
    {
        if (process_target == 0)
            process_target = own_name_.in ();
        
        // Dumb waiting - @todo better
        CORBA::ULongLong expiry_time;
        if (time_t_timeout == FOREVER)
        {
            expiry_time = FOREVER;
        }
        else
        {
            expiry_time = this->time_now () + time_t_timeout;
        }

        ::OSPLTestLib::RemoteProcessServer_var target = ::OSPLTestLib::RemoteProcessServer::_nil ();

        if (strcmp (process_target, this->own_name_.in ()) == 0)
        {
            target = ::OSPLTestLib::RemoteProcessServer::_duplicate (this->process_self_.in ());
        }
        else
        {
            /*CORBA::Boolean found = false;
            for (CORBA::ULong i = 0; ! found 
                                && i < this->other_processes_.in ().length () ; ++i)
            {
                if (strcmp (process_target, this->other_process_names_.in ()[i]) == 0)
                {
                    target = ::OSPLTestLib::RemoteProcess::_duplicate 
                        (this->other_processes_.in ()[i]);
                    found = true;
                }
            }*/
            target = this->get_named_process_target (process_target);
        }

        if (::CORBA::is_nil (target.in ()))
        {
        
            fprintf (stderr, "ERROR %s (%d): No target found for ::wait_for_process_state %d of %s"
                    "\n", __FILE__, __LINE__,
                                 process_status, process_target);
            throw "";
            // should be return false ?
        }

        
        //os_time sleep_for;
        //sleep_for.tv_sec = 0;
        //sleep_for.tv_nsec = 100000000U; // 1/10 sec

        ACE_Time_Value sleep_time (0, 100000000U); // 1/10 sec

        CORBA::Boolean result = false;
        do
        {
            try
            {
                ::CORBA::Long current_status = target->get_process_status (process_target);
                //fprintf (stdout, "Info ::wait_for_process_state"
                //    "of %d. Current status %d for %s\n", process_status, current_status, process_target);
                result = (current_status == process_status);
            }
    #ifdef ACE_VERSION // TAO
            catch (const CORBA::Exception& ex)
            {
                // Probably not an issue - @todo remove ?
                fprintf (stderr, "Warning %s (%d): CORBA Exception trying to ::wait_for_process_state"
                    "of %d for %s\n", __FILE__, __LINE__, process_status, process_target);
                ex._tao_print_exception ("");
                // throw;
            }
    #endif
            catch (...)
            {
                fprintf (stderr, "ERROR %s (%d): Unknown exception trying to ::wait_for_process_state"
                    " of %d for %s\n", __FILE__, __LINE__, process_status, process_target);
                throw;
            }

            if (!result && this->time_now () < expiry_time)
            {
                // Wait up to sleep_time for something to do (e.g. maybe a status update)
                CORBA::Boolean pending = this->the_orb_->work_pending (sleep_time);
                
                if (pending)
                {
                    // ... then do it
                    this->the_orb_->perform_work();
                }
            }
            else
            {
                return result;
            }
        }
        while (1); 

        return result;
    }

    void 
    ProcessControlImpl::set_process_self (::OSPLTestLib::RemoteProcessServer_ptr process_self)
    {
        this->servant_->set_process_self (process_self);
    }
                        
    void 
    ProcessControlImpl::park_thread_to_do_work ()
    {
        the_orb_.in ()->run ();
    }

    ::CORBA::ULongLong
    ProcessControlImpl::time_now ()
    {
        // This is heavily TAOistic - @todo check out OS abstraction for local time
        ACE_Time_Value time_val = ACE_OS::gettimeofday ();
        ::CORBA::ULongLong sec_part  = ((::CORBA::ULongLong)time_val.sec ()) * 10000000;
        ::CORBA::ULongLong usec_part = ((::CORBA::ULongLong)time_val.usec ()) * 10;
        return (sec_part + usec_part);
    }

    ::OSPLTestLib::RemoteProcessServer_ptr
    ProcessControlImpl::get_named_process_target (const char* process_name)
    {
        try
        {
            ::CORBA::Object_var ref 
                = this->the_orb_.in ()->resolve_initial_references (process_name);

            if (::CORBA::is_nil (ref.in ()))
            {
                fprintf (stderr, "ERROR %s (%d): No target found in ::get_named_process_target for process named %s\n", 
                                                __FILE__, __LINE__, process_name);
                
            }
            else
            {
                return ::OSPLTestLib::RemoteProcessServer::_unchecked_narrow (ref.in ());
            }
        }
    #ifdef ACE_VERSION // TAO
        catch (const CORBA::Exception& ex)
        {
            fprintf (stderr, "ERROR %s (%d): CORBA Exception in ::get_named_process_target for process named %s\n", 
                                                __FILE__, __LINE__, process_name);
            ex._tao_print_exception ("");
        }
    #endif
        catch (...)
        {
            fprintf (stderr, "ERROR %s (%d): Unknown Exception in ::get_named_process_target for process named %s\n", 
                                                __FILE__, __LINE__, process_name);
        }

        return ::OSPLTestLib::RemoteProcessServer::_nil ();
    }

}


