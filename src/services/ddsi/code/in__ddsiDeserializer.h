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
#ifndef IN__DDSIDESERIALIZER_H_
#define IN__DDSIDESERIALIZER_H_

/* interface */
#include "in_ddsiDeserializer.h"

/* dependencies */
#include "in_abstractReceiveBuffer.h"
#include "in_commonTypes.h"
#include "in_ddsiSubmessageToken.h"

/** \brief class */
OS_STRUCT(in_ddsiDeserializer)
{
	in_octet   *bufEnd;
	in_octet   *bufReader;
	os_boolean  requiresSwap;
};

/** \brief init
 */
void
in_ddsiDeserializerInit(
		in_ddsiDeserializer _this,
		in_abstractReceiveBuffer dataBuffer,
		os_boolean bigEndian);

/** \brief init
 */
void
in_ddsiDeserializerInitRaw(
		in_ddsiDeserializer _this,
		in_octet* buffer,
		os_size_t bufferLength,
		os_boolean bigEndian);

/** \brief init
 */
void
in_ddsiDeserializerInitWithDefaultEndianess(
		in_ddsiDeserializer _this,
		in_octet*  buffer,
		os_size_t  bufferLength);

/* Initialize from existing serializer.
 * This initialization method is used if data has to be deserialized
 * which was written into a serializer before */
void
in_ddsiDeserializerInitFromSerializer(
        in_ddsiDeserializer _this,
        in_ddsiSerializer serializer);


/** \brief deinit
 */
void
in_ddsiDeserializerDeinit(
		in_ddsiDeserializer deserializer);

#endif /* IN__DDSIDESERIALIZER_H_ */
