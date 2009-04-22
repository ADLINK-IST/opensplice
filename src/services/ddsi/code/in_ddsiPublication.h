/*
 * in_ddsiPublication.h
 *
 *  Created on: Feb 27, 2009
 *      Author: frehberg
 */

#ifndef IN_DDSIPUBLICATION_H_
#define IN_DDSIPUBLICATION_H_

#include "in_commonTypes.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define in_ddsiDiscoveredWriterData(Data) ((in_ddsiDiscoveredWriterData)Data)

#define in_ddsiDiscoveredWriterDataFree(c) in_objectFree(in_object(c))

#define in_ddsiDiscoveredWriterDataKeep(c) in_ddsiDiscoveredWriterData(in_objectKeep(in_object(c)))

#define in_ddsiDiscoveredWriterDataIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_DISCOVERED_WRITER_DATA)

in_ddsiDiscoveredWriterData
in_ddsiDiscoveredWriterDataNew(void);


#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIPUBLICATION_H_ */
