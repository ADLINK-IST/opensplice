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

#ifndef U__CFATTRIBUTE_H
#define U__CFATTRIBUTE_H

#include "u__cfNode.h"
#include "u_cfAttribute.h"

#include "v_cfAttribute.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfAttribute) {
    C_EXTENDS(u_cfNode);
};

#define u_cfAttribute(o) ((u_cfAttribute)(o))

u_cfAttribute
u_cfAttributeNew(
    const u_participant participant,
    v_cfAttribute kAttribute);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFATTRIBUTE_H */
