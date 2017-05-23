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
 * @file services/cmsoap/include/cms_service.h
 *
 * Represents the control & monitoring soap service (cmsoap).
 */
#ifndef CMS_SERVICE_H
#define CMS_SERVICE_H

#include "cms__typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define cms_service(a) ((cms_service)(a))

/**
 * Constructs a new control & monitoring SOAP service:
 * - Construct a user layer service.
 * - Initializes configuration.
 * - Start lease update thread.
 * - Initialize SOAP environment
 *
 * @param uri The uri where the domain configuration will be read from.
 */
cms_service cms_serviceNew          (const c_char* name, const c_char* uri);

/**
 * Terminates and frees the supplied service.
 *
 * @param cms The service to terminate and free.
 */
void        cms_serviceFree         (cms_service cms);

/**
 * Resolves a thread for a specific client request:
 * 1. The service checks if it is terminating. If so, it returns NULL. If not
 *    see (2).
 * 2. The service resolves the client IP address from the request.
 * 3. The service checks if a client thread exists for this IP address. If so,
 *    it is returned to the calling thread. If not see (4).
 * 4. The service checks if the maximum number of clients is reached. If so,
 *    it returns NULL to the calling thread. If not, see (5).
 * 5. The service creates a new client thread and returns it to the calling
 *    thread.
 *
 * @param cms The cmsoap service.
 * @return The client thread that is able to handle the request. In the
 *         following cases NULL is returned:
 *         - The service is terminating.
 *         - The maximum number of clients is reached.
 */
cms_client  cms_serviceLookupClient (cms_service cms);

/**
 * Deregisters the supplied client thread from the supplied service.
 *
 * @param cms The cmsoap service.
 * @param client The client thread.
 */
void        cms_serviceRemoveClient (cms_service cms, cms_client client);

#endif
