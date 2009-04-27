/*
 * in_ddsiSerializer.c
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

/* interface */
#include "in__ddsiDeserializer.h"

/* implementation */
#include <assert.h>
#include "in_commonTypes.h"
#include "in_align.h"
#include "in__endianness.h"
#include "in_abstractReceiveBuffer.h"
#include "in__ddsiSerializer.h"
#include "in_ddsiSerializedData.h"
#include "in_report.h"

/*  --------- inline macros --------- */
#define IN_ALIGN_ADDRESS(_address,_boundary) \
    IN_ALIGN_PTR_CEIL(_address,_boundary)

#define IN_TAIL_ALIGNED_ADDRESS(_address,_boundary) \
	IN_ALIGN_PTR_FLOOR(_address,_boundary)

#define IN_DESERIALIZER_HAS_MORE_N(_d,_n) \
	((((_d)->bufReader) +  (os_size_t)(_n)) <= ((_d)->bufEnd))

/*  --------- private operations --------- */

static os_boolean
_requiresSwap(os_boolean bigEndianess)
{
#ifdef PA_BIG_ENDIAN
    return !bigEndianess;
#else
    return bigEndianess;
#endif
}


/*  --------- public operations --------- */


/** \brief init
 */
void
in_ddsiDeserializerInit(
		in_ddsiDeserializer _this,
		in_abstractReceiveBuffer dataBuffer,
		os_boolean bigEndian)
{
	in_octet *begin = in_abstractReceiveBufferBegin(dataBuffer);

	_this->bufEnd = begin + in_abstractReceiveBufferLength(dataBuffer);
	_this->bufReader = begin;
	_this->requiresSwap = _requiresSwap(bigEndian);

	/* verify elementary alignment tests */
	assert(UI2P(0x00) == IN_ALIGN_ADDRESS(0x0,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x1,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x2,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x3,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x4,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x5,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x6,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x7,8));
	assert(UI2P(0x08) == IN_ALIGN_ADDRESS(0x8,8));
	assert(UI2P(0x10) == IN_ALIGN_ADDRESS(0x9,8));
}

/** \brief init
 */
void
in_ddsiDeserializerInitRaw(
		in_ddsiDeserializer _this,
		in_octet* buffer,
		os_size_t bufferLength,
		os_boolean bigEndian)
{
	assert(((os_size_t)buffer) % 4 == 0);

	_this->bufReader = buffer;
	_this->bufEnd    = buffer + bufferLength;
	_this->requiresSwap = _requiresSwap(bigEndian);
}

void
in_ddsiDeserializerInitWithDefaultEndianess(
		in_ddsiDeserializer _this,
		in_octet*  buffer,
		os_size_t  bufferLength)
{
	assert(((os_size_t)buffer) % 4 == 0);

	_this->bufReader = buffer;
	_this->bufEnd    = buffer + bufferLength;
	_this->requiresSwap = OS_FALSE;
}


void
in_ddsiDeserializerInitFromSerializer(
        in_ddsiDeserializer _this,
        in_ddsiSerializer serializer)
{
    assert(((os_size_t)serializer->bufBeginAligned) % 4 == 0);

    _this->bufReader = serializer->bufBeginAligned;
    _this->bufEnd = serializer->bufEnd;
    _this->requiresSwap = serializer->requiresSwap;
}


/**
 * */
void
in_ddsiDeserializerDeinit(in_ddsiDeserializer _this)
{
}

/**
 * */
in_long
in_ddsiDeserializerParseOctets(
		in_ddsiDeserializer _this,
		in_octet *dest,
		in_long numOctets)
{
	in_long result;

	if (!IN_DESERIALIZER_HAS_MORE_N(_this, numOctets)) {
		result = -1; /* error case, buffer exceeded */
	} else {
		memcpy(dest, (_this->bufReader), numOctets);
		_this->bufReader += numOctets;
		result = numOctets;
	}

	return result;
}

/**
 * */
in_long
in_ddsiDeserializerReferenceOctets(
		in_ddsiDeserializer _this,
		in_octet **ptr,
		in_long numOctets)
{
	in_long result;

	if (!IN_DESERIALIZER_HAS_MORE_N(_this, numOctets)) {
		result = -1; /* error case, buffer exceeded */
	} else {
		*ptr = _this->bufReader;
		_this->bufReader += numOctets;
		result = numOctets;
	}

	return result;
}


