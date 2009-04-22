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

/** */
os_boolean
in_ddsiParameterListInitFromEncapsulation(
        in_ddsiParameterList _this,
        in_ddsiSerializedData serializedData);

/** init  */
os_boolean
in_ddsiParameterListInitEmpty(in_ddsiParameterList _this);

/* may return with "out of memory" */
in_result
in_ddsiParameterListForParticipantParse(
        in_ddsiParameterList _this,
        in_ddsiDiscoveredParticipantData data);

/* may return with "out of memory" */
in_result
in_ddsiParameterListForReaderParse(
    in_ddsiParameterList _this,
    in_ddsiDiscoveredReaderData data,
    c_base base);

/* may return with "out of memory" */
in_result
in_ddsiParameterListForWriterParse(
    in_ddsiParameterList _this,
    in_ddsiDiscoveredWriterData data,
    c_base base);

/** */
in_long
in_ddsiParameterListForParticipantSerializeInstantly(
    in_connectivityParticipantFacade facade,
    in_ddsiSerializer serializer,
    in_endpointDiscoveryData discoveryData);

/** */
in_long
in_ddsiParameterListForPublicationSerializeInstantly(
    in_connectivityWriterFacade facade,
    in_ddsiSerializer serializer,
    in_endpointDiscoveryData discoveryData);

/** */
in_long
in_ddsiParameterListForSubscriptionSerializeInstantly(
    in_connectivityReaderFacade facade,
    in_ddsiSerializer serializer,
    in_endpointDiscoveryData discoveryData);


/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForParticipantCalculateSize(
    in_connectivityParticipantFacade facade,
    in_endpointDiscoveryData discoveryData);

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForPublicationCalculateSize(
    in_connectivityWriterFacade facade,
    in_endpointDiscoveryData discoveryData);

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForSubscriptionCalculateSize(
    in_connectivityReaderFacade facade,
    in_endpointDiscoveryData discoveryData);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIPARAMETERLIST_H_ */
