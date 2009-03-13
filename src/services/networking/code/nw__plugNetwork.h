
#ifndef NW__PLUGNETWORK_H
#define NW__PLUGNETWORK_H

#include "nw_plugTypes.h"
#include "nw_plugNetwork.h"
#include "os_abstract.h"

#ifdef PA_BIG_ENDIAN
#define nw_plugHostToNetwork(value) (value)
#define nw_plugNetworkToHost(value) (value)
#endif

#ifdef PA_LITTLE_ENDIAN
nw_seqNr nw_plugByteSwap(nw_seqNr value);
#define nw_plugHostToNetwork(value) nw_plugByteSwap(value)
#define nw_plugNetworkToHost(value) nw_plugByteSwap(value)
#endif

#endif /* NW__PLUGNETWORK_H */
