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
#ifndef IN_DDSISUBMESSAGE_H_
#define IN_DDSISUBMESSAGE_H_

#include "in__object.h"
#include "in_ddsiSerializer.h"
#include "in_ddsiElements.h"
#include "in__ddsiParameterList.h"
#include "in_ddsiParticipant.h"
#include "in_ddsiSerializedData.h"
#include "in_locatorList.h"

#if defined (__cplusplus)
extern "C" {
#endif


/** \brief Masking serialized data in receive-buffer
 *
 * The following types are masks for the serialized data in
 * the message buffer. The lifecycle is strongly bound to the lifecycle of
 * message buffer.
 *
 * */

/** \brief DDSi Data submessage */
OS_STRUCT(in_ddsiSubmessage)
{
	OS_STRUCT(in_ddsiSubmessageHeader) header;
};

#define in_ddsiSubmessage(_o) \
    ((in_ddsiSubmessage)_o)

/** \return OS_TRUE if flag set, otherwise OS_FALSE
 *
 * The flag is set in case of littleEndian encoding */
os_boolean
in_ddsiSubmessageHasFlagE(in_ddsiSubmessage _this);

/**
 *  */
os_boolean
in_ddsiSubmessageHasFlagD(in_ddsiSubmessage _this);

/**
 *  */
os_boolean
in_ddsiSubmessageHasFlagI(in_ddsiSubmessage _this);

/**
 *  */
os_boolean
in_ddsiSubmessageHasFlagH(in_ddsiSubmessage _this);

/**
 *  */
os_boolean
in_ddsiSubmessageHasFlagQ(in_ddsiSubmessage _this);


/** */
os_boolean
in_ddsiSubmessageIsBigEndian(in_ddsiSubmessage _this);



/** \brief DDSi InfoTimestamp submessage
 * Extends  in_ddsiSubmessage*/
OS_STRUCT(in_ddsiSubmessageInfoTimestamp)
{
	OS_EXTENDS(in_ddsiSubmessage);
	OS_STRUCT(in_ddsiTime) timestamp;
};

/** */
in_long
in_ddsiSubmessageInfoTimestampSerializeInstantly(
		const c_time *timestamp,
		in_ddsiSerializer serializer);


/** */
in_long
in_ddsiSerializedDataInitFromBuffer(
		in_ddsiSerializedData _this,
		in_ddsiDeserializer deserializer,
		os_size_t length);

/** */
void
in_ddsiSerializedDataInitEmpty(
		in_ddsiSerializedData _this);

/** \brief DDSi Data submessage
 * Extends  in_ddsiSubmessage*/
OS_STRUCT(in_ddsiSubmessageData)
{
	OS_EXTENDS(in_ddsiSubmessage);
	os_ushort                        extraFlags;
	OS_STRUCT(in_ddsiEntityId)       readerId;
	OS_STRUCT(in_ddsiEntityId)       writerId;
	OS_STRUCT(in_ddsiSequenceNumber) writerSN;
	OS_STRUCT(in_ddsiParameterList)  inlineQos;
	OS_STRUCT(in_ddsiSerializedData) serializedPayload;
};

in_long
in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
        os_ushort kind,
        in_ddsiSerializer serializer);

/** */
in_long
in_ddsiSubmessageDataInitFromBuffer(
		in_ddsiSubmessageData _this,
		in_ddsiSubmessageHeader preparsedHeader,
		in_ddsiDeserializer deserializer);

/** total size, from first octet to last one
 *
 * not identical to octetsToNextSubmessageHeader */
os_size_t
in_ddsiSubmessageDataSerializedSize(
        os_size_t  inlineQosSize,
        os_size_t  serializedPayloadSize);

/** */
/** */
in_long
in_ddsiSubmessageDataHeaderSerializeInstantly(
        os_size_t  inlineQosSize,
        os_size_t  serializedPayloadSize,
        in_ddsiSequenceNumber  sequenceNumber, /* OSPL native representation */
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSerializer serializer);

/** */
in_long
in_ddsiSubmessageInfoDestinationSerializeInstantly(
        in_ddsiGuidPrefix destPrefix,
        in_ddsiSerializer serializer);




/** **************************************
 * HEARTBEAT
 ** ***************************************/
OS_CLASS(in_ddsiSubmessageHeartbeat);
OS_STRUCT(in_ddsiSubmessageHeartbeat)
{
  OS_EXTENDS(in_ddsiSubmessage); /* the header */
  OS_STRUCT(in_ddsiEntityId) readerId;
  OS_STRUCT(in_ddsiEntityId) writerId;
  OS_STRUCT(in_ddsiSequenceNumber) firstSN;
  OS_STRUCT(in_ddsiSequenceNumber) lastSN;
  OS_STRUCT(in_ddsiCount) count;
};
#define in_ddsiSubmessageHeartbeat(_this) ((in_ddsiSubmessageHeartbeat)_this)

/** */
in_long
in_ddsiSubmessageHeartbeatSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumber firstSN,
        in_ddsiSequenceNumber lastSN,
        os_ushort count,
        in_ddsiSerializer serializer);

in_long
in_ddsiSubmessageHeartbeatInitFromBuffer(
        in_ddsiSubmessageHeartbeat _this,
        in_ddsiSubmessageHeader preparsedHeader,
        in_ddsiDeserializer deserializer);

/** **************************************
 * ACKNACK
 ** ***************************************/
OS_CLASS(in_ddsiSubmessageAckNack);
OS_STRUCT(in_ddsiSubmessageAckNack)
{
  OS_EXTENDS(in_ddsiSubmessage); /* the header */
  OS_STRUCT(in_ddsiEntityId) readerId;
  OS_STRUCT(in_ddsiEntityId) writerId;
  OS_STRUCT(in_ddsiSequenceNumberSet) readerSNState;
  OS_STRUCT(in_ddsiCount) count;
};

#define in_ddsiSubmessageAckNack(_this) ((in_ddsiSubmessageAckNack)_this)

/**
 */
os_size_t
in_ddsiSubmessageAckNackSerializedSize(
        in_ddsiSequenceNumberSet set);

/** */
in_long
in_ddsiSubmessageAckNackSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumberSet sequenceNumberSet,
        os_int32 count,
        in_ddsiSerializer serializer);

in_long
in_ddsiSubmessageAckNackInitFromBuffer(
        in_ddsiSubmessageAckNack _this,
        in_ddsiSubmessageHeader preparsedHeader,
        in_ddsiDeserializer deserializer);

/** **************************************
 * INFO_REPLY
 ** ***************************************/

/**
 */
os_size_t
in_ddsiSubmessageInfoReplySerializedSize(
        in_locatorList *unicastLocatorList,
        in_locatorList *multicastLocatorList);

/** */
in_long
in_ddsiSubmessageInfoReplySerializeInstantly(
        in_locatorList *unicastLocatorList,
        in_locatorList *multicastLocatorList,
        in_ddsiSerializer serializer);

/** change state of receiver object
 *
 * Append the unicast and multicast locators
 * to the given locator lists. if operation returns
 * with error (-1) the lists may be modified. */
in_long
in_ddsiSubmessageInfoReplyInitFromBuffer(
        in_locatorList *unicastLocatorList,
        in_locatorList *multicastLocatorList,
        in_ddsiSubmessageHeader preparsedHeader,
        in_ddsiDeserializer deserializer);


#if defined (__cplusplus)
}
#endif

#endif /* IN_DDSISUBMESSAGE_H_ */
