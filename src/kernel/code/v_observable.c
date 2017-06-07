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
#include "v__observable.h"
#include "v__entity.h"
#include "v_observer.h"
#include "v_proxy.h"
#include "v_public.h"
#include "v_event.h"
#include "os_report.h"

#define _USE_HANDLE_

c_bool
v_observableAddObserver(
    v_observable o,
    v_observer observer,
    c_voidp userData)
{
    v_proxy proxy;
    v_handle handle;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    c_mutexLock(&o->mutex);
    handle = v_publicHandle(v_public(observer));
    proxy = o->observers;
    while (proxy) {
        if (v_handleIsEqual(proxy->source, handle)) break;
        proxy = proxy->next;
    }
    if (!proxy) {
        proxy = v_proxyNew(v_objectKernel(o), handle, userData);
        /* remnant of proto typing hard referenced proxy objects in stead of
         * using handles. Needs some clean-up activity.
         * Also see the undefined _USE_HANDLE_ macro.
         */
        proxy->source2 = observer;
        proxy->next = o->observers;
        o->observers = proxy;
    }
    proxy->userData = userData;
    c_mutexUnlock(&o->mutex);
    return TRUE;
}

c_bool
v_observableRemoveObserver(
    v_observable o,
    v_observer observer,
    void** userData)
{
    v_proxy proxy, prev;
    v_handle handle;
    c_bool result = FALSE;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    c_mutexLock(&o->mutex);
    handle = v_publicHandle(v_public(observer));
    prev = NULL;
    proxy = o->observers;
    while (proxy) {
        if (v_handleIsEqual(proxy->source, handle)) {
            if (userData) {
                *userData = proxy->userData;
            }
            if (prev) {
                prev->next = proxy->next;
            } else {
                o->observers = proxy->next;
            }
            proxy->next = NULL;
            c_free(proxy);
            result = TRUE;
            break;
        }
        prev = proxy;
        proxy = proxy->next;
    }
    c_mutexUnlock(&o->mutex);
    return result;
}

void
v_observableInit(
    v_observable o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    o->observers = NULL;
    c_mutexInit(c_getBase(o), &o->mutex);
    v_publicInit(v_public(o));
}

void
v_observableFree(
    v_observable o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    v_publicFree(v_public(o));
}

void
v_observableDeinit(
    v_observable o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    v_publicDeinit(v_public(o));
}

void
v_observableNotify(
    v_observable o,
    v_event event)
{
    v_proxy proxy, prev, next;
    v_observer ob;
    v_observer* oPtr;
    v_handleResult r;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    prev = NULL;
    c_mutexLock(&o->mutex);
    proxy = o->observers;
    while (proxy) {
        next = proxy->next;
#ifdef _USE_HANDLE_
        oPtr = &ob;
        r = v_handleClaim(proxy->source,(v_object *)oPtr);
        if (r == V_HANDLE_OK) {
#else
            ob = proxy->source2;
#endif
            if (v_observable(ob) == o) {
                v_observerNotify(ob,event,proxy->userData);
            } else {
                v_observerLock(ob);
                v_observerNotify(ob,event, proxy->userData);
                v_observerUnlock(ob);
            }
            prev = proxy;
#ifdef _USE_HANDLE_
            (void) v_handleRelease(proxy->source);
        } else {
            /* The source has already left the system */
            if (prev) {
                prev->next = next;
            } else {
                o->observers = next;
            }
            proxy->next = NULL;
            c_free(proxy);
        }
#endif
        proxy = next;
    }
    c_mutexUnlock(&o->mutex);
}
