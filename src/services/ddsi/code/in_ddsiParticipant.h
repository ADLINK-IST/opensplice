/*
 * in_ddsiParticipant.h
 *
 *  Created on: Feb 26, 2009
 *      Author: frehberg
 */

#ifndef IN_DDSIPARTICIPANT_H_
#define IN_DDSIPARTICIPANT_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "kernelModule.h"

#include "in_ddsiDeserializer.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define in_ddsiDiscoveredParticipantData(Data) \
    ((in_ddsiDiscoveredParticipantData)Data)

#define in_ddsiDiscoveredParticipantDataFree(c) \
    in_objectFree(in_object(c))

#define in_ddsiDiscoveredParticipantDataKeep(c) \
    in_ddsiDiscoveredParticipantData(in_objectKeep(in_object(c)))

#define in_ddsiDiscoveredParticipantDataIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_DISCOVERED_PARTICIPANT_DATA)

/** */
in_ddsiDiscoveredParticipantData
in_ddsiDiscoveredParticipantDataNew(void);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIPARTICIPANT_H_ */
