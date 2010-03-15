/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
    
v_writeResult
v_groupResend(
    v_group _this,
    v_message o,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId);

#if defined (__cplusplus)
}
#endif

#endif
