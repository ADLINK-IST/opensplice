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
#ifndef NW_STREAM_H_
#define NW_STREAM_H_

#include "c_metabase.h"
#include "v_networkReaderEntry.h"
#include "nw_plugTypes.h"
#include "nw_plugChannel.h"
#include "nw_plugSendChannel.h"
#include "nw_plugReceiveChannel.h"

NW_CLASS(nw_stream);

nw_stream
nw_stream_writeOpen (
    nw_plugChannel channel);

nw_stream
nw_stream_readOpen (
    nw_plugChannel channel);

nw_plugChannel
nw_stream_channel (
    nw_stream _this);

nw_bool
nw_stream_writeBegin (
    nw_stream _this,
    v_networkPartitionId partitionId,
    nw_signedLength *bytesLeft,
    plugSendStatistics pss);

void
nw_stream_writeEnd  (
    nw_stream _this,
    plugSendStatistics pss);

c_bool
nw_stream_readBegin (
    nw_stream _this,
    nw_senderInfo senderAddress,
    plugReceiveStatistics prs);

void
nw_stream_readEnd  (
    nw_stream _this,
    plugReceiveStatistics prs);

void
nw_stream_close (
    nw_stream _this);

c_ulong
nw_stream_write (
    nw_stream _this,
    c_object object);

c_ulong
nw_stream_writeOpaq (
    nw_stream _this,
    c_ulong length,
    c_voidp data);

c_ulong
nw_stream_writeString (
    nw_stream _this,
    c_type dummy,
    c_voidp string);

c_object
nw_stream_read(
    nw_stream _this,
    c_type type);

c_voidp
nw_stream_readOpaq(
    nw_stream stream,
    c_ulong size,
    c_voidp data);

c_voidp
nw_stream_readString(
    nw_stream stream,
    c_type type,
    c_voidp data);

#undef OS_API

#endif /* NW_STREAM_H_ */
