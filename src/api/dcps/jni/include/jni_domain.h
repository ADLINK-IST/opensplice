
/**@file api/dcps/jni/include/jni_domain.h
 * @brief The jni_domain object can be used to create a new domain (Partition in
 * DCPS terminology).
 */
 
#ifndef JNI_DOMAIN_H
#define JNI_DOMAIN_H

#include "jni_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief JNI domain mapping, which contains a user domain and the name.
 */
C_STRUCT(jni_domain){
    const c_char* name; /*!The name of the domain.*/
    u_domain udomain;   /*!The user domain.*/
};

#define jni_domain(d) ((jni_domain)(d))

/**@brief Creates a new domain.
 * 
 * The domain will be attached to the supplied participant.
 * 
 * @param p The participant to attach the domain to.
 * @param name The name of the domain.
 * @param qos The domain QoS.
 * @return The newly created domain.
 */
jni_domain  jni_domainNew       (jni_participant p, 
                                 const c_char* name,
                                 v_domainQos qos);

/**@brief Detaches the domain and frees its resources.
 * 
 * @param domain The domain to clean up.
 * @return JNI_RESULT_OK if succeeded, JNI_RESULT_BAD_PARAMETER otherwise.
 */
jni_result  jni_domainFree      (jni_domain domain);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_DOMAIN_H */
