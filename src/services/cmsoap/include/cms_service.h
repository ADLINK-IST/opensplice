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
 * @param soap The SOAP request.
 * @return The client thread that is able to handle the request. In the 
 *         following cases NULL is returned:
 *         - The service is terminating.
 *         - The maximum number of clients is reached.
 */
cms_client  cms_serviceLookupClient (cms_service cms, struct soap* soap, const c_char* uri);

/**
 * Deregisters the supplied client thread from the supplied service.
 * 
 * @param cms The cmsoap service.
 * @param client The client thread.
 */
void        cms_serviceRemoveClient (cms_service cms, cms_client client);

#endif
