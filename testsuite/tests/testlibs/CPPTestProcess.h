#ifndef OSPL_TESTLIBS_CPPTESTPROCESS_H
#define OSPL_TESTLIBS_CPPTESTPROCESS_H

#include "TestLibsAPI.h"

namespace OSPLTestLib
{
    /**
    * @brief A base class that can be used for C++ test procedures
    */
    class /*OSPL_TESTLIBS_API_Export*/ CPPTestProcess
    {
    public:
        CPPTestProcess (const char* process_name)
            : process_name_(CORBA::string_dup (process_name)), // unique string
#ifdef OTHER_PROCESS_NAME
              other_process_name_ (CORBA::string_dup ("OTHER_PROCESS_NAME")),
#else
              other_process_name_ (CORBA::string_dup ("")),
#endif
              process_controller_ (0)

        {
                default_domain_id_ = 0x7fffffff;
        }

        virtual ~CPPTestProcess ()
        {
        }

        /**
        * @brief Argument parsing and initialisation.
        */
        virtual void init (int argc, char *argv[])
        {
            // @todo ...
            ProcessName foo = process_name_.inout ();
            process_controller_ = TestLib::get_process_controller (foo, argc, argv);
        }

        /**
        * @brief To be implemented with the test procedure.
        */
        virtual int run () = 0;

        /**
        * @brief Returns the 'other' process name, *iff* this is a *two* process test.
        */
        const char* get_other_process_name ()
        {
            return CORBA::string_dup (other_process_name_.in ());
        }

        /**
        * @brief Sets the 'other' process name. For use *iff* this is a two process test.
        */
        void set_other_process_name (const char* process_name)
        {
            other_process_name_ = CORBA::string_dup (process_name);
        }

    protected:
        OSPLTestLib::ProcessName_var process_name_;

        ::CORBA::String_var other_process_name_;

        ProcessControlImpl * process_controller_;

        ::CORBA::Long default_domain_id_;
    };

}

#endif /* OSPL_TESTLIBS_CPPTESTPROCESS_H */
