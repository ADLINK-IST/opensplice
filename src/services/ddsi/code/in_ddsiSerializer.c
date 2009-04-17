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
/*
 * in_serializer.c
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

/* interface */
#include "in__ddsiSerializer.h"

/* implementation */
#include "in_commonTypes.h"
#include "in__endianness.h"
#include "in_align.h"
#include "in_abstractSendBuffer.h"
#include "in_ddsiDefinitions.h"

/*  --------- inline macros --------- */
#define IN_ALIGN_ADDRESS(_address,_boundary) \
    IN_ALIGN_PTR_CEIL(_address,_boundary)

#define IN_TAIL_ALIGNED_ADDRESS(_address,_boundary) \
	IN_ALIGN_PTR_FLOOR(_address,_boundary)

#define IN_SERIALIZER_FITS_N(_d,_n) \
	((((_d)->bufWriter) +  (os_size_t)(_n)) <= ((_d)->bufEnd))


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
in_ddsiSerializerInit(
		in_ddsiSerializer _this,
		in_abstractSendBuffer buffer,
		os_boolean bigEndian)
{
	const os_size_t   length = in_abstractSendBufferLength(buffer);
	/* begin ptr must be aligned by 4 */
	in_octet *begin = in_abstractSendBufferBegin(buffer);
	/* end ptr must be aligned by 4 */
	in_octet *end   = (begin +  length);

	/* verify that the buffer is already aligned */
	assert(begin == IN_ALIGN_ADDRESS(begin, IN_DDSI_MESSAGE_ALIGNMENT));

	/* align beginning and ensure capacity is multiple of alignTo */
	_this->bufBeginAligned = begin;
	_this->bufEnd = end;
	_this->bufWriter = _this->bufBeginAligned;
	_this->requiresSwap = _requiresSwap(bigEndian);

	assert(_this->bufBeginAligned < _this->bufEnd);
	assert(_this->bufWriter < _this->bufEnd);
	assert(_this->bufWriter == _this->bufBeginAligned);

	/* zero out the buffer */
	memset(begin, 0, length);

	/* elementary alignment tests */

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

/** */
void
in_ddsiSerializerInitWithDefaultEndianess(
		in_ddsiSerializer _this,
		in_abstractSendBuffer buffer)
{
	os_boolean bigEndian;
#ifdef PA_BIG_ENDIAN
	/* Big endian is native encoding */
	bigEndian = TRUE;
#else
	/* Little endian is native encoding */
	bigEndian = FALSE;;
#endif
	in_ddsiSerializerInit(_this, buffer, bigEndian);
}

/** \brief init
 */
void
in_ddsiSerializerInitNil(
		in_ddsiSerializer _this)
{
	_this->bufBeginAligned = NULL;
	_this->bufEnd = NULL;
	_this->bufWriter = NULL;
	_this->requiresSwap = OS_FALSE;
}

void
in_ddsiSerializerDeinit(in_ddsiSerializer _this)
{
}


/** */
os_boolean
in_ddsiSerializeIsBigEndian(in_ddsiSerializer _this)
{
	os_boolean result;

#ifdef PA_BIG_ENDIAN
    result = !(_this->requiresSwap);
#else
    result = _this->requiresSwap;
#endif

    return result;
}


in_long
in_ddsiSerializerAppendOctets(in_ddsiSerializer _this,
					         const in_octet *valPtr,
					         in_long valOctetLength)
{
	in_long result;

	assert(valOctetLength > 0);

	if (!IN_SERIALIZER_FITS_N(_this, valOctetLength)) {
		result = -1;
	} else {
		memcpy(_this->bufWriter, valPtr, valOctetLength);
		result = valOctetLength;
	}

	return valOctetLength;
}


in_long
in_ddsiSerializerAppendLong(in_ddsiSerializer _this,
					       os_int32 value)
{
	const in_octet *writerBakup = _this->bufWriter;
	in_long result;

	assert(sizeof(value) == 4);

	_this->bufWriter = IN_ALIGN_ADDRESS(_this->bufWriter, sizeof(value));

	if (!IN_SERIALIZER_FITS_N(_this, sizeof(value))) {
		result = -1;
	} else {
		if (_this->requiresSwap) {
			/* this operation requires address alignment by 4 */
			*((os_int32*) (_this->bufWriter)) = IN_UINT32_SWAP_LE_BE(value);
		}
		else {
			/* this operation requires address alignment by 4 */
			*((os_int32*) (_this->bufWriter)) = value;
		}

		_this->bufWriter += sizeof(value);
		result = _this->bufWriter - writerBakup;
		assert(result > 0);
	}

	return result;
}



in_long
in_ddsiSerializerAppendUlong(in_ddsiSerializer _this,
					         os_uint32 value)
{
	const in_octet *writerBakup = _this->bufWriter;
	in_long result;

	assert(sizeof(value) == 4);

	_this->bufWriter = IN_ALIGN_ADDRESS(_this->bufWriter, sizeof(value));

	if (!IN_SERIALIZER_FITS_N(_this, sizeof(value))) {
		result = -1;
	} else {
		if (_this->requiresSwap) {
			/* this operation requires address alignment by 4 */
			*((os_uint32*) (_this->bufWriter)) = IN_UINT32_SWAP_LE_BE(value);
		}
		else {
			/* this operation requires address alignment by 4 */
			*((os_uint32*) (_this->bufWriter)) = value;
		}

		_this->bufWriter += sizeof(value);
		result = _this->bufWriter - writerBakup;
		assert(result > 0);
	}

	return result;
}



in_long
in_ddsiSerializerAppendShort(in_ddsiSerializer _this,
					         os_short value)
{
	const in_octet *writerBakup = _this->bufWriter;
	in_long result;

	assert(sizeof(value) == 2);

	_this->bufWriter = IN_ALIGN_ADDRESS(_this->bufWriter, sizeof(value));

	if (!IN_SERIALIZER_FITS_N(_this, sizeof(value))) {
		result = -1;
	} else {
		if (_this->requiresSwap) {
			/* this operation requires address alignment by 2 */
			*((os_short*) (_this->bufWriter)) = IN_UINT16_SWAP_LE_BE(value);
		}
		else {
			/* this operation requires address alignment by 2 */
			*((os_short*) (_this->bufWriter)) = value;
		}

		_this->bufWriter += sizeof(value);
		result = _this->bufWriter - writerBakup;
		assert(result > 0);
	}

	return result;
}



in_long
in_ddsiSerializerAppendUshort(in_ddsiSerializer _this,
					         os_ushort value)
{
	const in_octet *writerBakup = _this->bufWriter;
	in_long result;

	assert(sizeof(value) == 2);

	_this->bufWriter = IN_ALIGN_ADDRESS(_this->bufWriter, sizeof(value));

	if (!IN_SERIALIZER_FITS_N(_this, sizeof(value))) {
		result = -1;
	} else {
		if (_this->requiresSwap) {
			/* this operation requires address alignment by 4 */
			*((os_ushort*) (_this->bufWriter)) = IN_UINT16_SWAP_LE_BE(value);
		}
		else {
			/* this operation requires address alignment by 4 */
			*((os_ushort*) (_this->bufWriter)) = value;
		}

		_this->bufWriter += sizeof(value);
		result = _this->bufWriter - writerBakup;
		assert(result > 0);
	}

	return result;
}


in_long
in_ddsiSerializerAppendOctet(in_ddsiSerializer _this,
					        in_octet value)
{
	in_long result;

	if (!IN_SERIALIZER_FITS_N(_this, sizeof(value))) {
		result = -1;
	} else {
		* (_this->bufWriter) = value;
		_this->bufWriter += sizeof(value);
		result = sizeof(value);

		assert(result > 0);
	}

	return result; /* n-octets */
}

os_size_t
in_ddsiSerializerRemainingCapacity(
		in_ddsiSerializer _this)
{
	os_size_t result;

	result = _this->bufEnd - _this->bufWriter;

	return result;
}

/** \brief Request writer pointer position
 * */
in_octet*
in_ddsiSerializerGetPosition(
		in_ddsiSerializer _this)
{
	in_octet *result = _this->bufWriter;

	return result;
}

in_long
in_ddsiSerializerSeek(
		in_ddsiSerializer _this,
		os_size_t  nOctets)
{
	in_long result;

	assert(nOctets > 0);

	if (!IN_SERIALIZER_FITS_N(_this, nOctets)) {
			result = -1;
	} else {
		_this->bufWriter += (os_size_t) nOctets;
		result = nOctets;

		assert(result > 0);
	}

	return result;
}

in_long
in_ddsiSerializerSeekTo(
        in_ddsiSerializer _this,
        in_octet *position)
{
    in_long result;

    if (position < _this->bufWriter ||
        /* new position not within array range */
        position > _this->bufEnd) {
        result = -1;
    } else {
        result = (in_long) (position - _this->bufWriter);
        _this->bufWriter = position;
    }

    return result;
}


/** \brief Seek forward the writer pointer by N octets.
 * */
os_size_t
in_ddsiSerializerNofOctets(
		in_ddsiSerializer _this)
{
	os_size_t result;

	result = _this->bufWriter - _this->bufBeginAligned;

	return result;
}


in_long
in_ddsiSerializerAlign(
		in_ddsiSerializer _this,
		in_long boundary)
{
	const in_octet *bufWriterBak = _this->bufWriter;
	in_long result;

	_this->bufWriter =
		IN_ALIGN_ADDRESS(_this->bufWriter, boundary);

	if (!IN_SERIALIZER_FITS_N(_this, boundary)) {
		result = -1;
	} else {
		result = UI(_this->bufWriter - bufWriterBak);
		assert(result > 0);
	}
	return result;
}

/* \brief Calculate the required padding for
 * _this and the specified offset.
 *
 * _this may be NULL */
in_long
in_ddsiSerializerAlignmentPaddingSize(
		in_ddsiSerializer _this,
		in_long offset,
		in_long boundary)
{
	const in_octet *ptr = _this ? _this->bufWriter : NULL;
	const in_octet *ptrWithOffset = ptr + offset;
	in_long result =
		UI(IN_ALIGN_ADDRESS((ptrWithOffset), boundary) - ptrWithOffset);
	return result;
}

/* \brief Append string to the buffer
 *
 * Returns a value from 0..(boundary-1)*/
in_long
in_ddsiSerializerAppendString(
		in_ddsiSerializer _this,
		os_char *str)
{
	/* Inclusive the terminating '\0' character */
	const in_long cdrStrLen = strlen(str) + 1;

	in_long result;
	in_long nOctets;

	/* align by 4, nOctets may be 4..7 on success */
	nOctets = in_ddsiSerializerAppendUlong(_this, cdrStrLen);
	if (nOctets<0) {
		result = -1;
	} else {
		result = nOctets;
		nOctets =
			in_ddsiSerializerAppendOctets(_this, (in_octet*) str, cdrStrLen);
		if (nOctets < 0) {
			result = -1;
		} else {
			result += nOctets;
		}
	}
	return result;
}


