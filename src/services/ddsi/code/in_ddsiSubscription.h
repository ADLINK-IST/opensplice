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
#ifndef IN_DDSISUBSCRIPTION_H_
#define IN_DDSISUBSCRIPTION_H_

#include "in_commonTypes.h"

#if defined (__cplusplus)
extern "C" {
#endif


#define in_ddsiDiscoveredReaderData(Data) ((in_ddsiDiscoveredReaderData)Data)

#define in_ddsiDiscoveredReaderDataFree(c) in_objectFree(in_object(c))

#define in_ddsiDiscoveredReaderDataKeep(c) in_ddsiDiscoveredReaderData(in_objectKeep(in_object(c)))

#define in_ddsiDiscoveredReaderDataIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_DISCOVERED_READER_DATA)

in_ddsiDiscoveredReaderData
in_ddsiDiscoveredReaderDataNew(void);


#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISUBSCRIPTION_H_ */
