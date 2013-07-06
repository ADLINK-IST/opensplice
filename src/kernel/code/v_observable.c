/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

typedef struct findProxyArgument {
    v_handle observer;
    v_proxy proxy;
} findProxyArgument;

static c_bool
findProxy(
    c_object o,
    c_voidp arg)
{
    v_proxy proxy = (v_proxy)o;
    findProxyArgument *a = (findProxyArgument *)arg;

    if (v_handleIsEqual(proxy->source,a->observer)) {
        a->proxy = proxy;
        return FALSE;
    } else {
        return TRUE;
    }
}

c_bool
v_observableAddObserver(
    v_observable o,
    v_observer observer,
    c_voidp userData)
{
    v_proxy proxy;
    findProxyArgument arg;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    c_mutexLock(&o->mutex);
    arg.observer = v_publicHandle(v_public(observer));
    arg.proxy = NULL;
    c_setWalk(o->observers,findProxy,&arg);
    if (arg.proxy == NULL) { /* no proxy to the observer exists */
        proxy = v_proxyNew(v_objectKernel(o),
                           arg.observer, userData);
/* remnant of proto typing hard referenced proxy objects in stead of
 * using handles. Needs some clean-up activity.
 * Also see the undefined _USE_HANDLE_ macro.
 */
proxy->source2 = observer;
        proxy->userData = userData;
        c_insert(o->observers,proxy);
        c_free(proxy);
    } else {
        arg.proxy->userData = userData; /* replace userData */
    }
    c_mutexUnlock(&o->mutex);
    return TRUE;
}

c_bool
v_observableRemoveObserver(
    v_observable o,
    v_observer observer,
    void** userData)
{
    v_proxy found;
    findProxyArgument arg;
    c_bool result = FALSE;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));
    assert(C_TYPECHECK(observer,v_observer));

    assert(o != v_observable(observer));

    c_mutexLock(&o->mutex);
    arg.observer = v_publicHandle(v_public(observer));
    arg.proxy = NULL;
    c_setWalk(o->observers,findProxy,&arg);
    if (arg.proxy != NULL) { /* proxy to the observer found */
        found = c_remove(o->observers,arg.proxy,NULL,NULL);
        assert(found == arg.proxy);
        if(found && userData)
        {
            *userData = found->userData;
        }
        c_free(found);
        result = TRUE;
    }
    c_mutexUnlock(&o->mutex);
    return result;
}

void
v_observableInit(
    v_observable o,
    const c_char *name,
    v_statistics s,
    c_bool enable)
{
    c_type proxyType;
    v_kernel kernel;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    kernel = v_objectKernel(o);
    proxyType = v_kernelType(kernel,K_PROXY);
    o->observers = c_setNew(proxyType);
    assert(o->observers);
    c_mutexInit(&o->mutex,SHARED_MUTEX);
    v_entityInit(v_entity(o), name, s, enable);
}

void
v_observableFree(
    v_observable o)
{
    v_proxy found;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    found = c_take(o->observers);
    while (found != NULL) {
        c_free(found);
        found = c_take(o->observers);
    }
    v_entityFree(v_entity(o));
}

void
v_observableDeinit(
    v_observable o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    v_entityDeinit(v_entity(o));
}

struct proxyNotifyArg {
    v_event event;
    v_observable myself;
    c_iter  deadProxies;
};

static c_bool
v_proxyNotify(
    c_object proxy,
    c_voidp arg)
{
    v_proxy p = v_proxy(proxy);
    struct proxyNotifyArg *a = (struct proxyNotifyArg *)arg;
    v_observer o;
    v_observer* oPtr;
#ifdef _USE_HANDLE_
    v_handleResult r;

    oPtr = &o;
    r = v_handleClaim(p->source,(v_object *)oPtr);
    if (r == V_HANDLE_OK) {
#else
        o = p->source2;
#endif
        if (v_observable(o) == a->myself) {
            v_observerNotify(o,a->event,p->userData);
        } else {
            v_observerLock(o);
            v_observerNotify(o,a->event, p->userData);
            v_observerUnlock(o);
        }
#ifdef _USE_HANDLE_
        r = v_handleRelease(p->source);
    } else {
        /* The source has already left the system */
        a->deadProxies = c_iterInsert(a->deadProxies, proxy);
    }
#endif
    return TRUE;
}

void
v_observableNotify(
    v_observable o,
    v_event event)
{
    struct proxyNotifyArg pna;
    v_proxy proxy, foundProxy;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observable));

    if(c_setCount(o->observers) > 0) {
        c_mutexLock(&o->mutex);
        pna.event = event;
        pna.myself = o;
        pna.deadProxies = NULL;
        c_setWalk(o->observers,v_proxyNotify,&pna);
#ifdef _USE_HANDLE_
        proxy = c_iterTakeFirst(pna.deadProxies);
        while (proxy != NULL) {
            foundProxy = c_remove(o->observers, proxy, NULL, NULL);
            assert(foundProxy == proxy);
            c_free(proxy);
            proxy = c_iterTakeFirst(pna.deadProxies);
        }
        c_iterFree(pna.deadProxies);
#endif
        c_mutexUnlock(&o->mutex);
    }
}
