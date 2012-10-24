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
#ifndef IN_DDSI_SERIALIZER_H
#define IN_DDSI_SERIALIZER_H

#include "in_commonTypes.h"


/** */
os_boolean
in_ddsiSerializerIsBigEndian(in_ddsiSerializer serializer);

/** \brief Write a sequence of octets to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendOctets(
		in_ddsiSerializer _this,
		const in_octet* valPtr,
		in_ulong valLength);

/** append 4 octets to buffer without alignment
 *
 * This is a convenience function which allows to implement a number of type-serializations
 * via macros. */
in_long
in_ddsiSerializerAppendOctets_4(
        in_ddsiSerializer _this,
        const in_octet val0,
        const in_octet val1,
        const in_octet val2,
        const in_octet val3);

/* TODO ES should not be in header, move back to C file */
in_long
appendOctets(in_ddsiSerializer _this,
					         const in_octet *valPtr,
					         in_ulong valOctetLength);

/** \brief Append a value to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendLong(
		in_ddsiSerializer _this,
		os_int32 value);

/** \brief Append a value to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendUlong(
		in_ddsiSerializer _this,
		os_uint32 value);

/** \brief Append a value to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendShort(
		in_ddsiSerializer _this,
		os_short value);

/** \brief Append a value to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendUshort(
		in_ddsiSerializer _this,
		os_ushort value);

/** \brief Append a value to buffer
 * \return -1 on error otherwise the number of octets written
 * */
in_long
in_ddsiSerializerAppendOctet(
		in_ddsiSerializer _this,
		in_octet value);

/** \brief Number of remaining octet space in buffer until max content length reached
 * \return positive integer
 * */
os_size_t
in_ddsiSerializerRemainingCapacity(
		in_ddsiSerializer _this);


/** \brief Request writer pointer position
 * */
in_octet*
in_ddsiSerializerGetPosition(
		in_ddsiSerializer _this);

/** \brief Seek forward the writer pointer by N octets.
 * */
in_long
in_ddsiSerializerSeek(
		in_ddsiSerializer _this,
		os_size_t  nOctets);

/** \brief Seek forward the writer pointer to given address.
 *
 * The position must not be before the current index position.
 * */
in_long
in_ddsiSerializerSeekTo(
        in_ddsiSerializer _this,
        in_octet *position);

/** \brief Seek forward the writer pointer by N octets.
 * */
os_size_t
in_ddsiSerializerNofOctets(
		in_ddsiSerializer _this);


/** \brief Align the writer index.
 *
 * Boundary may be one of 2,4, or 8.
 *
 * Returns a value from 0..(boundary-1)
 */
in_long
in_ddsiSerializerAlign(
		in_ddsiSerializer _this,
		in_long boundary);


/* \brief Calculate the required padding for
 * _this and the specified offset.
 *
 * Argument _this may be NULL
 *
 * Returns a value from 0..(boundary-1)*/
in_long
in_ddsiSerializerAlignmentPaddingSize(
		in_ddsiSerializer _this,
		in_long offset,
		in_long boundary);

/* \brief Append string to the buffer
 *
 * Returns a value from 0..(boundary-1)*/
in_long
in_ddsiSerializerAppendString(
		in_ddsiSerializer _this,
		const os_char *str);

#endif /* IN_DDSI_SERIALIZER_H */
