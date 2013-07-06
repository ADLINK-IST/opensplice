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
#ifndef DLRL_KERNEL_TYPES_H
#define DLRL_KERNEL_TYPES_H

/* kernel includes */
#include "u_instanceHandle.h"

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL kernel includes */
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_ReadData_s;
typedef struct DK_ReadData_s DK_ReadData;

struct DK_EventDispatcher_s;
typedef struct DK_EventDispatcher_s DK_EventDispatcher;

struct DK_CollectionReader_s;
typedef struct DK_CollectionReader_s DK_CollectionReader;

struct DK_CollectionWriter_s;
typedef struct DK_CollectionWriter_s DK_CollectionWriter;

struct DK_Contract_s;
typedef struct DK_Contract_s DK_Contract;

struct DK_CacheAccessTypeRegistry_s;
typedef struct DK_CacheAccessTypeRegistry_s DK_CacheAccessTypeRegistry;

/*NOT IN DESIGN*/
typedef enum DK_ReadAction_e
{
    DK_READ_ACTION_DO_NOTHING, /* Indicates nothing has to be done */
    DK_READ_ACTION_CREATE, /* Indicates an instance should be created. */
    DK_READ_ACTION_CREATE_DELETE, /* Indicates an instance should be created as a deleted instance */
    DK_READ_ACTION_MODIFY, /* Indicates an instance has to be modified */
    DK_READ_ACTION_MODIFY_AND_DELETE, /* Indicates an instance has to be modified, then deleted */
    DK_READ_ACTION_NO_WRITERS_CHANGE, /* No writers state has changed */
    DK_READ_ACTION_DELETE, /* Indicates an instance should be deleted */
    DK_READ_ACTION_GENERATION, /* Indicates a new generation of the instance should be created */
    DK_READ_ACTION_GENERATION_DELETE, /* Indicates a new generation of the instance should be created as a deleted instance */
    DK_ReadAction_elements /* Sentinel */
} DK_ReadAction;

struct DK_ReadData_s
{
    DK_ObjectAdmin* objectAdmin;
    DK_ObjectAdmin* previousGeneration;
    DK_ReadAction action;
    u_instanceHandle handle;
    DLRL_LS_object ls_topic;
    c_value* keyValueArray;
    c_value* foreignKeyValueArray;
    LOC_long noWritersCount;
    LOC_long disposedCount;
};


/* inheritance table */

typedef enum DK_Class
{
    DK_CLASS_CACHE_ACCESS_ADMIN,
    DK_CLASS_CACHE_ADMIN,
    DK_CLASS_OBJECT_ADMIN,
    DK_CLASS_OBJECT_HOLDER,
    DK_CLASS_OBJECT_HOME_ADMIN,
    DK_CLASS_OBJECT_READER,
    DK_CLASS_OBJECT_WRITER,
    DK_CLASS_TOPIC_INFO,
    DK_CLASS_COLLECTION_READER,
    DK_CLASS_COLLECTION_WRITER,
    DK_CLASS_SET_ADMIN,
    DK_CLASS_MAP_ADMIN,
    DK_CLASS_SELECTION_ADMIN,
    DK_CLASS_EVENT_DISPATCHER,
    DK_Class_elements
}DK_Class;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_TYPES_H */
