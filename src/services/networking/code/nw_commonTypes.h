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

#ifndef NW_COMMONTYPES_H
#define NW_COMMONTYPES_H

#include "os_socket.h"
#include "c_metabase.h"    /* For c_type */

#define NW_STRUCT(name)  struct name##_s
#define NW_EXTENDS(type) NW_STRUCT(type) _parent
#define NW_CLASS(name)   typedef NW_STRUCT(name) *name

typedef os_uint32     nw_seqNr;
typedef os_char      *nw_name;
typedef os_boolean    nw_bool;
typedef os_address    nw_size;

/*
 * SenderInfo contains all information about a message sender.
 * This may be used for for authentication (subject lookup) but used for
 * other things too
 */
NW_CLASS(nw_senderInfo);
NW_STRUCT(nw_senderInfo) {
    os_sockaddr_storage ipAddress;
    const char* dn; /* reference, does not take ownership*/
};

#define NW_STRING_TERMINATOR ((char)'\0')

typedef void (*nw_onFatalCallBack)(c_voidp usrData);

#ifndef FALSE
#define FALSE              (0)
#endif

/*windows.h defines TRUE to 1*/
#ifdef TRUE
#undef TRUE
#endif
#define TRUE               (!FALSE)

#ifdef NULL
#undef NULL
#endif

#define NULL (0U)

#define NW_ID_UNDEFINED    (0U)


typedef enum nw_eq_e {NW_UNDEFINED, NW_LT, NW_EQ, NW_GT} nw_eq;


#endif /* NW_COMMONTYPES_H */

