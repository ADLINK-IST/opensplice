
#ifndef D_STOREXML_H
#define D_STOREXML_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_storeXML(s) ((d_storeXML)(s))

d_storeXML      d_storeNewXML               ();

d_storeResult   d_storeFreeXML              (d_storeXML store);

#if defined (__cplusplus)
}
#endif

#endif /*D_STOREXML_H*/
