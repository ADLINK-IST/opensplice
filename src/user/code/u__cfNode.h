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

#ifndef U__CFNODE_H
#define U__CFNODE_H

#include "u_handle.h"
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
    u_participant participant,
    v_cfNode kNode);

void
u_cfNodeDeinit (
    u_cfNode _this);

u_participant
u_cfNodeParticipant (
    u_cfNode _this);

u_result
u_cfNodeRelease(
    u_cfNode node);

u_result
u_cfNodeReadClaim(
    u_cfNode node,
    v_cfNode* kernelNode);

#if defined (__cplusplus)
}
#endif

#endif
