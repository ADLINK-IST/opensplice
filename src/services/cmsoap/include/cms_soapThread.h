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
void            cms_soapThreadStatisticsRequestHandledAdd   (v_public entity,
                                                             c_voidp args);

#endif
