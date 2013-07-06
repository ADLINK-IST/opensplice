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
 * @file services/cmsoap/include/cms_soapThread.h
 * 
 * Represents a thread that is able to handle SOAP requests.
 */
#ifndef CMS_SOAPTHREAD_H
#define CMS_SOAPTHREAD_H

#include "cms__typebase.h"
#include <stdsoap2.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define cms_soapThread(a) ((cms_soapThread)(a))

/**
 * Creates a new SOAP thread that is able to handle C&M SOAP requests. The
 * created thread is not started. The thread must be started by calling the
 * cms_soapThreadStart routine.
 * 
 * A SOAP thread is normally created and started by a cms_client thread.
 * 
 * @param name The name for the thread.
 * @param client The client that creates the thread.
 * @return The created SOAP thread.
 * 
 */
cms_soapThread  cms_soapThreadNew           (const c_char* name, 
                                             cms_client client);
                                 
/**
 * Starts the SOAP thread.
 * 
 * @param thread The SOAP thread to start.
 * @return TRUE if starting succeeded, FALSE otherwise.
 */
c_bool          cms_soapThreadStart         (cms_soapThread thread);

/**
 * Triggers the thread to terminate, awaits its terminating and frees it.
 * 
 * @param thread The thread to terminate and free.
 */
void            cms_soapThreadFree          (cms_soapThread thread);

/**
 * Assigns the supplied request to the supplied thread. The thread will handle
 * the request.
 * 
 * @param thread The thread that must handle the request.
 * @param soap The request to handle.
 */
c_bool          cms_soapThreadHandleRequest (cms_soapThread thread,
                                             struct soap* soap);

/**
 * Updates the statistics of #requests handled by the service.
 * 
 * @param entity The cmsoap participant.
 * @param args The cms_service of the thread.
 */
void            cms_soapThreadStatisticsRequestHandledAdd   (v_entity entity,
                                                             c_voidp args);

#endif
