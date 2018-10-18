/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef U__SERVICE_H
#define U__SERVICE_H

#include "u_service.h"

/**
 * \brief Initialises the service component of the user layer.
 *
 * \param void
 */
void
u__serviceInitialise(
    void);

/**
 * \brief De-initialises the service component of the user layer.
 *
 * \param void
 */
void
u__serviceExit(
    void);

u_service
u_serviceNewWrapper(
    v_service kService,
    u_domain domain);

/**
 * \brief Initialises the proxy object to a service object.
 *
 * \param _this The service proxy to operate on.
 * \param service The kernel service belonging to this proxy.
 * \param domain The kernel domain belonging to this proxy.
 */
u_result
u_serviceInit(
    u_service _this,
    const v_service service,
    const u_domain domain);

/**
 * \brief De-initialises the proxy object to a service object.
 *
 * \param service The service proxy to operate on.
 */
u_result
u__serviceDeinitW(
    void *_this);

void
u__serviceFreeW(
    void *_this);

#endif
