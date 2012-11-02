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
#ifndef IN_DDSI_DESERIALIZER_H_
#define IN_DDSI_DESERIALIZER_H_

#include "in_commonTypes.h"

/**
 */
os_boolean
in_ddsiDeserializerIsBigEndian(
		in_ddsiDeserializer _this);

/** \brief return number of unread octets
 */
in_long
in_ddsiDeserializerNofUnreadOctets(
		in_ddsiDeserializer _this);


/** */
in_long
in_ddsiDeserializerSeek(
		in_ddsiDeserializer _this,
		os_size_t nOctets); /* nOctets matches pointer byte-size */


/** \brief Parse the specific number of octets from buffer into the destination buffer.
 *
 * Return current read pointer and increment read index by N ,
 * -1 in case the valid buffer has been exceeded */
in_long
in_ddsiDeserializerParseOctets(
		in_ddsiDeserializer _this,
		in_octet *dest,
		in_long numOctets);

/** \brief Parse the specific number of octets from buffer
 *
 * Return current read pointer and increment read index by N,
 * -1 in case the valid buffer has been exceeded  */
in_long
in_ddsiDeserializerReferenceOctets(
		in_ddsiDeserializer _this,
		in_octet **valPtr,
		in_long valLength);


/** \brief Parse a single octet from buffer, increment the read index by 1
 *
 * Return -1 in case the valid buffer has been exceeded, otherwise 1.
 */
in_long
in_ddsiDeserializerParseOctet(
		in_ddsiDeserializer _this,
		in_octet *valPtr);

/** \brief reference string in buffer
 * */
in_long
in_ddsiDeserializerReferenceString(
		in_ddsiDeserializer _this,
		os_char **ptr,
		os_uint32 *strLen);

/** \brief Parse a short from buffer
 *
 * \return the number of octets consumed, -1 in case of error */
in_long
in_ddsiDeserializerParseShort(
		in_ddsiDeserializer _this,
		os_short *value);
/** \brief Parse a unsigned short from buffer
 *
 * \return the number of octets consumed, -1 in case of error */
in_long
in_ddsiDeserializerParseUshort(
		in_ddsiDeserializer _this,
		os_ushort *value);

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
		os_boolean bigEndian);

/** \brief Parse a long from buffer
 *
 * \return the number of octets consumed, -1 in case of error */
in_long
in_ddsiDeserializerParseLong(
		in_ddsiDeserializer _this,
		os_int32 *value);

/** \brief Parse a unsigned long from buffer
 *
 * \return the number of octets consumed, -1 in case of error */
in_long
in_ddsiDeserializerParseUlong(
		in_ddsiDeserializer _this,
		os_uint32 *value);

/**
 * \return pointer to current index
 */
in_octet*
in_ddsiDeserializerGetIndex(
		in_ddsiDeserializer _this);

/** \brief Align the reader index.
 *
 * Boundary may be one of 2,4, or 8.
 *
 * \return a value from 0..(boundary-1)
 */
in_long
in_ddsiDeserializerAlign(
		in_ddsiDeserializer _this,
		in_long boundary);

/* \brief Calculate the required padding for
 * _this and the specified offset.
 *
 * Argument _this may be NULL
 *
 * \return a value from 0..(boundary-1)*/
in_long
in_ddsiDeserializerAlignmentPaddingSize(
		in_ddsiDeserializer _this,
		in_long offset,
		in_long boundary);

/** \brief parse length encoded UTF8 string
 *
 * Allocates a new buffer to store the string in, to be free()-d  */
in_long
in_ddsiDeserializerParseString(
		in_ddsiDeserializer _this,
		os_char **str);



#endif /* IN_DDSI_DESERIALIZER_H_ */
