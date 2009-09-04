/*
 * in_ddsiSubmessage.c
 *
 *  Created on: Feb 24, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiSubmessage.h"

/* **** implementation headers **** */
#include "v_message.h"
#include "kernelModule.h"
#include "in_connectivityWriterFacade.h"
#include "in__ddsiParameterList.h"
#include "in_ddsiParameterList.h"
#include "in_report.h"
#include "in_align.h"
#include "in_ddsiEncapsulationHeader.h"

/* **** private functions **** */


static in_long
dataHeaderSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumber sequenceNumber,
        in_ddsiSerializer serializer)
{
	/*
     * 0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * +---------------+---------------+---------------+---------------+
     * |   Flags         extraFlags    |      octetsToInlineQos        |
     * +---------------+---------------+---------------+---------------+
     * | EntityId readerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * | EntityId writerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * + SequenceNumber writerSeqNum                                   +
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */


	os_ushort extraFlags = 0U;
	os_ushort octetsToInlineQos =16U; /*may become variable in future */
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;

	/** break out in case of serialization error */
	do {
		nofOctets = in_ddsiSerializerAppendUshort(
				serializer,
				extraFlags);
		if (nofOctets<0) break; /* leave while loop */
		total += nofOctets;

		nofOctets = in_ddsiSerializerAppendUshort(
				serializer,
				octetsToInlineQos);
		if (nofOctets<0) break; /* leave while loop */
		total += nofOctets;

		nofOctets = in_ddsiEntityIdSerialize(readerId, serializer);
		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets = in_ddsiEntityIdSerialize(writerId, serializer);
		if (nofOctets<0) break; /* leave while loop */
		total += nofOctets;

		nofOctets = in_ddsiSequenceNumberSerialize(sequenceNumber, serializer);
		if (nofOctets<0) break; /* leave while loop */
		total += nofOctets;

		assert(total == 20);

		/* final assignment */
		result = total;
	} while (0);
	/* result == -1 || result > 0 */
	assert(result != 0);

	return result;
}

/* **** public functions **** */


/** */
in_long
in_ddsiSerializedDataInitFromBuffer(
		in_ddsiSerializedData _this,
		in_ddsiDeserializer deserializer,
		os_size_t length)
{
	in_long nofOctets = 0;
	in_long total = 0;
	in_long result = -1;
	OS_STRUCT(in_ddsiEncapsulationHeader) encapsHeader;

	in_octet *begin =
		in_ddsiDeserializerGetIndex(deserializer);

	/* init */
	memset (_this, 0, sizeof(*_this));

	 if ((((os_size_t)begin) & (4-1)) > 0) {
		IN_REPORT_WARNING("in_ddsiSerializedDataInitFromBuffer",
				"serialized data not aligned by 4");
	 }

	do {
        /* parse the encapsulation header and store the codecId, also check if
         * codec is valid */
        nofOctets = in_ddsiEncapsulationHeaderInitFromBuffer(
                &encapsHeader, deserializer);
        if (nofOctets < 0) break;
        total += nofOctets;

        assert(total == 4);

        /* skip the successing octets */
	    nofOctets =
			in_ddsiDeserializerSeek(deserializer, length-total);
		if (nofOctets < 0) break;
		total += nofOctets;

		/* succeeded */
		result = total;
		_this->begin = begin;
		_this->length = length;
		_this->codecId =
		    in_ddsiEncapsulationHeaderGetCodecId(&encapsHeader);
		_this->flags =  /* reserved for future use */
		    in_ddsiEncapsulationHeaderGetFlags(&encapsHeader);
		/* done */

		result = total;
	} while (0);

	return result;
}


/** */
void
in_ddsiSerializedDataInitEmpty(
		in_ddsiSerializedData _this)
{
	_this->begin = NULL;
	_this->length = 0;
}


os_boolean
in_ddsiSubmessageHasFlagE(in_ddsiSubmessage _this)
{
	return in_ddsiSubmessageHeaderHasFlagE(&(_this->header));
}

os_boolean
in_ddsiSubmessageHasFlagI(in_ddsiSubmessage _this)
{
	return in_ddsiSubmessageHeaderHasFlagI(&(_this->header));
}

