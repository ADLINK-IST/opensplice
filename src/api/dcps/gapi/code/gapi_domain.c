/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
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
#include "u_domain.h"
#include "gapi_object.h"
#include "gapi_domain.h"
#include "gapi_kernel.h"

C_STRUCT(_Domain) {
    C_EXTENDS(_Object);
    u_domain domain;
};

_Domain
_DomainNew(
    gapi_domainName_t domainId)
{
   _Domain _this = NULL;

   u_domain domain;
   u_result uResult;
   gapi_returnCode_t result;

   if (domainId)
   {
      _this = _DomainAlloc();
      if (_this)
      {
         uResult = u_domainOpen(&domain, (c_char*)domainId,1);/* 1 = timeout */
         result = kernelResultToApiResult(uResult);

         if (result == GAPI_RETCODE_OK)
	 {
	    _this->domain = domain;
         }
         else
         {
            _DomainFree(_this);
            _this = NULL;
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
        if(_this->domain)
        {
/*            u_domainFree(_this->domain);*/
            _this->domain = NULL;
        }
        _ObjectDelete((_Object)_this);

    }
}

u_domain
_DomainGetKernel(
     _Domain _this)
{
    u_domain result = NULL;
    if (_this)
    {
        result =  _this->domain;
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
            uResult = u_domainCreatePersistentSnapshot(
                domain->domain,
                partition_expression,
                topic_expression,
                uri);
            result = kernelResultToApiResult(uResult);
            _ObjectRelease((_Object)domain);
        }

    }
    return result;
}

