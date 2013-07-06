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

#include "jni_nameService.h"
#include "jni_misc.h"
#include "c_typebase.h"
#include "os_stdlib.h"
#include "os_heap.h"

static jni_nameService ns = NULL;
static c_iter mappings = NULL;

C_CLASS(jni_mapping);

C_STRUCT(jni_mapping){
    c_long domainId;
    c_char* uri;
};

#define jni_mapping(a) ((jni_mapping)(a))

jni_nameService
jni_nameServiceNew()
{
    u_result r;
    
    if(ns == NULL){
        r = u_userInitialise();
        
        if(r == U_RESULT_OK){
            ns = jni_nameService(os_malloc((size_t)(C_SIZEOF(jni_nameService))));
            mappings = c_iterNew(NULL);
            ns->refCount = 1;
        }
    } else{
        ns->refCount++;
    }
    return ns;
}

jni_result
jni_nameServiceFree()
{
    u_result r;
    jni_mapping mapping;
    r = U_RESULT_OK;
    
    if(ns != NULL){
        ns->refCount--;
        
        if(ns->refCount == 0){
           mapping = jni_mapping(c_iterTakeFirst(mappings));

            while(mapping != NULL){
                os_free(mapping->uri);
                os_free(mapping);
                mapping = jni_mapping(c_iterTakeFirst(mappings));        
            }
            c_iterFree(mappings);
            os_free(ns);
            ns = NULL;
            r = u_userDetach();
        } else{
            r = U_RESULT_OK;
        }
    } else{
      r = U_RESULT_NOT_INITIALISED;
    }
    return jni_convertResult(r);
}

c_bool
jni_nameServiceAddDomain(
    const c_char* uri)
{
    c_bool result;
    const c_char* uri2;
    jni_mapping mapping;
    c_long domainId;
    c_iter copy;
    
    result = FALSE;
    uri2 = NULL;
    
    if(mappings != NULL){
        copy = c_iterCopy(mappings);
        mapping = jni_mapping(c_iterTakeFirst(copy));

        while((mapping != NULL) && (uri2 == NULL)){
            if(strcmp(mapping->uri, uri) == 0){
                uri2 = uri;
            }
            mapping = jni_mapping(c_iterTakeFirst(copy));
        }
        c_iterFree(copy);
        
        if(uri2 == NULL){
            mapping = jni_mapping(os_malloc(C_SIZEOF(jni_mapping)));
            domainId = c_iterLength(mappings);
            mapping->domainId = domainId;
            mapping->uri = (c_char*)(os_malloc(strlen(uri) + 1));
            os_strcpy(mapping->uri, uri);
            c_iterInsert(mappings, mapping);
        } else {
           /*URI already exists, do nothing.*/
        }
        result = TRUE;
    }
    return result;
}

const c_char*
jni_nameServiceResolveURI(
    c_long domainId)
{
    const c_char* result;
    c_iter copy;
    jni_mapping mapping;
    c_bool found;
    result = NULL;
    
    if(mappings != NULL){
        copy = c_iterCopy(mappings);
        found = FALSE;
        mapping = jni_mapping(c_iterTakeFirst(copy));

        while((mapping != NULL) && (found == FALSE)){
            if(mapping->domainId == domainId){
                result = mapping->uri;
                found = TRUE;
            }
            mapping = jni_mapping(c_iterTakeFirst(copy));
        }
        c_iterFree(copy);
    }
    /* if not found or mapping == NULL then result == NULL */
    return result;
}