os_boolean
in_ddsiSubmessageHasFlagD(in_ddsiSubmessage _this)
{
	return in_ddsiSubmessageHeaderHasFlagD(&(_this->header));
}

os_boolean
in_ddsiSubmessageHasFlagH(in_ddsiSubmessage _this)
{
	return in_ddsiSubmessageHeaderHasFlagH(&(_this->header));
}

os_boolean
in_ddsiSubmessageHasFlagQ(in_ddsiSubmessage _this)
{
	return in_ddsiSubmessageHeaderHasFlagQ(&(_this->header));
}


os_boolean
in_ddsiSubmessageIsBigEndian(in_ddsiSubmessage _this)
{
	os_boolean littleEndianEncoding =
		in_ddsiSubmessageHasFlagE(_this);

	return !littleEndianEncoding;
}

static in_ddsiSubmessageFlags
in_ddsiSubmessageCreateFlagE(in_ddsiSerializer serializer)
{
	os_boolean isBigE =
		in_ddsiSerializerIsBigEndian(serializer);
	in_ddsiSubmessageFlags result =
		isBigE ? 0 : IN_FLAG_E;
	return result;
}

static in_ddsiSubmessageFlags
in_ddsiSubmessageCreateFlagM(os_size_t nofMulticastLocators)
{
    in_ddsiSubmessageFlags result =
        (nofMulticastLocators == 0) ? 0 : IN_FLAG_INFO_REPLY_M;
    return result;
}

static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagQ(os_boolean expectsInlineQos)
{
	in_ddsiSubmessageFlags result =
		expectsInlineQos ? IN_FLAG_DATA_Q : 0;
	return result;
}

static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagAckNackF(os_boolean final)
{
	in_ddsiSubmessageFlags result;

    result = final ? IN_FLAG_ACKNACK_F : 0;
	return result;
}

static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagHeartbeatF(
    os_boolean final)
{
	in_ddsiSubmessageFlags result;

    result = final ? IN_FLAG_HEARTBEAT_F : 0;
	return result;
}


static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagD(os_boolean withFlagD)
{
	in_ddsiSubmessageFlags result =
		withFlagD ? IN_FLAG_DATA_D : 0;
	return result;
}



