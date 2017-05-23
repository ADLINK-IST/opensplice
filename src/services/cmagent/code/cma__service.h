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
#ifndef CMA__SERVICE_H_
#define CMA__SERVICE_H_

#include "cma__object.h"
#include "u_participant.h"

#include "vortex_agent.h"

#ifndef NDEBUG
#define cma_service(o) (assert(cma__objectKind(cma_object(o)) == CMA_OBJECT_SERVICE), (cma_service)(o))
#else
#define cma_service(o) ((cma_service)(o))
#endif /* NDEBUG */

#define cma_serviceFree(s) cma_objectFree(s);

const os_char*
cma_serviceName(
    cma_service _this) __nonnull_all__ __attribute_returns_nonnull__;

u_service
cma_serviceService(
    cma_service _this) __nonnull_all__ __attribute_returns_nonnull__;

#define cma_serviceParticipant(_this) u_participant(cma_serviceService(_this))

#endif /* CMA__SERVICE_H_ */
