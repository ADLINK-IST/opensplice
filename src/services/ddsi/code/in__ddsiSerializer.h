/*
 * in__ddsiSerializer.h
 *
 *  Created on: Feb 18, 2009
 *      Author: frehberg
 */

#ifndef IN__DDSISERIALIZER_H_
#define IN__DDSISERIALIZER_H_

#include "in_commonTypes.h"
#include "in_abstractSendBuffer.h"

OS_STRUCT(in_ddsiSerializer)
{
	in_octet  *bufBeginAligned;
    in_octet  *bufEnd;
    in_octet  *bufWriter;
	os_boolean requiresSwap;
};

/** \brief init
 */
void
in_ddsiSerializerInit(
		in_ddsiSerializer _this,
		in_abstractSendBuffer buffer,
		os_boolean bigEndian);

/** \brief init
 */
os_boolean
in_ddsiSerializerInitNil(
		in_ddsiSerializer _this);


/** \brief init from existing serializer with different endianess */
void
in_ddsiSerializerInitFrom(
		in_ddsiSerializer _this,
		in_ddsiSerializer from,
		os_boolean bigEndian);

/** \brief init
 */
os_boolean
in_ddsiSerializerInitNoOp(
        in_ddsiSerializer _this);

/** \brief init with default endianess
 */
void
in_ddsiSerializerInitWithDefaultEndianess(
		in_ddsiSerializer _this,
		in_abstractSendBuffer buffer);

/** \brief deinit
 */
void
in_ddsiSerializerDeinit(
		in_ddsiSerializer _this);



#endif /* IN__DDSISERIALIZER_H_ */
