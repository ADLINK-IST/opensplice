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


#include "v_proxy.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_handle.h"
#include "os_report.h"

v_proxy
v_proxyNew(
    v_kernel kernel,
    v_handle handle,
    c_voidp userData)
{
    v_proxy proxy;

    proxy = v_proxy(v_objectNew(kernel,K_PROXY));
    proxy->source = handle;
    proxy->userData = userData;

    return proxy;
}

v_entity
v_proxyClaim(
    v_proxy p)
{
    v_object o;

    if (p == NULL) {
        OS_REPORT(OS_WARNING,"Kernel Proxy",0,"No proxy specified to access");
        return NULL;
    }
    v_handleClaim(p->source,&o);
    return v_entity(o);
}

void
v_proxyRelease(
    v_proxy p)
{
    if (p == NULL) {
        OS_REPORT(OS_WARNING,"Kernel Proxy",0,"No proxy specified to access");
        return;
    }
    v_handleRelease(p->source);
}

