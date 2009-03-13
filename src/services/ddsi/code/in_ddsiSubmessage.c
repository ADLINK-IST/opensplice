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

/* **** private functions **** */
static in_long
in_ddsiSubmessageDataSerializeMessage(
		v_message message,
		in_ddsiSerializer serializer)
{
	in_long result = 0;
	in_octet *buffer =
		in_ddsiSerializerGetPosition(serializer);

	os_size_t bufferLength =
		in_ddsiSerializerRemainingCapacity(serializer);

	/* do the v_message serialization here */

	return result;
}

static in_long
dataHeaderSerializeInstantly(
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        c_ulong osplSequenceNumber,
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
	OS_STRUCT(in_ddsiEntityId) entityIdUnknown =
		UINT32_TO_ENTITYID(IN_ENTITYID_UNKNOWN);

	os_ushort extraFlags = 0U;
	os_ushort octetsToInlineQos =16U; /*may become variable in future */
	/* Note: 16 octets, ospl is using just 8 octets */
	OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber;
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;

	in_ddsiSequenceNumberInit(
			&sequenceNumber,
			osplSequenceNumber);

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

		nofOctets = in_ddsiSequenceNumberSerialize(&sequenceNumber, serializer);
		if (nofOctets<0) break; /* leave while loop */
		total += nofOctets;

		assert(total == 16);

		/* final assignment */
		result = total;
	} while (0);
	/* result == -1 || result > 0 */
	assert(result != 0);

	return result;
}

/* **** public functions **** */

/** */
void
in_ddsiSerializedDataInit(
		in_ddsiSerializedData _this,
		in_octet *begin,
		os_size_t length)
{
	_this->begin = begin;
	_this->length = length;
}

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

	in_octet *begin =
		in_ddsiDeserializerGetIndex(deserializer);

	_this->begin = NULL;
	_this->length = 0;

	 if ((((os_size_t)begin) & (4-1)) > 0) {
		IN_REPORT_WARNING("in_ddsiSerializedDataInitFromBuffer",
				"serialized data not aligned by 4");
	 }

	do {
		nofOctets =
			in_ddsiDeserializerSeek(deserializer, length);
		if (nofOctets < 0) break;
		total += nofOctets;

		/* succeeded */
		result = total;
		_this->begin = begin;
		_this->length = length;
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
		in_ddsiSerializeIsBigEndian(serializer);
	in_ddsiSubmessageFlags result =
		isBigE ? 0 : IN_FLAG_E;
	return result;
}


static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagI(os_boolean expectsInlineQos)
{
	in_ddsiSubmessageFlags result =
		expectsInlineQos ? IN_FLAG_DATA_I : 0;
	return result;
}


static in_ddsiSubmessageFlags
in_ddsiSubmessageDataCreateFlagD(os_boolean withFlagD)
{
	in_ddsiSubmessageFlags result =
		withFlagD ? IN_FLAG_DATA_D : 0;
	return result;
}


static os_size_t
readerFacadeSerializedSize(
        in_connectivityReaderFacade facade)
{
    os_size_t result = 0;

    return 0;
}

static os_size_t
writerFacadeSerializedSize(
        in_connectivityWriterFacade facade)
{
    os_size_t result = 0;

    return 0;
}

static os_size_t
participantFacadeSerializedSize(
        in_connectivityParticipantFacade facade)
{
    os_size_t result = 0;

    return 0;
}


in_long
in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
        os_ushort kind,
        in_ddsiSerializer serializer)
{
    os_ushort flags = 0;
    in_long total = 0;
    in_long result = -1;
    in_long nofOctets = 0;

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer, kind);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer, flags);
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
		nofOctets =
			in_ddsiDeserializerParseUshort(deserializer, &(_this->extraFlags));
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

		if (in_ddsiSubmessageHasFlagI(OS_SUPER(_this))) {
			/* take inlineQos */
			nofOctets =
				in_ddsiParameterListInitFromBuffer(&(_this->inlineQos), deserializer);
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
    os_size_t result =
        IN_DDSI_SUBMESSAGE_HEADER_SIZE +
        IN_DDSI_SUBMESSAGE_DATA_HEADER_SIZE +
        inlineQosSize +
        IN_DDSI_ENCAPSULATION_HEADER_SIZE
        + serializedPayloadSize;
    return result;
}


/** */
in_long
in_ddsiSubmessageDataHeaderSerializeInstantly(
		os_size_t  inlineQosSize,
		os_size_t  serializedPayloadSize,
		c_ulong    sequenceNumber, /* OSPL native representation */
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
	const in_ddsiSubmessageKind  kind = IN_DATA;
	const os_size_t submessageLength =
	    in_ddsiSubmessageDataSerializedSize(
	            inlineQosSize,
	            serializedPayloadSize);
	const in_ddsiSubmessageFlags flags =
		in_ddsiSubmessageCreateFlagE(serializer) |
		in_ddsiSubmessageDataCreateFlagI(withInlineQos) |
		in_ddsiSubmessageDataCreateFlagD(withSerializedPayload);

    const os_ushort octetsToNextHeader =
         ((os_ushort) submessageLength) - IN_DDSI_SUBMESSAGE_HEADER_SIZE;

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

#if 0
/** */
in_long
in_ddsiSubmessageParticipantDataSerializeInstantly(
		in_connectivityParticipantFacade facade,
		os_size_t dataSubmessageLength,
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

	const in_ddsiSubmessageKind  kind = IN_DATA;
	const in_ddsiSubmessageFlags flags =
		in_ddsiSubmessageCreateFlagE(serializer) |
		in_ddsiSubmessageDataCreateFlagD(OS_TRUE);

	const os_ushort submessageHeaderSize = 4; /* nof octets */
	const os_ushort octetsToNextHeader =
		dataSubmessageLength - submessageHeaderSize;
	in_long result = -1;
	in_long total = 0;
	in_long nofOctets = 0;
    /*TODO: correct sequence number */
    c_ulong seqnum = 0;

	do {
		nofOctets =
			in_ddsiSubmessageHeaderSerializeInstantly(
					IN_DATA,
					flags,
					octetsToNextHeader,
					serializer);
		if (nofOctets<0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiSubmessageDataHeaderSerializeInstantly(
					in_connectivityEntityFacade(facade),
					seqnum,
					serializer);
		if (nofOctets<0) break;
		total += nofOctets;

		/* parameter list as serialized data */
		{
			os_boolean isBigEndian =
				in_ddsiSerializeIsBigEndian(serializer);

			/* add encapsulation header */
			os_ushort encapsKind =
				isBigEndian
				? IN_ENCAPSULATION_PL_CDR_BE
				: IN_ENCAPSULATION_PL_CDR_LE;

			nofOctets =
				in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
						encapsKind,
						serializer);
			if (nofOctets<0) break; /* leave while loop */
			total += nofOctets;

			nofOctets =
				in_ddsiParameterListForParticipantSerializeInstantly(
					facade,
					serializer);
			if (nofOctets<0) break; /* leave while loop */
			total += nofOctets;
		}

		/* final assignment */
		result = total;
	} while(0);

	return result;
}
#endif

/** */
in_long
in_ddsiSubmessageInfoTimestampSerializeInstantly(
		c_time *timestamp,
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
