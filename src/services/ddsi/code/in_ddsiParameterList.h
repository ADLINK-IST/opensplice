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
 * in_ddsiParameterList.h
 *
 *  Created on: Feb 24, 2009
 *      Author: frehberg
 */

#ifndef IN_DDSIPARAMETERLIST_H_
#define IN_DDSIPARAMETERLIST_H_

#include "in__object.h"
#include "in_ddsiSerializer.h"
#include "in_ddsiDeserializer.h"
#include "in_connectivityWriterFacade.h"
#include "in_connectivityReaderFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** init */
in_long
in_ddsiParameterListInitFromBuffer(in_ddsiParameterList _this,
		in_ddsiDeserializer deserializer);

/** init  */
os_boolean
in_ddsiParameterListInitEmpty(in_ddsiParameterList _this);


/** */
in_long
in_ddsiParameterListForParticipantSerializeInstantly(
		in_connectivityParticipantFacade facade,
		in_ddsiSerializer serializer);

/** */
in_long
in_ddsiParameterListForPublicationSerializeInstantly(
		in_connectivityWriterFacade facade,
		in_ddsiSerializer serializer);

/** */
in_long
in_ddsiParameterListForSubscriptionSerializeInstantly(
		in_connectivityReaderFacade facade,
		in_ddsiSerializer serializer);


/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForParticipantCalculateSize(
        in_connectivityParticipantFacade facade);

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForPublicationCalculateSize(
        in_connectivityWriterFacade facade);

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForSubscriptionCalculateSize(
        in_connectivityReaderFacade facade);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIPARAMETERLIST_H_ */
