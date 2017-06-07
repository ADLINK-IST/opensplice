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


#include "v_proxy.h"
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

v_object
v_proxyClaim(
    v_proxy p)
{
    v_object o;

    if (p == NULL) {
        OS_REPORT(OS_WARNING,"Kernel Proxy",V_RESULT_ILL_PARAM,"No proxy specified to access");
        return NULL;
    }
    v_handleClaim(p->source,&o);
    return o;
}

void
v_proxyRelease(
    v_proxy p)
{
    if (p == NULL) {
        OS_REPORT(OS_WARNING,"Kernel Proxy",V_RESULT_ILL_PARAM,"No proxy specified to access");
        return;
    }
    v_handleRelease(p->source);
}

v_handle
v_proxyHandle(
    v_proxy p)
{
    return p->source;
}
