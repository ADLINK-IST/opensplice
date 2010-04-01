#ifndef TAO_REMOTEPROCESSSERVANT_H
#define TAO_REMOTEPROCESSSERVANT_H

#include "../TestLibsAPI_export.h"
#include "../TestLibsAPIS.h"
#include "../TestLibsAPI.h"

namespace OSPLTestLib
{
    /**
    * @brief A CORBA servant that will be activated to provide a remotable reference.
    *
    * This manages each process state and delegates calls from other processes to the test
    * application local interface implementation.
    *
    */
    class OSPL_TESTLIBS_API_Export RemoteProcessServant
        : public virtual ::POA_OSPLTestLib::RemoteProcessServer
    {
    public:
        /**
        * @brief Constructor
        * Doesn't do much. All the work goes on in init()
        * @see init ()
        */
        RemoteProcessServant (const char* process_name,
                            ::CORBA::ORB_ptr an_orb);

        /**
        * @brief Destructor
        */
        ~RemoteProcessServant ();

        // API methods

        /**
        * @brief Make a call to another process, optionally passing some data, and then continuing immediately it's been sent.
        * This is a 'oneway' call so once the request has been despatched control returns to the test straight away.
        * @param process_action Some string that be used to identify the action that is required from the other test process.
        * @param process_target Which other named 'process' this request is destined for. See special values ALL_PROCESSES
        * and ALL_OTHER_PROCESSES
        * @param poke_with Some data of interest to the other process. Defaults to an empty sequence.
        * @param seq_no A sequence number, gretaer than 0, identifying this request. Should be a unique value within the life of this process. Defaults
        * to some value that is.
        * @param process_source The orginating process name of this call. Defaults to the name of this process if not passed.
        */
        virtual void poke_process_with (
            const char * process_action,
            const char * process_target,
            const ::OSPLTestLib::DataSeq & poke_with,
            ::CORBA::ULong seq_no,
            const char * process_source);

        /**
        * @brief Make a call to another process, optionally passing some data, and then block until a reply is received.
        * This is a 'twoway' call so once the request has been despatched the thread blocks until a reply or exception
        * is received. The request will block up for up to ProcessControlImpl::default_timeout_
        * @param process_action Some string that be used to identify the action that is required from the other test process.
        * @param process_target Which other named 'process' this request is destined for. As a reply is required this must be axactly one process only.
        * @param call_with Some data of interest to the other process. Defaults to an empty sequence.
        * @param seq_no A sequence number identifying this request. Should be a unique value within the life of this process. Defaults
        * to some value that is.
        * @param process_source The orginating process name of this call. Defaults to the name of this process.
        */
        virtual ::OSPLTestLib::DataSeq * call_process_with (
            const char * process_action,
            const char * process_target,
            const ::OSPLTestLib::DataSeq & call_with,
            ::CORBA::ULong seq_no,
            const char * process_source);

        /**
        * @brief Get the value of a process state.  
        * @param process_target Which named 'process' this request is destined for. Defaults to this process if not supplied.
        * @return The process state - an integer value. -1 indicates not found / not set.
        */
        virtual ::CORBA::Long get_process_status (
            const char * process_target);

        /**
        * @brief Set the value of a process state.  
        * @param process_target Which named 'process' this request is destined for. Defaults to this process if not supplied.
        * @param process_status The integer status value to set. This has no particular predefined meaning right now. 
        * Tests should define their own values. 0 and -1 are reserved and it's probably a good idea to go sequentially
        * through states rather than revisit values that have been used already in a test. Up to you though.
        * @return True is the status was successfully set. False otherwise.
        */
        virtual CORBA::Boolean set_process_status (
            ::CORBA::Long process_status,
            const char * process_target);

        /**
        * @brief Instruct a process (or processes) to shutdown.
        * This is a 'oneway' call so once the request has been despatched control returns to the test pretty much straight away.
        * The other process(es) should do there best to shutdown in an orderly manner. There are no guarantees however.
        * @param process_target Which other named 'process' this request is destined for. See special values ALL_PROCESSES
        * and ALL_OTHER_PROCESSES. If the value is ALL_PROCESSES then this process should try and shut others down then
        * itself for obvious reasons.
        */
        virtual void shutdown (
            const char * process_target);

        
        /**
        * @brief set test application call back methods
        * @param Pointer to an implemetation of RemoteProcess that is to receive the calls for this process.
        * @todo move up to this interface
        */
        virtual void set_process_self (::OSPLTestLib::RemoteProcessServer_ptr process_self);
        

        // /**
        // * @brief Surrender this thread to the ProcessControl to do whatver work the ProcessControl might have for it.
        // * The thread will come back when the shutdown op has been called on the process.
        // */
        // virtual void park_thread_to_do_work ();
        
    // Public attributes - 
    public:
        /**
        * The string process name identifier for this test 'process'. This is required to
        * be unique within the known universe.
        */
        ::OSPLTestLib::ProcessName_var own_name_;

    private:
        /**
        * @brief ORB. 
        */
        ::CORBA::ORB_var the_orb_;

        /**
        * @brief The reference to this process application supplied call
        * back object. May be nil.
        */
        ::OSPLTestLib::RemoteProcessServer_var process_self_;

        /**
        * @brief The process status
        * Initial notional state is -1. As soon as the servanyt is activated it starts from 0.
        */
        ::CORBA::Long process_status_;
    };

} /* End of namespace */

#endif /* TAO_REMOTEPROCESSSERVANT_H */
