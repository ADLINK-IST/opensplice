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
