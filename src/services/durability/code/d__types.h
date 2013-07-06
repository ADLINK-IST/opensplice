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

#ifndef D__TYPES_H
#define D__TYPES_H

#include "c_typebase.h"
#include "kernelModule.h"
#include "durabilityModule2.h"
#include "d_object.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum d_storeType{
    D_STORE_TYPE_UNKNOWN, D_STORE_TYPE_XML, D_STORE_TYPE_BIG_ENDIAN, D_STORE_TYPE_MEM_MAPPED_FILE, D_STORE_TYPE_KV
} d_storeType;

typedef enum d_storeResult{
    D_STORE_RESULT_ERROR, D_STORE_RESULT_OK,
    D_STORE_RESULT_ILL_PARAM, D_STORE_RESULT_UNSUPPORTED,
    D_STORE_RESULT_PRECONDITION_NOT_MET, D_STORE_RESULT_IO_ERROR,
    D_STORE_RESULT_MUTILATED, D_STORE_RESULT_DROPPED,
    D_STORE_RESULT_REJECTED_BY_INSTANCE, D_STORE_RESULT_REJECTED_BY_SAMPLE,
    D_STORE_RESULT_REJECTED_BY_SAMPLE_PER_INSTANCE,
    D_STORE_RESULT_METADATA_MISMATCH, D_STORE_RESULT_OUT_OF_RESOURCES
} d_storeResult;

typedef enum d_level {
    D_LEVEL_FINEST, D_LEVEL_FINER, D_LEVEL_FINE,
    D_LEVEL_CONFIG, D_LEVEL_INFO,
    D_LEVEL_WARNING, D_LEVEL_SEVERE, D_LEVEL_NONE
} d_level;

C_CLASS(d_durability);
C_CLASS(d_configuration);
C_CLASS(d_nameSpace);
C_CLASS(d_policy);
C_CLASS(d_element);
C_CLASS(d_table);
C_CLASS(d_group);
C_CLASS(d_fellow);
C_CLASS(d_admin);
C_CLASS(d_publisher);
C_CLASS(d_subscriber);
C_CLASS(d_waitset);
C_CLASS(d_waitsetEntity);
C_CLASS(d_listener);
C_CLASS(d_readerListener);
C_CLASS(d_statusListener);
C_CLASS(d_groupLocalListener);
C_CLASS(d_groupRemoteListener);
C_CLASS(d_statusRequestListener);
C_CLASS(d_groupsRequestListener);
C_CLASS(d_sampleRequestListener);
C_CLASS(d_sampleChainListener);
C_CLASS(d_nameSpacesRequestListener);
C_CLASS(d_nameSpacesListener);
C_CLASS(d_persistentDataListener);
C_CLASS(d_deleteDataListener);
C_CLASS(d_eventListener);
C_CLASS(d_groupList);
C_CLASS(d_store);
C_CLASS(d_storeXML);
C_CLASS(d_storeMMF);
C_CLASS(d_storeKV);
C_CLASS(d_action);
C_CLASS(d_actionQueue);
C_CLASS(d_groupCreationQueue);
C_CLASS(d_chain);
C_CLASS(d_chainBead);
C_CLASS(d_chainLink);
C_CLASS(d_readerRequest);
C_CLASS(d_mergeAction);

/**
 * These definitions make "classes" of the odl-structs.
 * This completes the 'struct'-definitions in 'durability.odl'
 */
C_CLASS(d_message);
C_CLASS(d_statusRequest);
C_CLASS(d_groupsRequest);
C_CLASS(d_sampleRequest);
C_CLASS(d_status);
C_CLASS(d_newGroup);
C_CLASS(d_sampleChain);
C_CLASS(d_networkAddress);
C_CLASS(d_nameSpaces);
C_CLASS(d_nameSpacesRequest);
C_CLASS(d_deleteData);
C_CLASS(d_mergeState);

#define D_ARG_NAME                   "-name"
#define D_ARG_STRLEN_NAME            ((size_t)(5))
#define D_ARG_SERVICENAME            "-servicename"
#define D_ARG_STRLEN_SERVICENAME     ((size_t)(12))
#define D_ARG_URI_START              "file://"
#define D_ARG_STRLEN_URI_START       ((size_t)(7))
#define D_SERVICE_NAME               "DurabilityService"
#define D_STRING_LENGTH_LIMIT        (1024)
#define D_CONTEXT                    "DurabilityService"
#define D_CONFIDENCE                 ((c_ulong)0x4E614D65u) /* 'NaMe' */
#define D_CONFIDENCE_NULL            ((c_ulong)0x000000000)

#define D_MAX_STRLEN_NAMESPACE       (101)
#define D_MODULE_NAME                "durabilityModule2"

#if defined (__cplusplus)
}
#endif

#endif /* D__TYPES_H */
