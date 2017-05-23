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

#ifndef U__DOMAIN_H
#define U__DOMAIN_H

#include "u_participant.h"
#include "u_domain.h"
#include "u_entity.h"
#include "vortex_os.h"

u_result
u_domainOpenForService(
    u_domain *domain,
    const os_char *uri,
    const u_domainId_t id,
    os_int32 timeout);

os_uint32
u_domainThreadFlags(
    _In_ os_uint32 mask,
    _In_ os_uchar enable);

/** \brief Protect against process termination during domain access.
 *
 * This method is used by all other classes within this component whenever
 * the access to the domain is required.
 * Once access to the domain is no longer required the method
 * u_domainUnprotect must be called.
 */
u_result
u_domainProtect(
    const u_domain _this);

/** \brief Unprotect against process termination after domain access.
 *
 * This method is used by all other classes within this component to
 * release the protection against termination set by the method
 * u_domainProtect.
 */
void
u_domainUnprotect(void);

os_uint32
u_domainProtectCount(
    _In_ u_domain _this);

u_bool
u_domainIsService(
    _In_ u_domain _this);

u_result
u_domainAddWaitset(
    const u_domain _this,
    const u_waitset w);

u_result
u_domainRemoveWaitset(
    const u_domain _this,
    const u_waitset w);

u_bool
u_domainContainsWaitset(
    const u_domain _this,
    const u_waitset w);

os_uint32
u_domainWaitsetCount(
    const u_domain _this);

u_result
u_domainAddParticipant (
    const u_domain _this,
    const u_participant p);

u_result
u_domainRemoveParticipant (
    const u_domain _this,
    const u_participant p);

u_domain
u_domainKeep (
    const u_domain _this);

_Check_return_
u_bool
u_domainSetDetaching(
    _Inout_ u_domain _this,
    _In_ os_uint32 flags);

void
u_domainWaitDetaching(
    _Inout_ u_domain _this);

u_bool
u_domainProtectAllowed(
    _In_ u_domain _this);

u_bool
u_domainProtectAllowedClaimed(
    _In_ u_domain _this);

u_result
u_domainDetach (
    _Inout_ u_domain _this);

u_bool
u_domainCheckHandleServer (
    const u_domain _this,
    const os_uint32 serverId);

os_address
u_domainHandleServer(
    _In_ u_domain _this);

os_address
u_domainAddress(
    const u_domain _this);

u_result
u_domainFederationSpecificPartitionName (
    u_domain _this,
    c_char *buf,
    os_size_t bufsize);

void
u_domainIdSetThreadSpecific(
    _In_ u_domain domain);

#endif
