/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V__GROUP_H
#define V__GROUP_H

#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define v_groupKeyList(_this) \
        c_tableKeyList(v_group(_this)->instances)

v_group
v_groupNew (
    v_partition partition,
    v_topic topic,
    c_long id);

void
v_groupDeinit (
    v_group _this);            

void
v_groupAddWriter (
    v_group _this,
    v_writer w);

void
v_groupRemoveWriter (
    v_group _this,
    v_writer w);

c_bool
v_groupHasCache (
    v_group _this);

c_iter
v_groupGetRegisterMessages(
    v_group _this,
    c_ulong systemId);
    
c_iter
v_groupGetRegisterMessagesOfWriter(
    v_group _this,
    v_gid writerGid);

/* The following hash defines implement a basic form of
 * destination identification for resend messages.
 * The resendScope is a set of these bits specifying the
 * resend scope.
 */

#define V_RESEND_TOPIC       (1)
#define V_RESEND_VARIANT     (2)
#define V_RESEND_REMOTE      (4)
#define V_RESEND_DURABLE     (8)
#define V_RESEND_ALL        (15)
    
v_writeResult
v_groupResend(
    v_group _this,
    v_message o,
    v_groupInstance *instancePtr,
    v_resendScope *resendScope,
    v_networkId writingNetworkId);

v_result
v_groupDisposeAll (
    v_group group,
    c_time timestamp);

#if defined (__cplusplus)
}
#endif

#endif
