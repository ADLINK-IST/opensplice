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
#ifndef DLRL_UTIL_H
#define DLRL_UTIL_H

/* includes here */
#include "DLRL_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_UTIL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define DLRL_ALLOC_WITH_SIZE(_this, size, exception, ...) \
    _this = os_malloc(size); \
    DLRL_VERIFY_ALLOC(_this, (exception), __VA_ARGS__)

#define DLRL_ALLOC(_this, type, exception, ...) \
    DLRL_ALLOC_WITH_SIZE(_this, sizeof(type), (exception), __VA_ARGS__)

/* allow strdup of null pointer strings as well, it will just return null */
#define DLRL_STRDUP(_this, source, exception, ...) \
    do { \
        if(source){ \
            LOC_long size = (strlen(source) + 1); \
            _this = (LOC_string)os_malloc(size); \
            DLRL_VERIFY_ALLOC(_this, exception, __VA_ARGS__)\
            os_strncpy(_this, source, size); \
        } else { \
            _this = NULL; \
        } \
    } while(0)

#define DLRL_VERIFY_ALLOC(_this, exception, ...) \
    if (!_this) { \
        DLRL_Exception_THROW((exception), DLRL_OUT_OF_MEMORY, __VA_ARGS__); \
    }
/* NOT IN DESIGN */
OS_API char *
DLRL_Util_userResultToString(
    u_result result);

/* NOT IN DESIGN */
OS_API char*
DLRL_Util_writeResultToString(
    v_writeResult result);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_UTIL_H */
