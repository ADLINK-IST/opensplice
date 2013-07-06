/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef DLRL_TYPES_H
#define DLRL_TYPES_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_UTIL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef os_short            LOC_short;
typedef os_int32            LOC_long;
typedef os_ushort           LOC_unsigned_short;
typedef os_uint32           LOC_unsigned_long;
typedef os_float            LOC_float;
typedef os_double           LOC_double;
typedef os_char             LOC_char;
typedef os_uchar            LOC_octet;
typedef os_uchar            LOC_boolean;
typedef LOC_char *          LOC_string;
typedef const LOC_char *    LOC_const_string;
typedef void*               DLRL_LS_object;

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

struct DLRL_Exception_s;
typedef struct DLRL_Exception_s DLRL_Exception;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_TYPES_H */
