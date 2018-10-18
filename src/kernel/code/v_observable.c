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
#include "v__observable.h"
#include "v__observer.h"
#include "v_proxy.h"
#include "v_public.h"
#include "v_event.h"
#include "vortex_os.h"

void v_observableBlock(v_observable o)   { c_mutexLock(&o->eventLock); }
void v_observableUnblock(v_observable o) { c_mutexUnlock(&o->eventLock); }
void v_observableLock(v_observable o)    { c_mutexLock(&o->mutex); }
void v_observableUnlock(v_observable o)  { c_mutexUnlock(&o->mutex); }

void
v_observableInit(
    v_observable o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    o->observers = NULL;
    (void)c_mutexInit(c_getBase(o), &o->mutex);
    (void)c_mutexInit(c_getBase(o), &o->eventLock);
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

c_bool
v_observableAddObserver(
    v_observable o,
    v_observer observer,
    v_eventMask mask,
    c_voidp userData)
{
    v_proxy proxy;
    v_handle handle;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    OSPL_ADD_EVENT_MASK(observer,mask);
    OSPL_BLOCK_EVENTS(o);
    handle = v_publicHandle(v_public(observer));
    proxy = o->observers;
    while (proxy) {
        assert(c_refCount(proxy) == 1);
        if (v_handleIsEqual(proxy->source, handle)) {
            proxy->eventMask |= mask;
            proxy->userData = userData;
            break;
        }
        proxy = proxy->next;
    }
    if (proxy == NULL) {
        /* add observer to the observer list */
        proxy = v_proxyNew(v_objectKernel(o), handle, userData);
        proxy->source2 = c_keep(observer);
        proxy->next = o->observers;
        proxy->eventMask = mask;
        o->observers = proxy;
        proxy = proxy->next;
    }
    OSPL_UNBLOCK_EVENTS(o);
    return TRUE;
}

c_bool
v_observableRemoveObserver(
    v_observable o,
    v_observer observer,
    v_eventMask mask,
    void** userData)
{
    v_proxy proxy, prev;
    v_handle handle;
    c_bool result = FALSE;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    OSPL_BLOCK_EVENTS(o);
    handle = v_publicHandle(v_public(observer));
    prev = NULL;
    proxy = o->observers;
    while (proxy) {
        assert(c_refCount(proxy) == 1);
        if (v_handleIsEqual(proxy->source, handle)) {
            /* Found the observer in the list so now remove and free it. */
            if (userData) {
                *userData = proxy->userData;
            }
            proxy->eventMask &= ~mask;
            if (proxy->eventMask == 0) {
                if (prev) {
                    prev->next = proxy->next;
                } else {
                    o->observers = proxy->next;
                }
                proxy->next = NULL;
                c_free(proxy);
            }
            result = TRUE;
            break; /* observer found and removed from the observer list => break out the loop */
        }
        /* proceed to the next observer in the list. */
        prev = proxy;
        proxy = proxy->next;
    }
    OSPL_UNBLOCK_EVENTS(o);
    return result;
}

#if 0
#define CLAIM(proxy, o) (V_HANDLE_OK == v_handleClaim(proxy->source,(v_object *)(o)));
#define RELEASE(proxy) (void) v_handleRelease(proxy->source)
#else
#define CLAIM(proxy, o) ((*(o) = proxy->source2) != NULL)
#define RELEASE(proxy)
#endif

void
v_observableNotify(
    v_observable _this,
    v_event event)
{
    v_proxy proxy, next;
    v_observer o;

    OSPL_BLOCK_EVENTS(_this);
    if (event) {
        proxy = _this->observers;
        while (proxy) {
            assert(c_refCount(proxy) == 1);
            next = proxy->next;
            /* CLAIM returns a ref counted object either via a handle or hard reference. */
            if (CLAIM(proxy, &o)) {
                /* Check interest and don't self-trigger to avoid endless looping. */
                if ((v_observable(o) != _this) && (event->kind & proxy->eventMask))
                {
                    /* TODO : Need to lock o if not already locked by myself */
                    OSPL_TRIGGER_EVENT(o, event, proxy->userData);
                    OSPL_THROW_EVENT(o, event);
                }
                RELEASE(proxy);
            }
            proxy = next;
        }
    }
    OSPL_UNBLOCK_EVENTS(_this);
}
