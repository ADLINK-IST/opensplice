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

#ifndef V__WAITSET_H
#define V__WAITSET_H

#include "v_waitset.h"

#define v_waitsetLock(_this) \
        v_observerLock(v_observer(_this))

#define v_waitsetUnlock(_this) \
        v_observerUnlock(v_observer(_this))

/**
 * Releases all resources claimed by the referenced waitset object.
 *
 * \param _this  a reference to the waitset object.
 */
void
v_waitsetDeinit(
    v_waitset _this);

/**
 * Notifies the waitset on a state change of one of the conditions.
 * Every event the waitset receives is stored in a list. The events
 * are grouped on the origin of the event.
 *
 * \param _this    a reference to the waitset object.
 * \param e        a reference to the event describing the reason of the
 *                 notification.
 * \param userData data that has to be forwarded with the given event.
 */
void
v_waitsetNotify(
    v_waitset _this,
    v_event e,
    c_voidp userData);

#endif