in_long
in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
        os_ushort kind,
        in_ddsiSerializer serializer)
{
    in_octet header[4];


    in_long total = 0;
    in_long result = -1;
    in_long nofOctets = 0;

    header[0] = ((in_octet)((kind >> 8) & 0xff));
    header[1] = ((in_octet)((kind) & 0xff));
    header[2] = 0; /* flags */
    header[3] = 0; /* flags */

    do {
        nofOctets = in_ddsiSerializerAppendOctets(serializer, header, 4);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while(0);

    return result;
}


/** */
in_long
in_ddsiSubmessageDataInitFromBuffer(
		in_ddsiSubmessageData _this,
		in_ddsiSubmessageHeader preparsedHeader,
		in_ddsiDeserializer deserializer)
{
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;

	/* copy the preparsed header */
	OS_SUPER(_this)->header = *preparsedHeader;

	/** The deserializer points onto first octet behind the submessage
	 * header */

    /*
     * Submessage-Header
     * 0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |   DATA        |X|X|X|X|K|D|Q|E|      octetsToNextHeader       |
     * +---------------+---------------+---------------+---------------+

     * Data-Body
     * 0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * +---------------+---------------+---------------+---------------+
     * |   Flags         extraFlags    |      octetsToInlineQos        |
     * +---------------+---------------+---------------+---------------+
     * | EntityId readerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * | EntityId writerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * + SequenceNumber writerSeqNum                                   +
     * |                                                               |
     * +---------------+---------------+---------------+---------------+

     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~ ParameterList inlineQos [only if Q==1]                        ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+

     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~ SerializedData serializedData [only if D==1 || K==1]          ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */

	/* break out in case of error */
	do {
        os_ushort dummy;
		nofOctets =
			in_ddsiDeserializerParseUshort(deserializer, &(_this->extraFlags));

		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiDeserializerParseUshort(deserializer, &dummy);

		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiEntityIdInitFromBuffer(&(_this->readerId), deserializer);

		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiEntityIdInitFromBuffer(&(_this->writerId), deserializer);

		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiSequenceNumberInitFromBuffer(&(_this->writerSN), deserializer);

		if (nofOctets<0) break;
		total += nofOctets;

		if (in_ddsiSubmessageHasFlagQ(OS_SUPER(_this))) {
			/* take inlineQos */
			nofOctets =
				in_ddsiParameterListInitFromBuffer(
						&(_this->inlineQos), deserializer);

			if (nofOctets<0) break;
			total += nofOctets;
		} else {
			in_ddsiParameterListInitEmpty(&(_this->inlineQos));
		}

		if (in_ddsiSubmessageHasFlagD(OS_SUPER(_this))) {
			/* take serialized data */
			const os_size_t remainingBodyLength =
				((os_size_t) preparsedHeader->octetsToNextHeader)
				- ((os_size_t)total);

			nofOctets =
				in_ddsiSerializedDataInitFromBuffer(
						&(_this->serializedPayload),
						deserializer,
						remainingBodyLength);

			if (nofOctets<0) break;

			total += nofOctets;
		} else {
			in_ddsiSerializedDataInitEmpty(&(_this->serializedPayload));
		}
		result = total;
	} while (0);

	return result;
}


/** total size, from first octet to last one
 *
 * not identical to octetsToNextHeader */
os_size_t
in_ddsiSubmessageDataSerializedSize(
        os_size_t  inlineQosSize,
        os_size_t  serializedPayloadSize)
{
	os_size_t result;

	if(serializedPayloadSize > 0)
	{
		result = IN_DDSI_SUBMESSAGE_HEADER_SIZE +
			IN_DDSI_SUBMESSAGE_DATA_HEADER_SIZE +inlineQosSize +
			IN_DDSI_ENCAPSULATION_HEADER_SIZE + serializedPayloadSize;
	} else
	{
		result = IN_DDSI_SUBMESSAGE_HEADER_SIZE +
		    IN_DDSI_SUBMESSAGE_DATA_HEADER_SIZE + inlineQosSize;
	}
    return result;
}


/** */
in_long
in_ddsiSubmessageDataHeaderSerializeInstantly(
		os_size_t  inlineQosSize,
		os_size_t  serializedPayloadSize,
		in_ddsiSequenceNumber  sequenceNumber, /* OSPL native representation */
		in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
		in_ddsiSerializer serializer)
{
    /*
     * Submessage-Header
     * 0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |   DATA        |X|X|X|X|K|D|Q|E|      octetsToNextHeader       |
     * +---------------+---------------+---------------+---------------+

     * Data-Body
     * 0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * +---------------+---------------+---------------+---------------+
     * |   Flags         extraFlags    |      octetsToInlineQos        |
     * +---------------+---------------+---------------+---------------+
     * | EntityId readerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * | EntityId writerEntityId                                       |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * + SequenceNumber writerSeqNum                                   +
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     * */

    /** the following pieces shall be serialized by callee
     *
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~ ParameterList inlineQos [only if Q==1]                        ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+

     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~ SerializedData serializedData [only if D==1 || K==1]          ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */
    const os_boolean withInlineQos = (inlineQosSize > 0);
    const os_boolean withSerializedPayload = (serializedPayloadSize > 0);
	const in_ddsiSubmessageKind  kind = IN_RTPS_DATA;
	const os_size_t submessageLength =
	    in_ddsiSubmessageDataSerializedSize(
	            inlineQosSize,
	            serializedPayloadSize);
	const in_ddsiSubmessageFlags flags =
		in_ddsiSubmessageCreateFlagE(serializer) |
		in_ddsiSubmessageDataCreateFlagQ(withInlineQos) |
		in_ddsiSubmessageDataCreateFlagD(withSerializedPayload);

    const os_ushort octetsToNextHeader =
         ((os_ushort) submessageLength) - (IN_DDSI_SUBMESSAGE_HEADER_SIZE);

    in_long result = -1;
    in_long total = 0;
    in_long nofOctets = 0;

	/** callee should have verified that total size does not exceed buffer size */
	assert(submessageLength <= in_ddsiSerializerRemainingCapacity(serializer));


    do {
        nofOctets =
            in_ddsiSubmessageHeaderSerializeInstantly(
                    kind,
                    flags,
                    octetsToNextHeader,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            dataHeaderSerializeInstantly(
                    readerId,
                    writerId,
                    sequenceNumber,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* final assignment */
        result = total;
    } while(0);

    return result;
}



/** */
in_long
in_ddsiSubmessageInfoTimestampSerializeInstantly(
		const c_time *timestamp,
		in_ddsiSerializer serializer)
{
	const in_ddsiSubmessageKind  kind = IN_INFO_TS;
	const in_ddsiSubmessageFlags flags =
		in_ddsiSubmessageCreateFlagE(serializer);
	const os_ushort nofOctetsToNextHeader =
		OS_SIZEOF(in_ddsiTime);

	in_long result = 0;
	in_long nofOctets;

	assert(nofOctetsToNextHeader == 8);

	nofOctets =
		in_ddsiSubmessageHeaderSerializeInstantly(
			kind,
			flags,
			nofOctetsToNextHeader,
			serializer);
	if (nofOctets < 0) {
		result = -1;
	} else {
		result += nofOctets;
		nofOctets =
			in_ddsiTimeInstantCTimeSerialize(
					timestamp,
					serializer,
					OS_FALSE /* isDuration */);
		if (nofOctets < 0) {
			result = -1;
		} else {
			result += nofOctets;
		}
	}

	return result;
}

/** */
in_long
in_ddsiSubmessageInfoDestinationSerializeInstantly(
        in_ddsiGuidPrefix destPrefix,
        in_ddsiSerializer serializer)
{
    const in_ddsiSubmessageKind  kind = IN_INFO_DST;
    const in_ddsiSubmessageFlags flags =
        in_ddsiSubmessageCreateFlagE(serializer);
    const os_ushort octetsToNextHeader =
        IN_DDSI_SUBMESSAGE_INFODESTINATION_BODY_SIZE;

    in_long result = -1; /* error condition */
    in_long nofOctets;
    in_long total = 0;

    assert(octetsToNextHeader == 12);
    assert(octetsToNextHeader % 4 == 0);

    do {
        nofOctets =
             in_ddsiSubmessageHeaderSerializeInstantly(
                     kind, flags, octetsToNextHeader,
                     serializer);
         if (nofOctets<0) break;
         total += nofOctets;

         nofOctets =
             in_ddsiGuidPrefixSerialize(destPrefix, serializer);
         if (nofOctets<0) break;
         total += nofOctets;

        /* done */
        result = total;
    } while (0);

    return result;
}

in_long
in_ddsiSubmessageHeartbeatSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumber firstSN,
        in_ddsiSequenceNumber lastSN,
        os_ushort count,
        in_ddsiSerializer serializer)
{
    const in_ddsiSubmessageKind  kind = IN_HEARTBEAT;
    const in_ddsiSubmessageFlags flags =
        in_ddsiSubmessageCreateFlagE(serializer) | in_ddsiSubmessageDataCreateFlagHeartbeatF(OS_FALSE);
    const os_ushort octetsToNextHeader =
        (os_ushort) (OS_SIZEOF(in_ddsiSubmessageHeartbeat) -
        IN_DDSI_SUBMESSAGE_HEADER_SIZE);
    OS_STRUCT(in_ddsiCount) countObj;

    in_long result = -1; /* error condition */
    in_long nofOctets;
    in_long total = 0;

    assert(octetsToNextHeader == 28);
    assert(octetsToNextHeader % 4 == 0);

    countObj.value = count;

    do {
        nofOctets =
            in_ddsiSubmessageHeaderSerializeInstantly(
                    kind, flags, octetsToNextHeader,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiEntityIdSerialize(
                    readerId,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiEntityIdSerialize(
                    writerId,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiSequenceNumberSerialize(
                    firstSN,
                    serializer);
        if (nofOctets<0) break;

        nofOctets =
            in_ddsiSequenceNumberSerialize(
                    lastSN,
                    serializer);
        if (nofOctets<0) break;

        total += nofOctets;
        nofOctets =
            in_ddsiCountSerialize(
                    &countObj,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* done */
        result = total;
    } while (0);

    return result;
}


/** */
in_long
in_ddsiSubmessageHeartbeatInitFromBuffer(
        in_ddsiSubmessageHeartbeat _this,
        in_ddsiSubmessageHeader preparsedHeader,
        in_ddsiDeserializer deserializer)
{
    in_long result = -1;
    in_long nofOctets = 0;
    in_long total = 0;

    /* copy the preparsed header */
    OS_SUPER(_this)->header = *preparsedHeader;

    /** The deserializer points onto first octet behind the submessage
     * header */

    /*
     * Submessage-Header
     *   0...2...........7...............15.............23...............31
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  | HEARTBEAT     |X|X|X|X|X|L|F|E|       octetsToNextHeader      |
     *  +---------------+---------------+---------------+---------------+
     *  |           EntityId              readerId                      |
     *  +---------------+---------------+---------------+---------------+
     *  |           EntityId              writerId                      |
     *  +---------------+---------------+---------------+---------------+
     *  |                                                               |
     *  +           SequenceNumber        firstSN                       +
     *  |                                                               |
     *  +---------------+---------------+---------------+---------------+
     *  |                                                               |
     *  +           SequenceNumber        lastSN                        +
     *  |                                                               |
     *  +---------------+---------------+---------------+---------------+
     *  |           Count                 count                         |
     *  +---------------+---------------+---------------+---------------+

     */

    /* break out in case of error */
    do {

        nofOctets = in_ddsiEntityIdInitFromBuffer(&(_this->readerId), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiEntityIdInitFromBuffer(&(_this->writerId), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSequenceNumberInitFromBuffer(&(_this->firstSN), deserializer);
        if (nofOctets<0) break;
        if (!in_ddsiSequenceNumberIsValid(&(_this->firstSN))) break; /* must not be zero or negative */
        total += nofOctets;

        nofOctets = in_ddsiSequenceNumberInitFromBuffer(&(_this->lastSN), deserializer);
        if (nofOctets<0) break;
        if (!in_ddsiSequenceNumberIsValid(&(_this->lastSN))) break; /* must not be zero or negative */
        if (in_ddsiSequenceNumberCompare(&(_this->lastSN), &(_this->firstSN)) == C_LT) break;
        total += nofOctets;

        nofOctets = in_ddsiCountInitFromBuffer(&(_this->count), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while (0);

    return result;
}


/** **************************************
 * ACKNACK
 ** ***************************************/

/**
 */
os_size_t
in_ddsiSubmessageAckNackSerializedSize(
        in_ddsiSequenceNumberSet sequenceNumberSet)
{
    const os_size_t snSetSize =
        /* variable length between 8+4+(M*4) octets, where M=0 in case of base==0 */
        in_ddsiSequenceNumberSetSerializedSize(sequenceNumberSet);
    const os_size_t result =
        IN_DDSI_SUBMESSAGE_HEADER_SIZE +
        OS_SIZEOF(in_ddsiEntityId) + /* 4 octets for reader Id */
        OS_SIZEOF(in_ddsiEntityId) + /* 4 octets for writer Id */
        snSetSize + /* 12-20 octets */
        OS_SIZEOF(in_ddsiCount); /* 4 octets for count */

    assert(OS_SIZEOF(in_ddsiEntityId) == 4);
    assert(OS_SIZEOF(in_ddsiCount) == 4);
    assert(IN_DDSI_SUBMESSAGE_HEADER_SIZE == 4);
    assert(snSetSize >= 12);
    assert(sequenceNumberSet->numBits!=0 || snSetSize == 12);

    assert(result % 4 == 0);

    return result;
}


/** */
in_long
in_ddsiSubmessageAckNackSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumberSet sequenceNumberSet,
        os_int32 count,
        in_ddsiSerializer serializer)
{
    /*
     *  0...2...........7...............15.............23...............31
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |   ACKNACK     |X|X|X|X|X|X|F|E|      octetsToNextHeader       |
     * +---------------+---------------+---------------+---------------+
     * |           EntityId              readerId                      |
     * +---------------+---------------+---------------+---------------+
     * |           EntityId             writerId                       |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~           SequenceNumberSet    readerSNState                  ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     * |              Count             count                          |
     * +---------------+---------------+---------------+---------------+
     */
    const in_ddsiSubmessageKind  kind = IN_ACKNACK;
    const in_ddsiSubmessageFlags flags =
        in_ddsiSubmessageCreateFlagE(serializer)/* | in_ddsiSubmessageDataCreateFlagAckNackF(OS_TRUE)*/;
    const os_ushort octetsToNextHeader =
        (os_ushort) (in_ddsiSubmessageAckNackSerializedSize(
                sequenceNumberSet) - IN_DDSI_SUBMESSAGE_HEADER_SIZE);
    OS_STRUCT(in_ddsiCount) countObj;

    in_long result = -1; /* error condition */
    in_long nofOctets;
    in_long total = 0;

    assert(octetsToNextHeader >= 24);
    assert(octetsToNextHeader % 4 == 0);

    countObj.value = count;

    do {
        nofOctets =
            in_ddsiSubmessageHeaderSerializeInstantly(
                    kind, flags, octetsToNextHeader,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiEntityIdSerialize(
                    readerId,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;


        nofOctets =
            in_ddsiEntityIdSerialize(
                    writerId,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;
        nofOctets =
            in_ddsiSequenceNumberSetSerialize(
                    sequenceNumberSet,
                    serializer);
        if (nofOctets<0) break;

        total += nofOctets;
        nofOctets =
            in_ddsiCountSerialize(
                    &countObj,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* done */
        result = total;
    } while (0);

    return result;
}


in_long
in_ddsiSubmessageAckNackInitFromBuffer(
        in_ddsiSubmessageAckNack _this,
        in_ddsiSubmessageHeader preparsedHeader,
        in_ddsiDeserializer deserializer)
{
    /*
        *  0...2...........7...............15.............23...............31
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |   ACKNACK     |X|X|X|X|X|X|F|E|      octetsToNextHeader       |
        * +---------------+---------------+---------------+---------------+
        * |           EntityId              readerId                      |
        * +---------------+---------------+---------------+---------------+
        * |           EntityId             writerId                       |
        * +---------------+---------------+---------------+---------------+
        * |                                                               |
        * ~           SequenceNumberSet    readerSNState                  ~
        * |                                                               |
        * +---------------+---------------+---------------+---------------+
        * |              Count             count                          |
        * +---------------+---------------+---------------+---------------+
        */
    in_long result = -1;
    in_long nofOctets = 0;
    in_long total = 0;

    /* copy the preparsed header */
    OS_SUPER(_this)->header = *preparsedHeader;

    /** The deserializer points onto first octet behind the submessage
     * header */


    /* break out in case of error */
    do {
        nofOctets =
            in_ddsiEntityIdInitFromBuffer(&(_this->readerId), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;
        nofOctets =
            in_ddsiEntityIdInitFromBuffer(&(_this->writerId), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;
        nofOctets =
            in_ddsiSequenceNumberSetInitFromBuffer(&(_this->readerSNState), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;
        nofOctets =
            in_ddsiCountInitFromBuffer(&(_this->count), deserializer);
        if (nofOctets<0) break;
        total += nofOctets;
        result = total;
    } while (0);

    return result;
}


/** **************************************
 * INFO_REPLY
 ** ***************************************/

static os_size_t
infoReplySerializedSize(
        os_size_t nofUnicastLocators,
         os_size_t nofMulticastLocators)

{
    const os_size_t top =
        IN_DDSI_SUBMESSAGE_HEADER_SIZE +
        sizeof(os_uint32) + /* length value of first list */
        (OS_SIZEOF(in_ddsiLocator) * nofUnicastLocators); /* N elems*/
    const os_size_t bottom =
        sizeof(os_uint32) +  /* length value of second list */
        (OS_SIZEOF(in_ddsiLocator) * nofMulticastLocators); /* M elems */

    /* conditional size */
    const os_size_t result =
        (nofMulticastLocators > 0)
        ? (top + bottom)
        : top;

    return result;
}

static in_long
locatorListSerialize(
        in_locatorList *locatorList,
        in_ddsiSerializer serializer)
{
    in_long result = -1;
    in_long nofOctets = 0;
    in_long total = 0;

    Coll_Iter* iter = Coll_List_getFirstElement(locatorList);
    /* iter may be 0 */
    while(iter) {
        in_locator locator =
            (in_locator) Coll_Iter_getObject(iter);
        if (!locator) break; /* result == -1 */

        /* first element of locator object is "Long" enforcing 4-octet
         * alignment. So no explicit alignment necessary here. */
        nofOctets =
            in_locatorSerialize(locator, serializer);
        if (nofOctets<0) break; /* result == -1 */
        total += nofOctets;

        /* see above, no explicit alignment necessary here */
    }
    result = total;

    return result;
}


/**
 */
os_size_t
in_ddsiSubmessageInfoReplySerializedSize(
        in_locatorList *unicastLocatorList,
        in_locatorList *multicastLocatorList)
{
    /**
     *  0...2...........8...............16.............24...............32
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * | INFO_REPLY    |X|X|X|X|X|X|M|E|     octetsToNextHeader        |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   unicastLocatorList                         ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   multicastLocatorList [ only if M==1 ]      ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */

    const os_size_t nofUnicastLocators =
            in_locatorListLength(unicastLocatorList);
    const os_size_t nofMulticastLocators =
            in_locatorListLength(multicastLocatorList);
    /* TODO current assumption that all locators are of equal size */
    const os_size_t result =
        infoReplySerializedSize(
                nofUnicastLocators,
                nofMulticastLocators);
    return result;
}

/** */
in_long
in_ddsiSubmessageInfoReplySerializeInstantly(
        in_locatorList *unicastLocatorList,
        in_locatorList *multicastLocatorList,
        in_ddsiSerializer serializer)
{
    /**
     *  0...2...........8...............16.............24...............32
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * | INFO_REPLY    |X|X|X|X|X|X|M|E|     octetsToNextHeader        |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   unicastLocatorList                         ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   multicastLocatorList [ only if M==1 ]      ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */

    const in_ddsiSubmessageKind  kind = IN_INFO_REPLY;
    const os_size_t nofUnicastLocators =
            in_locatorListLength(unicastLocatorList);
    const os_size_t nofMulticastLocators =
            in_locatorListLength(multicastLocatorList);
    const in_ddsiSubmessageFlags flags =
        in_ddsiSubmessageCreateFlagE(serializer) |
        in_ddsiSubmessageCreateFlagM(nofMulticastLocators);

    const os_ushort octetsToNextHeader =
        (os_ushort) (infoReplySerializedSize(
                nofUnicastLocators,
                nofMulticastLocators) -
        IN_DDSI_SUBMESSAGE_HEADER_SIZE);

    in_long result = -1; /* error condition */
    in_long nofOctets;
    in_long total = 0;

    assert(octetsToNextHeader % 4 == 0);


    do {
        nofOctets =
            in_ddsiSubmessageHeaderSerializeInstantly(
                    kind, flags, octetsToNextHeader,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiSerializerAppendUlong(
                    serializer,
                    (os_uint32) nofUnicastLocators);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            locatorListSerialize(
                    unicastLocatorList,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        if (nofMulticastLocators > 0) {
            assert(flags != 0x0);

            nofOctets =
                 in_ddsiSerializerAppendUlong(
                         serializer,
                         (os_uint32) nofMulticastLocators);
             if (nofOctets<0) break;
             total += nofOctets;

             nofOctets =
                 locatorListSerialize(
                         multicastLocatorList,
                         serializer);
             if (nofOctets<0) break;
             total += nofOctets;
        }
        /* done */
        result = total;
    } while (0);

    return result;
}


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
        in_ddsiDeserializer deserializer)
{
    /**
     *  0...2...........8...............16.............24...............32
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * | INFO_REPLY    |X|X|X|X|X|X|M|E|     octetsToNextHeader        |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   unicastLocatorList                         ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     * |                                                               |
     * ~      LocatorList   multicastLocatorList [ only if M==1 ]      ~
     * |                                                               |
     * +---------------+---------------+---------------+---------------+
     */

    in_long result = -1;

    return result;
}

