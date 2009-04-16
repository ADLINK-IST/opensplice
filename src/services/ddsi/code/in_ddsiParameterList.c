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
 * in_ddsiParameterList.c
 *
 *  Created on: Mar 3, 2009
 *      Author: frehberg
 */

#include "in__ddsiParameterList.h"

#include "in_ddsiSerializer.h"
#include "in_connectivityWriterFacade.h"
#include "kernelModule.h"
#include "in__ddsiParameter.h"
#include "in_ddsiParameterList.h"
#include "in_ddsiDeserializer.h"

/** */
static in_long
in_ddsiParameterListSeekToEnd(in_ddsiDeserializer deserializer)
{
	OS_STRUCT(in_ddsiParameterHeader) header;
	os_boolean continueScan = OS_TRUE;
	in_long nofOctets = 0;
	in_long total = 0;
	in_long result = -1;

	/* skip all parameters to find end of list */
	while (continueScan) {
		nofOctets =
			in_ddsiParameterHeaderInitFromBuffer(&header, deserializer);
		if (nofOctets < 0) {
			continueScan = OS_FALSE;
			result = -1;
		} else {
			/* header parsed successfully, now
			 * check for Sentinel, otherwise seek the body*/
			total += nofOctets;
			if (header.id.value == IN_PID_SENTINEL) {
				/* end of list reached */
				continueScan = OS_FALSE;
				result = total;
			} else {
				/* seek body of parameter */
				nofOctets = in_ddsiDeserializerSeek(deserializer,
						(os_size_t) header.octetsToNextParameter);
				if (nofOctets < 0) {
					continueScan = OS_FALSE; /* error */
					result = -1;
				} else {
					total += nofOctets;
				}
			}
		}
	}

	return result;
}


/** */
in_long
in_ddsiParameterListInitFromBuffer(in_ddsiParameterList _this,
		in_ddsiDeserializer deserializer)
{
	in_long result = -1;

	in_ddsiParameterToken firstParameter =
		in_ddsiDeserializerGetIndex(deserializer);
	os_boolean isBigEndian =
		in_ddsiDeserializerIsBigEndian(deserializer);
	in_long totalOctetLength =
		in_ddsiParameterListSeekToEnd(deserializer);

	if (totalOctetLength < 0) {
		result = -1;
		/* init */
		in_ddsiParameterListInitEmpty(_this);
	} else {
		result = totalOctetLength;
		_this->firstParameter = firstParameter;
		_this->isBigEndian = isBigEndian;
		_this->totalOctetLength = (os_size_t) totalOctetLength;
	}

	return result;
}

/**  */
os_boolean
in_ddsiParameterListInitEmpty(in_ddsiParameterList _this)
{
	_this->firstParameter = NULL;
	_this->totalOctetLength = 0L;
	_this->isBigEndian = OS_FALSE;
	return OS_TRUE;
}


/** */
in_long
in_ddsiParameterListForPublicationSerializeInstantly(
		in_connectivityWriterFacade facade,
		in_ddsiSerializer serializer)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;


	struct v_publicationInfo *info =
		in_connectivityWriterFacadeGetInfo(facade);

	do {

        /* write sentinel */
        total =
            in_ddsiSerializerAppendUshort(serializer, IN_PID_SENTINEL) +
            in_ddsiSerializerAppendUshort(serializer, 0);


		/* final assignment */
		result = total;
	} while (0);

	return  result;
}



/** */
in_long
in_ddsiParameterListForSubscriptionSerializeInstantly(
		in_connectivityReaderFacade facade,
		in_ddsiSerializer serializer)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;
	struct v_subscriptionInfo *info =
		in_connectivityReaderFacadeGetInfo(facade);

	do {
        /* write sentinel */
        total =
            in_ddsiSerializerAppendUshort(serializer, IN_PID_SENTINEL) +
            in_ddsiSerializerAppendUshort(serializer, 0);

		/* final assignment */
		result = total;
	} while (0);

	return  result;
}


/** */
in_long
in_ddsiParameterListForParticipantSerializeInstantly(
		in_connectivityParticipantFacade facade,
		in_ddsiSerializer serializer)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
	in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;
	struct v_participantInfo *info =
		in_connectivityParticipantFacadeGetInfo(facade);

	do {

	    /* write sentinel */
        total =
            in_ddsiSerializerAppendUshort(serializer, IN_PID_SENTINEL) +
            in_ddsiSerializerAppendUshort(serializer, 0);

		/* final assignment */
		result = total;
	} while (0);

	return  result;
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForParticipantCalculateSize(
        in_connectivityParticipantFacade facade)
{
    return 4; /* sentinel size */
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForPublicationCalculateSize(
        in_connectivityWriterFacade facade)
{
    return 4; /* sentinel size */
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForSubscriptionCalculateSize(
        in_connectivityReaderFacade facade)
{
    return 4; /* sentinel size */
}
