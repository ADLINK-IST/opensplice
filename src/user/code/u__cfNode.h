
#ifndef U__CFNODE_H
#define U__CFNODE_H

#include "u_cfNode.h"
#include "u_participant.h"

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfNode) {
    u_participant participant;
    v_handle      configuration;
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
    
v_cfNode
u_cfNodeClaim (
    u_cfNode _this);

void
u_cfNodeRelease (
    u_cfNode _this);

u_participant
u_cfNodeParticipant (
    u_cfNode _this);

#if defined (__cplusplus)
}
#endif

#endif
