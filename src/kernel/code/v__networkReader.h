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

#ifndef V__NETWORKREADER_H
#define V__NETWORKREADER_H

#include "v_networkReader.h"

#define NW_BLOCKING_READER

/* ----------------------------- v_networkReader ----------------------- */

/* Protected methods to be used by v_reader only */

c_bool
v_networkReaderSubscribeGroup(
    v_networkReader _this,
    v_group group);

c_bool
v_networkReaderUnSubscribeGroup(
    v_networkReader _this,
    v_group group);


/* Protected methods to be used by v_networkReaderEntry only */

c_bool
v_networkReaderWrite(
    v_networkReader _this,
    v_message message,
    v_networkReaderEntry entry,
    c_ulong sequenceNumber,
    v_gid sender,
    c_bool sendTo,
    v_gid receiver);

#endif
