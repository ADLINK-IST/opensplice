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
#ifndef V_STATUSCONDITION_H
#define V_STATUSCONDITION_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \class v_statusCondition
 *
 * The statusCondition extends the <code>v_observer</code> class and can be used to wait
 * on statis changes.
 */

/**
 * \brief The <code>v_statusCondition</code> cast method.
 *
 * This method casts a kernel object to a <code>v_statusCondition</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_statusCondition</code> or one of its subclasses.
 */
#define v_statusCondition(o)   (C_CAST(o,v_statusCondition))

#include "v_kernel.h"
#include "v_event.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Creates a new statusCondition.
 *
 * \param entity a reference to the owner
 * \return       the reference to the newly created statusCondition.
 */
OS_API v_statusCondition
v_statusConditionNew(
    v_entity entity);

/**
 * Destroys the referenced statusCondition.
 *
 * \param _this a reference to the statusCondition object.
 */
OS_API void
v_statusConditionFree(
    v_statusCondition _this);

OS_API void
v_statusConditionDeinit(
   v_statusCondition _this);

OS_API void
v_statusConditionSetMask(
    v_statusCondition _this,
    v_eventMask mask);

OS_API v_eventMask
v_statusConditionGetTriggerValue (
    v_statusCondition _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
