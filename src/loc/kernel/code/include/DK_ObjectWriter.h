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
#ifndef DLRL_KERNEL_OBJECT_WRITER_H
#define DLRL_KERNEL_OBJECT_WRITER_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL kernel includes */
#include "DK_Entity.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct DK_WriterData_s
{
    Coll_List* keyFields;
    DK_ObjectAdmin* objectAdmin;
    DMM_DCPSTopic* mainTopic;
    DLRL_Exception* exception;
    u_writer writer;
}DK_WriterData;

struct DK_ObjectWriter_s
{
    DK_Entity entity;
    LOC_boolean alive;
    u_writer writer;
    DLRL_LS_object ls_writer;
    DK_TopicInfo* topicInfo;
    Coll_List collectionWriters;/* NOT IN DESIGN */
};

DK_ObjectWriter*
DK_ObjectWriter_new(
    DLRL_Exception* exception,
    u_writer writer,
    DLRL_LS_object ls_writer,
    DK_TopicInfo* topicInfo);

void
DK_ObjectWriter_us_delete(
    DK_ObjectWriter* _this,
    void* userData);

/* NOT IN DESIGN, name changed, param added */
void
DK_ObjectWriter_us_enable(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData);

void
DK_ObjectWriter_us_createCollectionWriter(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo);

/* NOT IN DESIGN */
u_instanceHandle
DK_ObjectWriter_us_registerInstance(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin,
    DK_WriterData* writerData);

/* NOT IN DESIGN? */
LOC_boolean
DK_ObjectWriter_us_isAlive(
    DK_ObjectWriter* _this);

/* NOT IN DESIGN */
void
DK_ObjectWriter_us_write(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object);

/* NOT IN DESIGN */
void
DK_ObjectWriter_us_dispose(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object);

Coll_List*
DK_ObjectWriter_us_getCollectionWriters(
    DK_ObjectWriter* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_WRITER_H */
