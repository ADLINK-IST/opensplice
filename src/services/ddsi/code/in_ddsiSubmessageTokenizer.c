/*
 * in_ddsiSubmessageTokenizer.c
 *
 *  Created on: Feb 24, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiSubmessageTokenizer.h"

/* **** implementation headers **** */
#include "in_ddsiSubmessageDeserializer.h"
#include "in_ddsiSubmessage.h"

/* **** private functions **** */

/* **** public functions **** */
/** skips the ddsi-message header and points to first submessage
 */
os_boolean
in_ddsiSubmessageTokenizerInit(in_ddsiSubmessageTokenizer _this,
		in_abstractReceiveBuffer receiveBuffer)
{
	const in_ulong headerSize =
		OS_SIZEOF(in_ddsiMessageHeader);

	assert (receiveBuffer!=NULL);

	{
		in_octet *begin =
			in_abstractReceiveBufferBegin(receiveBuffer);
		in_ulong length =
			in_abstractReceiveBufferLength(receiveBuffer);

		if (length < headerSize) {
			_this->nextToken = NULL; /* error case */
			_this->remainingLength = 0;
		} else {
			_this->nextToken = begin + headerSize;
			_this->remainingLength = length - headerSize;
		}
	}
	return OS_TRUE;
}

/** skips the ddsi-message header and points to first submessage
 */
void
in_ddsiSubmessageTokenizerDeinit(in_ddsiSubmessageTokenizer _this)
{
	/* nop */
	_this->nextToken = NULL;
	_this->remainingLength = 0;
}

/** Get next token
 *
 * Is Parsing the next submessage header, and verifying the length.
 *  \return NULL on parse error
 */
in_ddsiSubmessageToken
in_ddsiSubmessageTokenizerGetNext(
		in_ddsiSubmessageTokenizer _this,
		in_ddsiSubmessageDeserializer deserializer)
{
	in_ddsiSubmessageToken result = NULL; /* NULL indicates end of buffer */
	assert(_this!=NULL);

	if (_this->nextToken==NULL) {
		result = NULL;
	} else {
		/* implements CDR deserialization */
		if (_this->remainingLength == 0) {
		    /* end of list */
		    result = NULL;
		} else if (!in_ddsiSubmessageDeserializerInit(
				deserializer,
				in_ddsiSubmessageTokenGetPtr(_this->nextToken),
				_this->remainingLength  /* may be 0 */)) {
			/* parse error */
			result = NULL;
		} else {
			/* store current pointer as result and
			 * seek 'nextToken' to next token in buffer */
			result = _this->nextToken;

			/* deserializers bufEnd points to first octet of next token */
			_this->nextToken =
				OS_SUPER(deserializer)->bufEnd;
			/* subtract from remainingLength the distance of current
			 * token and next token  */
			assert(UI(_this->nextToken) > UI(result));
			_this->remainingLength -=
				UI(_this->nextToken) - UI(result);
		}
	}
	return result;
}
