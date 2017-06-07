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

#ifndef U__CFNODE_H
#define U__CFNODE_H

#include "u__handle.h"
#include "u_cfNode.h"
#include "u_participant.h"

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfNode) {
    u_participant participant;
    u_handle      configuration;
    c_ulong       id;
    v_cfKind      kind;
};

#define u_cfNode(o) ((u_cfNode)(o))

void
u_cfNodeInit (
    u_cfNode _this,
    const u_participant participant,
    const v_cfNode kNode);

void
u_cfNodeDeinit (
    u_cfNode _this);

u_participant
u_cfNodeParticipant (
    const u_cfNode _this);

void
u_cfNodeRelease(
    const u_cfNode node);

u_result
u_cfNodeReadClaim(
    const u_cfNode node,
    v_cfNode* kernelNode);

#if defined (__cplusplus)
}
#endif

#endif
