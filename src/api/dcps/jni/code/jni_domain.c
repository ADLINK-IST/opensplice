/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "v_kernel.h"
#include "u_domain.h"
#include "jni_domain.h"
#include "jni_participant.h"
#include "jni_misc.h"
#include "os.h"
#include "os_heap.h"
#include "os_report.h"

jni_domain
jni_domainNew(
    jni_participant p, 
    const c_char* name,
    v_domainQos qos)
{
    jni_domain domain;
    u_domain udomain;
    
    domain = NULL;
    
    if((p != NULL) && (p->uparticipant != NULL) && (name != NULL)){
        udomain = u_domainNew(p->uparticipant, name, qos);

        if(udomain != NULL){            
            domain = jni_domain(os_malloc((size_t)(C_SIZEOF(jni_domain))));
            domain->name = name;
            domain->udomain = udomain;
        }
    }
    return domain;
}

jni_result
jni_domainFree(
    jni_domain domain)
{
    jni_result r;
    
    r = JNI_RESULT_OK;
    
    if((domain == NULL) || (domain->udomain == NULL)){
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied domain is NULL"); 
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else{
        r = jni_convertResult(u_domainFree(domain->udomain));
        
        if(r == JNI_RESULT_OK){            
            os_free(domain);
        }
    }
    return r;
}
