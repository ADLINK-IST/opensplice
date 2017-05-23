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

#ifndef V__OBSERVABLE_H
#define V__OBSERVABLE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_observable.h"

void
v_observableFree (
    v_observable _this);

/**
 * The initialisation of an observable object.
 * This method initialises all attributes of the observable class and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 * \param name  the name of the observable.
 */
void
v_observableInit (
    v_observable _this);

/**
 * The de-initialisation of an observable object.
 * This method releases all used resources by the observable object and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 */
void
v_observableDeinit (
    v_observable _this);

#define v_observableHasObservers(_this) \
        (v_observable(_this)->observers != NULL)

#if defined (__cplusplus)
}
#endif

#endif /* V__OBSERVABLE_H */