/**
 * */
in_long
in_ddsiDeserializerReferenceString(
		in_ddsiDeserializer _this,
		os_char **ptr,
		os_uint32 *strLen)
{
	os_uint32 cdrStrLen;
	in_long result;
	in_long nOctets;

	nOctets = in_ddsiDeserializerParseUlong(_this, &cdrStrLen);
	if (nOctets < 0) {
		result = -1;
	} else {
		result = nOctets;
		nOctets =
			in_ddsiDeserializerReferenceOctets(_this,
					(in_octet**)ptr,
					cdrStrLen);

		/* check error cases  */
		if (nOctets < 0) {
			result = -1;
		} else {
			assert(nOctets == (in_long)cdrStrLen);
			/* enforce null-termination of string */
			(*ptr)[cdrStrLen-1] = '\0';
			if (strLen) {
				*strLen = cdrStrLen - 1;
			}
			result += nOctets;
		}
	}

	return result;
}


/**
 * */
in_long
in_ddsiDeserializerParseOctet(
		in_ddsiDeserializer _this,
		in_octet *value)
{
	in_long result;

	if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
		result = -1;
	} else {
		*value = *((in_octet*) (_this->bufReader));
		_this->bufReader += sizeof(*value);
		result = sizeof(*value);
	}

	return result;
}

/**
 * */
in_long
in_ddsiDeserializerParseUshort(
		in_ddsiDeserializer _this,
		os_ushort *value)
{
	in_long result;
	assert(sizeof(*value) == 2);
	assert(_this);

	{
		const in_octet *bufReaderBak = _this->bufReader;

		/* align the read pointer */
		_this->bufReader =
			IN_ALIGN_ADDRESS(_this->bufReader, sizeof(*value));

		if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
			result = -1;
		} else {
			/* fast path */
			if (_this->requiresSwap) {
				/* this operation requires address alignment by 2 */
				*value = IN_UINT16_SWAP_LE_BE(*((os_ushort*) (_this->bufReader)));
			}
			else {
				/* this operation requires address alignment by 2 */
				*value = *((os_ushort*) (_this->bufReader));
			}

			_this->bufReader += sizeof(*value);

			result =  _this->bufReader - bufReaderBak; /* n-octets */
		}
	}
	return result;
}

/** \brief Parse a unsigned short from buffer
 *
 * Overwrites the default endianess. This operation is used to
 * parse the submessage header.
 *
 * \return the number of octets consumed, -1 in case of error */
in_long
in_ddsiDeserializerParseUshortWithEndianess(
		in_ddsiDeserializer _this,
		os_ushort *value,
		os_boolean bigEndian)
{
	in_long result;
	assert(sizeof(*value) == 2);
	assert(_this);

	{
		const in_octet *bufReaderBak = _this->bufReader;

		/* align the read pointer */
		_this->bufReader =
			IN_ALIGN_ADDRESS(_this->bufReader, sizeof(*value));

		if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
			result = -1;
		} else {
			/* fast path */
			if (_requiresSwap(bigEndian)) {
				/* this operation requires address alignment by 2 */
				*value = IN_UINT16_SWAP_LE_BE(*((os_ushort*) (_this->bufReader)));
			}
			else {
				/* this operation requires address alignment by 2 */
				*value = *((os_ushort*) (_this->bufReader));
			}

			_this->bufReader += sizeof(*value);

			result =  _this->bufReader - bufReaderBak; /* n-octets */
		}
	}
	return result;
}



/**
 * */
in_long
in_ddsiDeserializerParseShort(
		in_ddsiDeserializer _this,
		os_short *value)
{
	in_long result;
	assert(sizeof(*value) == 2);
	assert(_this);

	{
		const in_octet *bufReaderBak = _this->bufReader;

		/* align the read pointer */
		_this->bufReader =
			IN_ALIGN_ADDRESS(_this->bufReader, sizeof(*value));

		if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
			result = -1;
		} else {
			/* fast path */
			if (_this->requiresSwap) {
				/* this operation requires address alignment by 2 */
				*value = IN_UINT16_SWAP_LE_BE(*((os_short*) (_this->bufReader)));
			}
			else {
				/* this operation requires address alignment by 2 */
				*value = *((os_short*) (_this->bufReader));
			}

			_this->bufReader += sizeof(*value);

			result =  _this->bufReader - bufReaderBak; /* n-octets */
		}
	}
	return result;
}


