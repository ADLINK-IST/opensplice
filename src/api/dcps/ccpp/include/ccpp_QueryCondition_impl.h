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
#ifndef CCPP_QUERYCONDITION_H
#define CCPP_QUERYCONDITION_H

#include "ccpp.h"
#include "ccpp_ReadCondition_impl.h"
#include "ccpp_DataReader_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API QueryCondition_impl
    : public virtual ::DDS::QueryCondition,
      public ::DDS::ReadCondition_impl
    {
    friend class ::DDS::DataReader_impl;
    friend class ::DDS::DataReaderView_impl;

    private:
        QueryCondition_impl(gapi_queryCondition handle);

    public:
      virtual char * get_query_expression (
      ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t get_query_parameters (
        ::DDS::StringSeq & query_parameters
    ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t set_query_parameters (
      const ::DDS::StringSeq & query_parameters
    ) THROW_ORB_EXCEPTIONS;

    };
    typedef QueryCondition_impl* QueryCondition_impl_ptr;
}

#endif /* QUERYCONDITION */
