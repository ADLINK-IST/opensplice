#ifndef APPLICATION_PROCESS_BASE_H
#define APPLICATION_PROCESS_BASE_H

#include "TestLibsAPI_export.h"
#include "TestLibsAPIC.h"
#include "TestLibsAPI.h"

namespace OSPLTestLib
{
    /**
    * @brief Base class for an application to extend and overload to get notifications from
    * the ProcessController, if it needs them.
    * The primary methods of interest are: ::poke_process_with() and ::call_process_with()
    *
    * Register via ProcessController::set_process_self()
    */
    class /* OSPL_TESTLIBS_API_Export */ ApplicationProcessBase
        : public virtual ::OSPLTestLib::RemoteProcess
    {
    public:
        // API methods

        /**
        * @brief You should overload this method with appropriate reactions
        * if your test uses it.
        * @see ProcessControlImpl::poke_process_with()
        */
        virtual void poke_process_with (
            const char * /* process_action */,
            const char * /* process_target */,
            const ::OSPLTestLib::DataSeq & /* poke_with */,
            ::CORBA::ULong /* seq_no */,
            const char * /* process_source */) {}

        /**
        * @brief You should overload this method with appropriate reactions
        * if your test uses it.
        * @see ProcessControlImpl::call_process_with()
        */
        virtual ::OSPLTestLib::DataSeq *  call_process_with (
            const char * /* process_action */,
            const char * /* process_target */,
            const ::OSPLTestLib::DataSeq & /* call_with */,
            ::CORBA::ULong /* seq_no */,
            const char * /* process_source */)
        {
            return & ProcessControlImpl::NO_DATA_ARG;
        }

        /**
        * @brief This will never get called so should not be overloaded
        * It's only here cos I'm too lazy to sub divide the IDL again.
        * @see ProcessControlImpl::get_process_status()
        * @return Is ignored (@todo Am I happy with this ?)
        */
        virtual ::CORBA::Long get_process_status (
            const char * /* process_target */)
        {
            return 0;
        };

        /**
        * @brief You need only overload this method if you wish to respond
        * at the point that you process status is set.
        * There is no need to add code to store the process status. ProcessControl handles that.
        * @see ProcessControlImpl::set_process_status()
        * @return Should always be true.
        */
        virtual CORBA::Boolean set_process_status (
            ::CORBA::Long /* process_status */,
            const char * /* process_target */)
        {
            return true;
        }

        /**
        * @brief Only overload if you need a notification BEFORE SHUTDOWN STARTS.
        * You should not normally need this.
        * @see ProcessControlImpl::shutdown()
        */
        virtual void shutdown (
            const char * /* process_target */) {};
    };

} /* End of namespace */

#endif /* APPLICATION_PROCESS_BASE_H */


