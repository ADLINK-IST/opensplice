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

/**@file api/dcps/jni/include/jni_misc.h
 * 
 * @brief The jni_misc object provides some additional functions that don't belong to
 * any specific object. 
 * 
 * Currently it provides facilities to convert u_result
 * objects to jni_result objects as well as functionality to convert both
 * types of results to a string representation.
 */
#ifndef JNI_MISC_H
#define JNI_MISC_H

#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSJNI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
 
/**@brief Converts a u_result object to a jni_result object.
 * 
 * @param ur The u_result object to convert to jni_result.
 * @return The jni_result representation of the u_result object.
 */
OS_API jni_result      jni_convertResult       (u_result ur);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* JNI_MISC_H */
