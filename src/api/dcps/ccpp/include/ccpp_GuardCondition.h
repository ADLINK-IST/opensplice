#ifndef CCPP_GUARDCONDITION_H
#define CCPP_GUARDCONDITION_H

#include "ccpp.h"
#include "ccpp_Condition_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    typedef GuardConditionInterface_ptr GuardCondition_ptr;
    typedef GuardConditionInterface_var GuardCondition_var;

    class OS_DCPS_API GuardCondition
        : public virtual GuardConditionInterface,
          public ::DDS::Condition_impl
    {
    public:
        GuardCondition( );

        ~GuardCondition();

        virtual ::DDS::ReturnCode_t set_trigger_value (
            ::CORBA::Boolean value
        ) THROW_ORB_EXCEPTIONS;
    };
}

#endif /* GUARDCONDITION */
