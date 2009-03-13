#ifndef NW_COMMONTYPES_H
#define NW_COMMONTYPES_H

#include "c_metabase.h"    /* For c_type */

#define NW_STRUCT(name)  struct name##_s
#define NW_EXTENDS(type) NW_STRUCT(type) _parent
#define NW_CLASS(name)   typedef NW_STRUCT(name) *name

typedef unsigned int    nw_seqNr;
typedef unsigned int    nw_address;
typedef char           *nw_name;
typedef unsigned char   nw_bool;

typedef void (*nw_onFatalCallBack)(c_voidp usrData);

#define NW_STRING_TERMINATOR ((char)'\0')

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#define FALSE              (0)
#define TRUE               (!FALSE)

#ifdef NULL
#undef NULL
#endif

#define NULL (0U)

#define NW_ID_UNDEFINED    (0U)



#endif /* NW_COMMONTYPES_H */

