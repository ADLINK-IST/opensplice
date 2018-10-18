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
#ifndef V_OBSERVABLE_H
#define V_OBSERVABLE_H

/**
 * \class v_observable
 *
 * An observable object can be observed by observer objects. State changes of an
 * observable are notified to its observers by an event object. The event object
 * describes the reason of notification.
 */

/**
 * \brief The <code>v_observable</code> cast method.
 *
 * This method casts a kernel object to a <code>v_observable</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_observable</code> or one of its subclasses.
 */
#define v_observable(_this) (C_CAST(_this,v_observable))

/* Following is a temporary public declaration of v_observableAddObserver to support cmx legacy API.
 * The dependency on this operation must be removed so it can be made private as intended.
 */
#include "kernelModuleI.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API c_bool
v_observableAddObserver(
    v_observable _this,
    v_observer observer,
    v_eventMask mask,
    c_voidp userData);

#undef OS_API

#endif
