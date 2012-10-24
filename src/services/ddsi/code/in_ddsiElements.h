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
#ifndef IN_DDSIELEMENTS_H_
#define IN_DDSIELEMENTS_H_

#include "os_iterator.h"
#include "in_commonTypes.h"
#include "in_ddsiSerializer.h"
#include "in_ddsiDeserializer.h"
#include "c_time.h"
#include "v_kernel.h"
#include "in_ddsiDefinitions.h"
#include <assert.h>
#include "in_address.h"
#include "c_typebase.h" /* for c_ulong as sequenceNumber */

typedef in_octet in_ddsiSubmessageKind;
typedef in_octet in_ddsiSubmessageFlags;

/* \brief verify flag E is set */
#define in_ddsiSubmessageFlagsWithFlagE(_kind,_flags) ((_flags) & IN_FLAG_E)

/* -------------------------------------------------- */
/* ---- in_ddsiGuidprefix --------------------------- */
/* -------------------------------------------------- */
#define in_ddsiGuidPrefixLength 12
typedef in_octet in_ddsiGuidPrefix[in_ddsiGuidPrefixLength] /*in_list<in_octet,12>*/;
typedef in_octet *in_ddsiGuidPrefixRef;


void
in_ddsiGuidPrefixInit(
		in_ddsiGuidPrefixRef self,
		const v_gid *gid);

void
in_ddsiGuidPrefixDeinit(
		in_ddsiGuidPrefixRef self);

