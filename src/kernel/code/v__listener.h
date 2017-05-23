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

#ifndef V__LISTENER_H
#define V__LISTENER_H

#include "v_listener.h"

#define v_listenerLock(_this) \
        v_observerLock(v_observer(_this))

#define v_listenerUnlock(_this) \
        v_observerUnlock(v_observer(_this))

/**
 * Releases all resources claimed by the referenced listener object.
 *
 * \param _this  a reference to the listener object.
 */
void
v_listenerDeinit(
    v_listener _this);

void
v_listenerFlush(
    v_listener _this,
    v_eventMask events,
    c_voidp userData);

#endif
