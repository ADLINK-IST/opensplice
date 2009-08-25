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
#include "v_kernel.h"

gapi_returnCode_t
gapi_domain_create_persistent_snapshot (
    gapi_domain _this,
    const gapi_char * partition_expression,
    const gapi_char * topic_expression,
    const gapi_char * URI)
{
    gapi_returnCode_t result;

    result = GAPI_RETCODE_UNSUPPORTED;

    return result;
}


