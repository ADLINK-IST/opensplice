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
#ifndef DLRL_EXCEPTION_H
#define DLRL_EXCEPTION_H

/* DCPS includes */
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

#ifdef __ospl_func__
#undef __ospl_func__
#endif
    
#if ! defined  ( _MSC_VER ) && ! defined ( INTEGRITY )
#define __ospl_func__ __func__
#else /* _MSC_VER */
#define __ospl_func__ __FUNCTION__

#endif /* _MSC_VER */
    
#define DLRL_NO_EXCEPTION                    0u
#define DLRL_ERROR                           0x8000000u
#define DLRL_DCPS_ERROR                      (DLRL_ERROR + 1)
#define DLRL_BAD_HOME_DEFINITION             (DLRL_ERROR + 2)
#define DLRL_NOT_FOUND                       (DLRL_ERROR + 3)
#define DLRL_ALREADY_EXISTING                (DLRL_ERROR + 4)
#define DLRL_PRECONDITION_NOT_MET            (DLRL_ERROR + 5)
#define DLRL_ALREADY_DELETED                 (DLRL_ERROR + 6)
#define DLRL_NO_SUCH_ELEMENT                 (DLRL_ERROR + 7)
#define DLRL_SQL_ERROR                       (DLRL_ERROR + 8)
#define DLRL_BAD_PARAMETER                   (DLRL_ERROR + 9)
#define DLRL_INVALID_OBJECTS                 (DLRL_ERROR + 10)
#define DLRL_TIMEOUT                         (DLRL_ERROR + 11)

#define DLRL_STANDARD_ERROR                  0x4000000u
#define DLRL_OUT_OF_MEMORY                   (DLRL_STANDARD_ERROR + 1)

#define DLRL_MAX_EXCEPTION_MESSAGE_SIZE      2048

struct DLRL_Exception_s {
    LOC_long exceptionID;
    char exceptionMessage[DLRL_MAX_EXCEPTION_MESSAGE_SIZE];
};

#define DLRL_Exception_THROW(_this, id, ...) \
  DLRL_Exception_throw(_this, id, (char*)#id , __VA_ARGS__); \
  DLRL_Exception_PROPAGATE(_this)

#define DLRL_Exception_PROPAGATE(_this) \
  do { if (DLRL_NO_EXCEPTION != (_this)->exceptionID) { \
    DLRL_Exception_propagate(_this, (char*)__FILE__, __LINE__, (char*)__ospl_func__);  \
    goto DLRL_EXCEPTION_EXIT; \
  } } while (0)

#define DLRL_Exception_EXIT(_this) \
  DLRL_EXCEPTION_EXIT:

/* NOT IN DESIGN */
#define DLRL_Exception_PROPAGATE_RESULT(_this, ...)\
    DLRL_Exception_transformResultToException(_this, __VA_ARGS__);\
    DLRL_Exception_PROPAGATE(_this);

/* NOT IN DESIGN */
#define DLRL_Exception_PROPAGATE_WRITE_RESULT(_this,  ...)\
    DLRL_Exception_transformWriteResultToException(_this,  __VA_ARGS__);\
    DLRL_Exception_PROPAGATE(_this);

/* NOT IN DESIGN */
#define DLRL_VERIFY_NOT_NULL(exception, param, name) \
  if (!param) DLRL_Exception_THROW((exception), DLRL_BAD_PARAMETER, "%s parameter may not be null.", name)

OS_API void
DLRL_Exception_init(
    DLRL_Exception *_this);

OS_API void
DLRL_Exception_throw(
    DLRL_Exception *_this,
    LOC_long exceptionID,
    char *exceptionName,
    ...);

OS_API void
DLRL_Exception_propagate(
    DLRL_Exception *_this,
    char *file,
    int line,
    char *function);

/* NOT IN DESIGN */
OS_API void
DLRL_Exception_transformResultToException(
    DLRL_Exception *_this,
    ...);

/* NOT IN DESIGN */

OS_API void
    DLRL_Exception_transformWriteResultToException(
    DLRL_Exception *_this,
    ...);

OS_API char *
DLRL_Exception_exceptionIDToString(
    LOC_long exceptionID);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_EXCEPTION_H */
