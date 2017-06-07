/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
    /* It is possible (depending on environment variables for instance) that the
     * os_reportInit() has to do slightly different things when it is initialized
     * by a single process setup. So, do it again in that case. */
    os_reportInit(OS_TRUE);
}

os_boolean
os_serviceGetSingleProcess (void)
{
    return _ospl_singleProcess;
}
