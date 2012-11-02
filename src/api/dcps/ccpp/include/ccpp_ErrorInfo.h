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
#ifndef CCPP_ERRORINFO_H
#define CCPP_ERRORINFO_H

#include "ccpp.h"
#include "gapi.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

  typedef ErrorInfoInterface_ptr ErrorInfo_ptr;
  typedef ErrorInfoInterface_var ErrorInfo_var;

  class OS_DCPS_API ErrorInfo
    : public virtual ::DDS::ErrorInfoInterface,
      public LOCAL_REFCOUNTED_OBJECT
    {
    private:
        gapi_errorInfo _gapi_self;

    public:

        ErrorInfo( void );
       ~ErrorInfo( void );

        virtual ::DDS::ReturnCode_t update(
            void
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_code(
            ::DDS::ErrorCode_t & code
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_message(
           char *& message
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_location(
            char * & location
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_source_line(
            char * & source_line
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_stack_trace(
            char * & stack_trace
        ) THROW_ORB_EXCEPTIONS;

    };
}

#endif /* ERRORINFO */
