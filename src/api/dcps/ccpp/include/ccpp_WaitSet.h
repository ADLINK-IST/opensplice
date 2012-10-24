/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef CCPP_WAITSET_H
#define CCPP_WAITSET_H

#include "ccpp.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

  typedef WaitSetInterface_ptr WaitSet_ptr;
  typedef WaitSetInterface_var WaitSet_var;

  class OS_DCPS_API WaitSet
    : public virtual ::DDS::WaitSetInterface,
      public LOCAL_REFCOUNTED_OBJECT
    {
    private:
        gapi_waitSet _gapi_self;

    public:

        WaitSet( void );
       ~WaitSet( void );

        virtual ::DDS::ReturnCode_t wait (
            ::DDS::ConditionSeq & active_conditions,
            const ::DDS::Duration_t & timeout
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t attach_condition (
            ::DDS::Condition_ptr cond
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t detach_condition (
            ::DDS::Condition_ptr cond
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_conditions (
            ::DDS::ConditionSeq & attached_conditions
        ) THROW_ORB_EXCEPTIONS;

    };
}

#endif /* WAITSET */
