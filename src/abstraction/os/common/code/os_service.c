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
/** \file os/common/code/os_service.c
 *  \brief Common OS service
 *
 * Stub for OS service implementation for platforms
 * that don't need it.
 */
#include "os_init.h"

static os_boolean _ospl_singleProcess = OS_FALSE;

os_result
os_serviceStart(const char *name)
{
    (void) name;
    return os_resultSuccess;
}

os_result
os_serviceStop(void)
{
    return os_resultSuccess;
}

const char *
os_serviceName(void)
{
    return (const char *)0;
}

void
os_createPipeNameFromDomainName(const os_char *name)
{
    OS_UNUSED_ARG(name);
}

void
os_serviceSetSingleProcess (void)
{
    _ospl_singleProcess = OS_TRUE;
}

os_boolean
os_serviceGetSingleProcess (void)
{
    return _ospl_singleProcess;
}
