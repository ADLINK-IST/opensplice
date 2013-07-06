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

/**@file api/dcps/jni/include/jni_nameService.h
 * 
 * @brief jni_nameService provides facilities to look up a Splice2v3 kernel according
 * to a domainId of a DCPS Domain. 
 * 
 * Currently its function are dummy functions,
 * because there exists no mapping from domainId's to kernel objects.
 */
 
#ifndef JNI_NAMESERVICE_H
#define JNI_NAMESERVICE_H

#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Nameservice that contains mapping from domainId to kernel.
 */
C_STRUCT(jni_nameService){
    int refCount;       /*! Number of references; if 0 it can be freed.*/
};

#define jni_nameService(ns) ((jni_nameService)(ns))

/**@brief Initializes a name service.
 * 
 * @return The newly created name service.
 */
jni_nameService
jni_nameServiceNew ();

/**@brief Frees the memory allocated for the name service.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
jni_result
jni_nameServiceFree ();

/**@brief Looks up the kernel URI that is associated with the supplied Domain.
 * 
 * @param domainId The Domain to look for.
 * @return The associated kernel URI, or NULL if it could not be found.
 */
const c_char*
jni_nameServiceResolveURI (
    c_long domainId);

c_bool
jni_nameServiceAddDomain (
    const c_char* uri);


#if defined (__cplusplus)
}
#endif

#endif /* JNI_NAMESERVICE_H */