/**
 * */
in_long
in_ddsiDeserializerParseUlong(
		in_ddsiDeserializer _this,
		os_uint32 *value)
{
	in_long result;
	assert(sizeof(*value) == 4);
	assert(_this);

	{
		const in_octet *bufReaderBak = _this->bufReader;

		/* align the read pointer */
		_this->bufReader =
			IN_ALIGN_ADDRESS(_this->bufReader, sizeof(*value));

		if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
			result = -1;
		} else {
			/* fast path */
			if (_this->requiresSwap) {
				/* this operation requires address alignment by 2 */
				*value = IN_UINT32_SWAP_LE_BE(*((os_uint32*) (_this->bufReader)));
			}
			else {
				/* this operation requires address alignment by 2 */
				*value = *((os_uint32*) (_this->bufReader));
			}

			_this->bufReader += sizeof(*value);

			result =  _this->bufReader - bufReaderBak; /* n-octets */
		}
	}
	return result;
}

/**
 * */
in_long
in_ddsiDeserializerParseLong(
		in_ddsiDeserializer _this,
		os_int32 *value)
{
	in_long result;
	assert(sizeof(*value) == 4);
	assert(_this);

	{
		const in_octet *bufReaderBak = _this->bufReader;

		/* align the read pointer */
		_this->bufReader =
			IN_ALIGN_ADDRESS(_this->bufReader, sizeof(*value));

		if (!IN_DESERIALIZER_HAS_MORE_N(_this, sizeof(*value))) {
			result = -1;
		} else {
			/* fast path */
			if (_this->requiresSwap) {
				/* this operation requires address alignment by 2 */
				*value = IN_UINT32_SWAP_LE_BE(*((os_int32*) (_this->bufReader)));
			}
			else {
				/* this operation requires address alignment by 2 */
				*value = *((os_int32*) (_this->bufReader));
			}

			_this->bufReader += sizeof(*value);

			result =  _this->bufReader - bufReaderBak; /* n-octets */
		}
	}
	return result;
}

/**
 */
os_boolean
in_ddsiDeserializerIsBigEndian(
		in_ddsiDeserializer _this)
{
	os_boolean result;

	#ifdef PA_BIG_ENDIAN
	    return !(_this->requiresSwap);
	#else
	    return (_this->requiresSwap);
	#endif

	return result;
}

/**
 * */
in_long
in_ddsiDeserializerNofUnreadOctets(
		in_ddsiDeserializer _this)
{
	in_long result;

	assert(_this->bufReader <=  _this->bufEnd);

	result = _this->bufEnd - _this->bufReader;

	return result;
}

/**
 * */
in_long
in_ddsiDeserializerSeek(
		in_ddsiDeserializer _this,
		os_size_t  nOctets)
{
	in_long result;

	if (!IN_DESERIALIZER_HAS_MORE_N(_this, nOctets)) {
		result = -1; /* error case */
	} else {
		_this->bufReader += nOctets;
		result = nOctets;
	}
	return result;
}


/**
 * */
in_octet*
in_ddsiDeserializerGetIndex(
		in_ddsiDeserializer _this)
{
	return _this->bufReader;
}

/**
 * */
in_long
in_ddsiDeserializerAlign(
		in_ddsiDeserializer _this,
		in_long boundary)
{
	in_long result;
	const in_octet *bufReaderBak = _this->bufReader;

	_this->bufReader =
		IN_ALIGN_ADDRESS(_this->bufReader, boundary);

	if (_this->bufReader > _this->bufEnd) {
		result = -1;
	} else {
		result = _this->bufReader- bufReaderBak;
	}

	return result;
}



/* \brief Calculate the required padding for
 * _this and the specified offset.
 *
 * _this may be NULL */
in_long
in_ddsiDeserializerAlignmentPaddingSize(
		in_ddsiDeserializer _this,
		in_long offset,
		in_long boundary)
{
	const in_octet *ptr = _this ? _this->bufReader : NULL;
	const in_octet *ptrWithOffset = ptr + offset;
	in_long result;

	result =
		UI(IN_ALIGN_ADDRESS((ptrWithOffset), boundary) - ptrWithOffset);

	return result;
}

