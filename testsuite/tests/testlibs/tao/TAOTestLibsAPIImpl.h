#ifndef TAO_TEST_LIBS_API_IMPL_H
#define TAO_TEST_LIBS_API_IMPL_H

#include "../TestLibsAPI_export.h"
#include "../TestLibsAPIS.h"

#include "RemoteProcessServant.h"

namespace OSPLTestLib
{
    class RemoteProcessServant;
    /**
    * @brief The implementation of the local process controller / signaller API thing.
    *
    * This class is the means by which a particular test process communicates with other OSPL test processes.
    * N.B. A POSIX process may hold one or more OSPL test processes. Each OSPL test process has exactly one
    * ProcessControl instance and is identified by a unique name.
    *
    */
    class OSPL_TESTLIBS_API_Export ProcessControlImpl
        : public virtual ::OSPLTestLib::ProcessControl
    {
    public:
        /**
        * @brief Constructor
        * Doesn't do much. All the work goes on in init()
        * @see init ()
        */
        ProcessControlImpl (const char* process_name);

        /**
        * @brief Destructor
        */
        ~ProcessControlImpl ();

        /**
        * @brief Initialisation method
        */
        ::CORBA::Boolean init (int argc, char *argv[]);

        /**
        * @brief Default convenience empty data sequence for use as arg and return value when 
        * no data is required or cared about.
        */
        static ::OSPLTestLib::DataSeq NO_DATA_ARG;


        // API methods

        /**
        * @brief Make a call to another process, optionally passing some data, and then continuing immediately it's been sent.
        * This is a 'oneway' call so once the request has been despatched control returns to the test straight away.
        * @param process_action Some string that be used to identify the action that is required from the other test process.
        * @param process_target Which other named 'process' this request is destined for. 
        * @param poke_with Some data of interest to the other process. Defaults to an empty sequence.
        * @param seq_no A sequence number, gretaer than 0, identifying this request. Should be a unique value within the life of this process. Defaults
        * to some value that is.
        * @param process_source The orginating process name of this call. Defaults to the name of this process if not passed.
        */
        virtual void poke_process_with (
            const char * process_action,
            const char * process_target,
            const ::OSPLTestLib::DataSeq & poke_with = NO_DATA_ARG,
            ::CORBA::ULong seq_no = 0,
            const char * process_source = 0);

        /**
        * @brief Make a call to another process, optionally passing some data, and then block until a reply is received.
        * This is a 'twoway' call so once the request has been despatched the thread blocks until a reply or exception
        * is received. The request will block up for up to ProcessControlImpl::default_timeout_
        * @param process_action Some string that be used to identify the action that is required from the other test process.
        * @param process_target Which other named 'process' this request is destined for. As a reply is required this must be axactly one process only.
        * @param poke_with Some data of interest to the other process. Defaults to an empty sequence.
        * @param seq_no A sequence number identifying this request. Should be a unique value within the life of this process. Defaults
        * to some value that is.
        * @param process_source The orginating process name of this call. Defaults to the name of this process.
        */
        virtual ::OSPLTestLib::DataSeq * call_process_with (
            const char * process_action,
            const char * process_target,
            const ::OSPLTestLib::DataSeq & call_with= NO_DATA_ARG,
            ::CORBA::ULong seq_no = 0,
            const char * process_source = 0);

        /**
        * @brief Get the value of a process state.  
        * @param process_target Which named 'process' this request is destined for. Defaults to this process if not supplied.
        * @return The process state - an integer value. -1 indicates not found / not set.
        */
        virtual ::CORBA::Long get_process_status (
            const char * process_target = 0);

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
            const char * process_target = 0);

        /**
        * @brief Instruct a process (or processes) to shutdown.
        * This is a 'oneway' call so once the request has been despatched control returns to the test pretty much straight away.
        * The other process should do its best to shutdown in an orderly manner. There are no guarantees however.
        * @param process_target Which other named 'process' this request is destined for.
        */
        virtual void shutdown (
            const char * process_target = 0);

        /**
        * @brief Wait until desired state is reached or a timeout occurs.
        * The thread may be used by the ProcessControl to do other work in the meantime.
        * @param process_state The desired status
        * @param process_target Which process are we waiting for. Defaults to ourself.
        * @param What is the max time we will potentially wait for. Defaults to ProcessControlImpl::default_timeout_
        * @return True if the status has been encountered before the timeout; false otherwise.
        */
        virtual ::CORBA::Boolean wait_for_process_state (
                                        ::CORBA::Long process_status,
                                        const char * process_target = 0,
                                        ::CORBA::ULongLong time_t_timeout = FOREVER);

        /**
        * @brief set test application call back methods
        * @param Pointer to an implemetation of RemoteProcess that is to receive the calls for this process.
        * @see 
        */
        virtual void set_process_self (::OSPLTestLib::RemoteProcessServer_ptr process_self);
        

        /**
        * @brief Surrender this thread to the ProcessControl to do whatver work the ProcessControl might have for it.
        * The thread will come back when the shutdown op has been called on the process.
        */
        virtual void park_thread_to_do_work ();
    
    protected:
        /**
        * @brief Get the time now in TimeT units
        * This is actually missing the absolute offset but as it's just used locally this doesn't matter.
        */
        ::CORBA::ULongLong time_now ();

        /**
        * Internal method to get named process target
        * @param process_name The porcess we want
        * @return A reference that con be used to communicate with the process
        * or nil and an error message reported to standard err. 
        */
        ::OSPLTestLib::RemoteProcessServer_ptr get_named_process_target (const char* process_name);

        /**
        * The default value for the maximum time we will wait for something to happen.
        * The default is forever.
        */
        ::CORBA::ULongLong default_timeout_;

        /** 
        * Unique id for each request of some sort made by this process. This means process name &
        * this value are a universally unique tuple. Otherwise, nothing else should be relied on.
        */
        ::CORBA::ULong sequence_no_;

        /**
        * The reference to this process
        */
        ::OSPLTestLib::RemoteProcessServer_var process_self_;

    // Public attributes - 
    public:
        /**
        * The string process name identifier for this test 'process'. This is required to
        * be unique within the known universe.
        */
        ::OSPLTestLib::ProcessName_var own_name_;

        /**
        * The sequence of references to all other processes.
        */
        //::OSPLTestLib::RemoteProcessSeq_var other_processes_;

        /**
        * The sequence of all process names that corresponds to entries in ProcessControlImpl::all_processes_
        */
        //::OSPLTestLib::ProcessNameSeq_var other_process_names_;
    private:
        // Stuff to shift to TAO only class when I split this
        ::CORBA::ORB_var the_orb_;

        // @todo - get rid of this.
        RemoteProcessServant* servant_;
    };

} /* End of namespace */

#endif /* TAO_TEST_LIBS_API_IMPL_H */
