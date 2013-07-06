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
 * @file services/cmsoap/include/cms_client.h
 * 
 * Represents a thread that is associated with a client connected to the 
 * cmsoap service. 
 */
#ifndef CMS_CLIENT_H
#define CMS_CLIENT_H

#include "cms__typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define cms_client(a) ((cms_client)(a))

/**
 * Constructs a new thread that is associated with a specific client that is
 * connected to the cmsoap service. The thread is able to handle client soap
 * requests. This routine does NOT start the thread. The thread must be 
 * started using the cms_clientStart routine.
 * 
 * A client thread is normally created and started by the cmsoap service.
 * 
 * @param ip The IP address of the client.
 * @param service A reference to the cmsoap service. This is used when the
 *                client disconnects and the thread deregisters itself with the
 *                service.
 * @return The thread that is able to handle soap requests.
 */
cms_client  cms_clientNew           (unsigned long ip, 
                                     cms_service service,
                                     const c_char* uri);

/**
 * Starts the supplied client thread. When successfull the thread is running
 * and is able to handle requests.
 * 
 * @param client The client thread to start.
 * @return Whether or not the starting of the thread succeeded. If successfull,
 *         TRUE is returned, FALSE otherwise.
 */
c_bool      cms_clientStart         (cms_client client);

/**
 * Triggers the thread to terminate, awaits its termination and frees it 
 * afterwards.
 * 
 * @param client The client to free.
 */
void        cms_clientFree          (cms_client client);

/**
 * Assign the supplied request to the supplied client thread.
 * If the current number of threads < the maximum number of threads, the 
 * request is handled immediately. If current number of threads == maximum 
 * number of threads, the request is placed in a queue and will be handled when
 * it is on top of the queue and the current number of threads < the maximum
 * number of threads. If the service has triggered the client thread that it
 * is shutting down, the request is not accepted.
 * 
 * @param client The client thread that must handle the request.
 * @param request The request that muist be handled by the client thread.
 * @return TRUE if the client accepted the request, FALSE otherwise.
 */
c_bool      cms_clientHandleRequest (cms_client client, 
                                     struct soap* request);

/**
 * Raises the client reference count by one. The client thread terminates
 * when the reference count becomes zero.
 * 
 * @param client The client thread, which reference count to raise.
 */
void        cms_clientAdd           (cms_client client);

/**
 * Lowers the client reference count by one. The client thread terminates
 * when the reference count becomes zero.
 * 
 * @param client The client thread, which reference count to lower.
 */
void        cms_clientRemove        (cms_client client);

/**
 * Function to determine whether the client has already called the initialize
 * routine in case the client lease has expired, but the client thinks it
 * has still access. 
 * 
 * @param client The client to check.
 * @return TRUE if initialized, FALSE otherwise.
 */
c_bool      cms_clientInitialized   (cms_client client);

#endif
