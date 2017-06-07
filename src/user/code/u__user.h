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
#ifndef U__USER_H
#define U__USER_H

#include "u_user.h"
#include "u__domain.h"

#define OSRPT_CNTXT_USER "user layer"

typedef c_voidp u_domainActionArg;
typedef void (*u_domainAction)(u_domain domain, u_domainActionArg arg);

#define u_resultFromKernel(r) ((u_result)r)

u_result
u_userAddDomain(
    _In_ u_domain domain);

u_result
u_userRemoveDomain(
    _In_ u_domain domain);

c_address
u_userServer (
    _In_ c_ulong id);

c_ulong
u_userServerId (
    const v_public o);

void
u_userSetupSignalHandling (
    c_bool installExitRequestHandler);

os_uint32
u__userDomainIndex(
    _In_ u_domain domain);

u_result
u__userDomainDetach(
    _In_ os_uint32 idx,
    _In_ os_uint32 flags);

#endif

