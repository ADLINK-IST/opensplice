/*
 * in_ddsiSubmessageDeserializer.c
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiSubmessageDeserializer.h"

/* **** implementation headers **** */
#include "in__ddsiDeserializer.h"
#include "in_ddsiElements.h"

/* **** private functions **** */

/* **** public functions **** */

os_boolean
in_ddsiSubmessageDeserializerInit(
		in_ddsiSubmessageDeserializer _this,
		in_ddsiSubmessageToken   token,
		os_size_t  maxNofOctets)
{
	const os_size_t headerSize =
		OS_SIZEOF(in_ddsiSubmessageHeader);

	os_boolean result = OS_TRUE;

	assert(headerSize == IN_DDSI_SUBMESSAGE_HEADER_SIZE);

	if (maxNofOctets < headerSize) {
		/* error: header would exceed the buffer */
		result = OS_FALSE;
	} else {
		os_boolean isBigEndian;
		os_size_t octetsToNextHeader = 0;

		in_ddsiDeserializerInitWithDefaultEndianess(
				OS_SUPER(_this), token, headerSize);
		in_ddsiSubmessageHeaderInitFromBuffer(
				&(_this->submessageHeader),
				OS_SUPER(_this));

		isBigEndian = in_ddsiSubmessageHeaderIsBigEndian(&(_this->submessageHeader));
		/* now reinit again with correct endianess */
		in_ddsiDeserializerInitRaw(OS_SUPER(_this), token, headerSize, isBigEndian);
		in_ddsiSubmessageHeaderInitFromBuffer(
				&(_this->submessageHeader), OS_SUPER(_this));
		octetsToNextHeader =
			(os_size_t) (_this->submessageHeader.octetsToNextHeader);
		/* finally adapt the end-pointer */
		if (octetsToNextHeader + headerSize > maxNofOctets) {
			/* error: submessage exceeding available buffer */
			result = OS_FALSE;
		} else {
			OS_SUPER(_this)->bufEnd += octetsToNextHeader;
		}
	}

	return result;
}


/** \brief deinit
 */
void
in_ddsiSubmessageDeserializerDeinit(
		in_ddsiSubmessageDeserializer _this)
{
	in_ddsiDeserializerDeinit(OS_SUPER(_this));
}
