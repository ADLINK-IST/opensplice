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

#include "v_kernel.h"
#include "u_partition.h"
#include "jni_partition.h"
#include "jni_participant.h"
#include "jni_misc.h"
#include "os.h"
#include "os_heap.h"
#include "os_report.h"

jni_partition
jni_partitionNew(
    jni_participant p, 
    const c_char* name,
    v_partitionQos qos)
{
    jni_partition _this;
    u_partition upartition;
    
    _this = NULL;
    
    if((p != NULL) && (p->uparticipant != NULL) && (name != NULL)){
        upartition = u_partitionNew(p->uparticipant, name, qos);

        if(upartition != NULL){            
            _this = jni_partition(os_malloc((size_t)(C_SIZEOF(jni_partition))));
            _this->name = name;
            _this->upartition = upartition;
        }
    }
    return _this;
}

jni_result
jni_partitionFree(
    jni_partition _this)
{
    jni_result r;
    
    r = JNI_RESULT_OK;
    
    if((_this == NULL) || (_this->upartition == NULL)){
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied partition is NULL"); 
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else{
        r = jni_convertResult(u_partitionFree(_this->upartition));
        
        if(r == JNI_RESULT_OK){            
            os_free(_this);
        }
    }
    return r;
}
