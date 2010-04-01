/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "gapi.h"
#include "gapi_error.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "u_user.h"
#include "u_kernel.h"
#include "gapi_object.h"
#include "gapi_domain.h"
#include "gapi_kernel.h"

C_STRUCT(_Domain) {
    C_EXTENDS(_Object);
    gapi_domainId_t domainId;
    u_kernel kernel;
};

_Domain
_DomainNew(
    gapi_domainId_t domainId)
{
   _Domain _this = NULL;

   if (domainId)
   {
      _this = _DomainAlloc();
      if (_this)
      {
         _this->domainId = os_strdup(domainId);
         if(!_this->domainId)
         {
            _this->kernel = NULL;
            _DomainFree(_this);
            _this = NULL;
         } else
         {
            _this->kernel = u_userKernelOpen((c_char *)domainId, 1);/* 1 = timeout */
            if(!_this->kernel)
            {
               _DomainFree(_this);
               _this = NULL;
            }
         }
      }
   }

   return _this;
}

void
_DomainFree(
    _Domain _this)
{
    if(_this)
    {
        if(_this->domainId)
        {
            os_free(_this->domainId);
            _this->domainId = NULL;
        }
        if(_this->kernel)
        {
            u_userKernelClose(_this->kernel);
            _this->kernel = NULL;
        }
        _ObjectDelete((_Object)_this);

    }
}

gapi_domainId_t
_DomainGetDomainIdNoCopy(
    _Domain _this)
{
    gapi_domainId_t result;

    if(_this)
    {
        result = _this->domainId;
    } else
    {
        result = NULL;
    }
    return result;
}

gapi_returnCode_t
gapi_domain_create_persistent_snapshot (
    gapi_domain _this,
    const gapi_char * partition_expression,
    const gapi_char * topic_expression,
    const gapi_char * uri)
{
    gapi_returnCode_t result;
    _Domain domain;
    u_result uResult;

    if(!_this || !partition_expression || !topic_expression || !uri)
    {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else
    {
        result = GAPI_RETCODE_OK;
    }

    if(result == GAPI_RETCODE_OK)
    {
        domain = gapi_domainClaim(_this, &result);
        if(result == GAPI_RETCODE_OK)
        {
            uResult = u_kernelCreatePersistentSnapshot(
                domain->kernel,
                partition_expression,
                topic_expression,
                uri);
            result = kernelResultToApiResult(uResult);
            _ObjectRelease((_Object)domain);
        }

    }
    return result;
}

