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

/**
 * @file services/cmsoap/include/cmsoap.h
 *
 * Main class for the Control & Monitoring SOAP service.
 */
#ifndef CMSOAP_H
#define CMSOAP_H

#include "vortex_os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_CMSOAP
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Starts the Control & Monitoring SOAP service.
 *
 * @param argc The number of arguments.
 * @param argv The list of arguments. The service expects one argument; the
 *             uri where to attach to.
 * @return If the execution was successfull 0 is returned, if not
 *         error is returned.
 */

OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_cmsoap);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