/** \return -1 on error, otherwise number of octets written */
in_long
in_ddsiGuidPrefixSerialize(
        in_ddsiGuidPrefixRef self,
        in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets read */
in_long
in_ddsiGuidPrefixInitFromBuffer(
		in_ddsiGuidPrefixRef self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiGuidPrefixEqual(
		const in_ddsiGuidPrefixRef self,
		const in_ddsiGuidPrefixRef other);

/* -------------------------------------------------- */
/* ---- in_ddsiBuiltinEndpoint ---------------------- */
/* -------------------------------------------------- */


OS_CLASS(in_ddsiBuiltinEndpointSet);
OS_STRUCT(in_ddsiBuiltinEndpointSet) {
    os_uint32 flags /* set of flags */;
};


/** \return -1 on error, otherwise number of octets written */
in_long
in_ddsiBuiltinEndpointSetSerialize(
		in_ddsiBuiltinEndpointSet builtinEndpoint,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets read */
in_long
in_ddsiBuiltinEndpointSetInitFromBuffer(
		in_ddsiBuiltinEndpointSet builtinEndpoint,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiBuiltinEndpointSetEqual(
		in_ddsiBuiltinEndpointSet self,
		in_ddsiBuiltinEndpointSet other);


/* -------------------------------------------------- */
/* ---- in_ddsiProductVersion ----------------------- */
/* -------------------------------------------------- */


OS_CLASS(in_ddsiProductVersion);
OS_STRUCT(in_ddsiProductVersion) {
    in_octet version[4] /*in_list<in_octet,4>*/;
};


/** \return -1 on error, otherwise number of octets read */
in_long
in_ddsiProductVersionSerialize(
		in_ddsiProductVersion productVersion,
		in_ddsiSerializer serializer);

in_long
in_ddsiProductVersionInitFromBuffer(
		in_ddsiProductVersion productVersion,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiProductVersionEqual(
		in_ddsiProductVersion self,
		in_ddsiProductVersion other);


/* -------------------------------------------------- */
/* ---- in_ddsiEntityId ----------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiEntityId);
OS_STRUCT(in_ddsiEntityId) {
    in_octet entityKey[3] /*in_list<in_octet,3>*/;
    in_octet entityKind;
};


/** \return -1 on error, otherwise number of octets read */
in_long
in_ddsiEntityIdInitFromBuffer(
		in_ddsiEntityId self,
		in_ddsiDeserializer deserializer);


/** \return -1 on error, otherwise number of octets written */
in_long
in_ddsiEntityIdSerialize(
		in_ddsiEntityId self,
		in_ddsiSerializer serializer);

os_boolean
in_ddsiEntityIdEqual(
		in_ddsiEntityId self,
		in_ddsiEntityId other);

os_uint32
in_ddsiEntityIdAsUint32(in_ddsiEntityId self);

/* -------------------------------------------------- */
/* ---- in_ddsiGuid --------------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiGuid);
OS_STRUCT(in_ddsiGuid) {
    in_ddsiGuidPrefix guidPrefix;
    OS_STRUCT(in_ddsiEntityId) entityId;
};


/** \return -1 on error, otherwise number of octets written */
in_long
in_ddsiGuidSerialize(
		in_ddsiGuid self,
		in_ddsiSerializer serializer);

os_boolean
in_ddsiGuidEqual(
		in_ddsiGuid self,
		in_ddsiGuid other);

/** */
os_boolean
in_ddsiGuidInit(
        in_ddsiGuid _this,
        in_ddsiGuidPrefixRef guidPrefix,
        in_ddsiEntityId entityId);

/** \return -1 on error, otherwise number of octets read */
in_long
in_ddsiGuidInitFromBuffer(
		in_ddsiGuid self,
		in_ddsiDeserializer deserializer);

/* -------------------------------------------------- */
/* ---- in_ddsiTime --------------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiTime);
OS_STRUCT(in_ddsiTime) {
    os_int32 seconds;
    os_uint32 fraction;
};
extern const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_ZERO;
extern const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_INVALID;
extern const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_INFINITE;
extern const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_MIN_INFINITE;

void
in_ddsiTimeInit(
		in_ddsiTime self,
		const c_time *time,
		os_boolean duration);

in_long
in_ddsiTimeInitFromBuffer(
		in_ddsiTime self,
		in_ddsiDeserializer deserializer);

void
in_ddsiTimeDeinit(
		in_ddsiTime self);

c_equality
in_ddsiTimeCompare(
		const in_ddsiTime t1,
		const in_ddsiTime t2);

void
in_ddsiTimeAsCTime(
		in_ddsiTime self,
		c_time *ctime,
		os_boolean duration);

/** \brief compare c_time parameters with tolerance */
os_boolean
in_ctimeEqualWithTolerance(
		c_time *t1,
		c_time *t2);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiTimeSerialize(
		in_ddsiTime self,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiTimeInstantCTimeSerialize(
		const c_time *minTime,
		in_ddsiSerializer serializer,
		os_boolean isDuration);


/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiTimeInstantCTimeDeserialize(
		c_time *parsedTime,
		in_ddsiDeserializer deserializer,
		os_boolean isDuration);

/* -------------------------------------------------- */
/* ---- in_ddsiVendor* ------------------------------ */
/* -------------------------------------------------- */

typedef in_octet in_ddsiVendorId[2];
OS_CLASS(in_ddsiVendor);
OS_STRUCT(in_ddsiVendor) {
  in_ddsiVendorId vendorId; /*in_list<in_octet,2>*/
};

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiVendorSerialize(
		in_ddsiVendor vendor,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiVendorInitFromBuffer(
		in_ddsiVendor vendor,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiVendorEqual(
		in_ddsiVendor self,
		in_ddsiVendor other);

os_ushort
in_ddsiVendorToInteger(in_ddsiVendor self);


/* -------------------------------------------------- */
/* ---- in_ddsiSequenceNumber ----------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiSequenceNumber);
OS_STRUCT(in_ddsiSequenceNumber) {
    os_int32  high;
    os_uint32 low;
};

/** */
void
in_ddsiSequenceNumberInitNative(
		in_ddsiSequenceNumber self,
		c_ulong sequenceNumber);

/** */
void
in_ddsiSequenceNumberInit(
        in_ddsiSequenceNumber self,
        os_int32  high,
        os_uint32 low);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiSequenceNumberSerialize(
		in_ddsiSequenceNumber self,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiSequenceNumberInitFromBuffer(
		in_ddsiSequenceNumber self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiSequenceNumberEqual(
		in_ddsiSequenceNumber self,
		in_ddsiSequenceNumber other);

/** */
os_boolean
in_ddsiSequenceNumberIsValid(
        in_ddsiSequenceNumber _this);

/**  \brief define an ordering on sequence numbers */
c_equality
in_ddsiSequenceNumberCompare(
        in_ddsiSequenceNumber _this,
        in_ddsiSequenceNumber other);
/* -------------------------------------------------- */
/* ---- in_ddsiSequenceNumberSet -------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiSequenceNumberSet);
OS_STRUCT(in_ddsiSequenceNumberSet) {
    OS_STRUCT(in_ddsiSequenceNumber) bitmapBase;
    os_uint32 numBits;
    os_uint32 bitmap[8]; /* sequence<long, 8> */
    /* Note: AFAICS, the bitmap should be declared as
     * sequence<unsigned long, 8>
     * Otherwise each element provides only 31 bits + sign-bit and
     * signed-shift would have a different semenatic. */
};

/** */
void
in_ddsiSequenceNumberSetDeinit(
        in_ddsiSequenceNumberSet _this);

/** */
os_boolean
in_ddsiSequenceNumberSetInit(
        in_ddsiSequenceNumberSet _this,
        in_ddsiSequenceNumber base);


/** call this operation to add a sequence number to the set,
 *  with sequenceNumber<base+256 */
os_boolean
in_ddsiSequenceNumberSetAdd(
        in_ddsiSequenceNumberSet _this,
        in_ddsiSequenceNumber sequenceNumber);

/**  return the set size, may be 0 in case base==0 */
#define in_ddsiSequenceNumberSetSize(_s) ((_s)->numBits)

/** calculate the bitmap-slot the bit resides in */
#define in_ddsiSequenceNumberSetSlotOfNthBit(_nth) \
    (((_nth)+31)/32)

/** calculate the index of bit within the slot */
#define in_ddsiSequenceNumberSetMaskOfNthBit(_nth) \
    ( ((os_uint32)1UL) << (_nth-(32*in_ddsiSequenceNumberSetSlotOfNthBit(_nth))))

/** verify the set contains the specific offset value (_nth<=numBits) */
#define in_ddsiSequenceNumberSetContainsNth(_s,_nth) \
    (_nth<=(_s)->numBits && \
     0!=(((_s)->bitmap[in_ddsiSequenceNumberSetSlotOfNthBit(_nth)]) & (in_ddsiSequenceNumberSetMaskOfNthBit(_nth))))

/** */
os_size_t
in_ddsiSequenceNumberSetSerializedSize(
        in_ddsiSequenceNumberSet _this);

/** */
in_long
in_ddsiSequenceNumberSetSerialize(
        in_ddsiSequenceNumberSet self,
        in_ddsiSerializer serializer);

in_long
in_ddsiSequenceNumberSetInitFromBuffer(
        in_ddsiSequenceNumberSet _this,
        in_ddsiDeserializer deserializer);

/* -------------------------------------------------- */
/* ---- in_ddsiFragmentNumber ----------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiFragmentNumber);
OS_STRUCT(in_ddsiFragmentNumber) {
    os_uint32 value;
};

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiFragmentNumberSerialize(
		in_ddsiFragmentNumber self,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiFragmentNumberInitFromBuffer(
		in_ddsiFragmentNumber self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiFragmentNumberEqual(
		in_ddsiFragmentNumber self,
		in_ddsiFragmentNumber other);

/* -------------------------------------------------- */
/* ---- in_ddsiLocator ------------------------------ */
/* -------------------------------------------------- */

#define IN_ANY_IP_ADDRESS (NULL)
#define IN_ANY_UDP_PORT   (0)


OS_CLASS(in_ddsiLocator);
OS_STRUCT(in_ddsiLocator) {
    os_int32 kind;
    os_uint32 port; /* yes, should be short, but DDSi declared it this way */
    OS_STRUCT(in_address) address; /*in_list<in_octet,16>*/
};


void
in_ddsiLocatorInitInvalid(
		in_ddsiLocator self);

void
in_ddsiLocatorInit(
		in_ddsiLocator self,
		os_int32 kind,
		os_uint32 port,
		const in_address address);


/** \brief UDPv4 init function for convenience */
void
in_ddsiLocatorInitUDPv4(
		in_ddsiLocator _this,
		os_uint32 port,
		const in_addressIPv4 address);

/** \brief UDPv6 init function for convenience */
void
in_ddsiLocatorInitUDPv6(
		in_ddsiLocator _this,
		os_uint32 port,
		const in_addressIPv6 address);

void
in_ddsiLocatorDeinit(
		in_ddsiLocator self);

in_long
in_ddsiLocatorSerialize(
		in_ddsiLocator locator,
		in_ddsiSerializer writer);

in_long
in_ddsiLocatorInitFromBuffer(
		in_ddsiLocator self, in_ddsiDeserializer reader);

void
in_ddsiLocatorInitFromSockaddr(
		in_ddsiLocator self,
		const struct sockaddr *address);

os_boolean
in_ddsiLocatorEqual(
		in_ddsiLocator self,
		in_ddsiLocator other);

void
in_ddsiLocatorToSockaddr(
		in_ddsiLocator self,
		struct sockaddr *dest);

void
in_ddsiLocatorToSockaddrForAnyAddress(
	    in_ddsiLocator _this,
	    struct sockaddr *dest);

os_uint32
in_ddsiLocatorGetPort(
		in_ddsiLocator self);

in_address
in_ddsiLocatorGetIp(
    in_ddsiLocator _this);

void
in_ddsiLocatorSetPort(
		in_ddsiLocator self,
		os_uint32 port);

os_int32
in_ddsiLocatorGetKind(
		in_ddsiLocator self);

c_equality
in_ddsiLocatorCompare(
		in_ddsiLocator self,
		in_ddsiLocator other);

void
in_ddsiLocatorCopy(
		in_ddsiLocator _this,
		in_ddsiLocator copyFrom);

/** \brief Verifying locator */
os_boolean
in_ddsiLocatorIsValid(in_ddsiLocator _this);

/* -------------------------------------------------- */
/* ---- in_ddsiTopicKind ---------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiTopicKind);
OS_STRUCT(in_ddsiTopicKind) {
    os_int32 value;
};

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiTopicKindSerialize(
		in_ddsiTopicKind self,
		in_ddsiSerializer serializer);

/** \return -1 on error, otherwise number of octets */
in_long
in_ddsiTopicKindInitFromBuffer(
		in_ddsiTopicKind self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiTopicKindEqual(
		in_ddsiTopicKind self,
		in_ddsiTopicKind oither);

/* -------------------------------------------------- */
/* ---- in_ddsiStatusInfo --------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiStatusInfo);
OS_STRUCT(in_ddsiStatusInfo) {
    os_int32 value;
};

in_long
in_ddsiStatusInfoSerialize(
		in_ddsiStatusInfo self,
		in_ddsiSerializer serializer);

in_long
in_ddsiStatusInfoInitFromBuffer(
		in_ddsiStatusInfo self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiStatusInfoEqual(
		in_ddsiStatusInfo self,
		in_ddsiStatusInfo other);

/* -------------------------------------------------- */
/* ---- in_ddsiReliabilityKind ---------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiReliabilityKind);
OS_STRUCT(in_ddsiReliabilityKind) {
    os_int32 value;
};

in_long
in_ddsiReliabilityKindSerialize(
		in_ddsiReliabilityKind self,
		in_ddsiSerializer serializer);

in_long
in_ddsiReliabilityKindInitFromBuffer(
		in_ddsiReliabilityKind self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiReliabilityKindEqual(
		in_ddsiReliabilityKind self,
		in_ddsiReliabilityKind other);

/* -------------------------------------------------- */
/* ---- in_ddsiCount  ------------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiCount);
OS_STRUCT(in_ddsiCount) {
    os_int32 value;
};

in_long
in_ddsiCountSerialize(
		in_ddsiCount self,
		in_ddsiSerializer serializer);

in_long
in_ddsiCountInitFromBuffer(
		in_ddsiCount self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiCountEqual(
		in_ddsiCount self,
		in_ddsiCount other);

/* -------------------------------------------------- */
/* ---- in_ddsiProtocolVersion ---------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiProtocolVersion);
OS_STRUCT(in_ddsiProtocolVersion) {
    in_octet major;
    in_octet minor;
};

in_long
in_ddsiProtocolVersionSerialize(
		in_ddsiProtocolVersion self,
		in_ddsiSerializer serializer);

in_long
in_ddsiProtocolVersionInitFromBuffer(
		in_ddsiProtocolVersion self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiProtocolVersionEqual(
		in_ddsiProtocolVersion self,
		in_ddsiProtocolVersion other);

/* -------------------------------------------------- */
/* ---- in_ddsiKeyHash* ----------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiKeyHashPrefix);
OS_STRUCT(in_ddsiKeyHashPrefix) {
    in_octet value[12] /*in_list<in_octet,12>*/;
};

in_long
in_ddsiKeyHashPrefixSerialize(
		in_ddsiKeyHashPrefix self,
		in_ddsiSerializer serializer);

in_long
in_ddsiKeyHashPrefixInitFromBuffer(
		in_ddsiKeyHashPrefix self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiKeyHashPrefixEqual(
		in_ddsiKeyHashPrefix self,
		in_ddsiKeyHashPrefix other);

OS_CLASS(in_ddsiKeyHashSuffix);
OS_STRUCT(in_ddsiKeyHashSuffix) {
    in_octet value[4] /*in_list<in_octet,4>*/;
};

in_long
in_ddsiKeyHashSuffixSerialize(
		in_ddsiKeyHashSuffix self,
		in_ddsiSerializer serializer);

in_long
in_ddsiKeyHashSuffixInitFromBuffer(
		in_ddsiKeyHashSuffix self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiKeyHashSuffixEqual(
		in_ddsiKeyHashSuffix self,
		in_ddsiKeyHashSuffix other);

/* -------------------------------------------------- */
/* ---- in_ddsiParameterId -------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiParameterId);
OS_STRUCT(in_ddsiParameterId) {
    os_ushort value; /* 16bit integer */
};

in_long
in_ddsiParameterIdSerialize(
		in_ddsiParameterId self,
		in_ddsiSerializer serializer);

in_long
in_ddsiParameterIdInitFromBuffer(
		in_ddsiParameterId self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiParameterIdEqual(
		in_ddsiParameterId self,
		in_ddsiParameterId other);

/* -------------------------------------------------- */
/* ---- in_ddsiContentFilter* ----------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiContentFilterProperty);
OS_STRUCT(in_ddsiContentFilterProperty) {
    os_char* contentFilteredTopicName;
    os_char* relatedTopicName;
    os_char* filterClassName;
    os_char* filterExpression;
    os_iter expressionParameters /* C_SEQUENCE<os_char*> */; /* TODO */
};

in_long
in_ddsiContentFilterPropertySerialize(
		in_ddsiContentFilterProperty self,
		in_ddsiSerializer serializer);

in_long
in_ddsiContentFilterPropertyInitFromBuffer(
		in_ddsiContentFilterProperty self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiContentFilterPropertyEqual(
		in_ddsiContentFilterProperty self,
		in_ddsiContentFilterProperty other);

typedef os_iter in_ddsiFilterResult /*C_SEQUENCE<os_int32>*/;

typedef os_int32 in_ddsiFilterSignature[4] /*in_list<os_int32,4>*/;

OS_CLASS(in_ddsiContentFilterInfo);
OS_STRUCT(in_ddsiContentFilterInfo) {
    in_ddsiFilterResult filterResult;
    os_iter filterSignatures /*C_SEQUENCE<networkingModule::in_filterSignature>*/;
};

in_long
in_ddsiContentFilterInfoSerialize(
		in_ddsiContentFilterInfo self,
		in_ddsiSerializer serializer);

in_long
in_ddsiContentFilterInfoInitFromBuffer(
		in_ddsiContentFilterInfo self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiContentFilterInfoEqual(
		in_ddsiContentFilterInfo self,
		in_ddsiContentFilterInfo other);

/* -------------------------------------------------- */
/* ---- in_ddsiProperty ----------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiProperty);
OS_STRUCT(in_ddsiProperty) {
    os_char* name;
    os_char* value;
};

in_long
in_ddsiPropertySerialize(
		in_ddsiProperty self,
		in_ddsiSerializer serializer);

in_long
in_ddsiPropertyInitFromBuffer(
		in_ddsiProperty self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiPropertyEqual(
		in_ddsiProperty self,
		in_ddsiProperty other);

/* -------------------------------------------------- */
/* ---- in_ddsiEntityName --------------------------- */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiEntityName);
OS_STRUCT(in_ddsiEntityName)
{
	os_char* value;
};

void
in_ddsiEntityNameInit(
		in_ddsiEntityName _this,
		char* name);

in_long
in_ddsiEntityNameSerialize(
		in_ddsiEntityName self,
		in_ddsiSerializer serializer);

in_long
in_ddsiEntityNameInitFromBuffer(
		in_ddsiEntityName self,
		in_ddsiDeserializer deserializer);

os_boolean
in_ddsiEntityNameEqual(
		in_ddsiEntityName self,
		in_ddsiEntityName other);

/* -------------------------------------------------- */
/* ---- in_ddsiMessageHeader ------------------------ */
/* -------------------------------------------------- */

OS_CLASS(in_ddsiMessageHeader);
OS_STRUCT(in_ddsiMessageHeader) {
    in_octet protocolId[4] /*in_list<in_octet,4>*/;
    OS_STRUCT(in_ddsiProtocolVersion) version;
    OS_STRUCT(in_ddsiVendor) vendor;
    in_ddsiGuidPrefix guidPrefix;
    /* total/const octet size is 20 */
};


/** */
in_long
in_ddsiMessageHeaderSerializeInstantly(
		const in_ddsiProtocolVersion protocolVersion,
		const in_ddsiVendor vendor,
		const in_ddsiGuidPrefixRef guidPrefix,
		in_ddsiSerializer serializer);

in_long
in_ddsiMessageHeaderInitFromBuffer(
		in_ddsiMessageHeader self,
		in_ddsiDeserializer deserializer);


os_boolean
in_ddsiMessageHeaderIsValid(in_ddsiMessageHeader self);

/* -------------------------------------------------- */
/* ---- in_ddsiSubmessageHeader ------------------------ */
/* -------------------------------------------------- */


OS_CLASS(in_ddsiSubmessageHeader);
OS_STRUCT(in_ddsiSubmessageHeader)
{
	in_ddsiSubmessageKind  kind;
	in_ddsiSubmessageFlags flags;
	os_ushort              octetsToNextHeader; /* uint16 */
};

in_long
in_ddsiSubmessageHeaderSerializedSize(
		in_ddsiSerializer serializer);

/** \brief serialize submessage header
 * \param octetsToNextHeader positive integer */
in_long
in_ddsiSubmessageHeaderSerializeInstantly(
		in_ddsiSubmessageKind kind,
		in_ddsiSubmessageFlags flags,
		os_ushort octetsToNextHeader,
		in_ddsiSerializer cdr);

/** */
in_long
in_ddsiSubmessageHeaderInitFromBuffer(
		in_ddsiSubmessageHeader self,
		in_ddsiDeserializer deserializer);

/** */
os_boolean
in_ddsiSubmessageHeaderIsBigEndian(
		in_ddsiSubmessageHeader self);

/** */
os_boolean
in_ddsiSubmessageHeaderHasFlagE(
		in_ddsiSubmessageHeader _this);


/** */
os_boolean
in_ddsiSubmessageHeaderHasFlagI(
		in_ddsiSubmessageHeader _this);

/** */
os_boolean
in_ddsiSubmessageHeaderHasFlagD(
		in_ddsiSubmessageHeader _this);

/** */
os_boolean
in_ddsiSubmessageHeaderHasFlagQ(
		in_ddsiSubmessageHeader _this);


/** */
os_boolean
in_ddsiSubmessageHeaderHasFlagH(in_ddsiSubmessageHeader _this);

/** \brief deinit */
#define in_ddsiSubmessageHeaderDeinit(_self)

#endif /*IN_DDSIELEMENTS_H_*/
