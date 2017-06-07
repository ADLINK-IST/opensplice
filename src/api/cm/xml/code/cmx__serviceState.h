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
/**@file api/cm/xml/code/cmx__serviceState.h
 *
 * Offers internal routines on a serviceState.
 */
#ifndef CMX__SERVICESTATE_H
#define CMX__SERVICESTATE_H

#include "c_typebase.h"
#include "v_service.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Initializes the serviceState specific part of the XML representation of the
 * supplied kernel serviceState. This function should only be used by the
 * cmx_entityNewFromWalk function.
 *
 * @param entity The entity to create a XML representation of.
 * @return The serviceState specific part of the XML representation of the
 *         entity.
 */
c_char*             cmx_serviceStateInit            (v_serviceState entity);

/**
 * Constructs the string representation of the supplied serviceStateKind.
 *
 * @param stateKind The kernel state kind.
 * @return The string representation of the supplied kind.
 */
const c_char*       cmx_serviceStateKindToString    (v_serviceStateKind stateKind);

/**
 * Constructs the kernel stateKind representation of the supplied string.
 *
 * @param stateKind The string representation of the kind.
 * @return The matching stateKind.
 */
v_serviceStateKind  cmx_serviceStateKindFromString  (const c_char* stateKind);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__SERVICESTATE_H */
