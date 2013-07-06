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
#ifndef DLRL_KERNEL_OBJECT_READER_H
#define DLRL_KERNEL_OBJECT_READER_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_List.h"

/* DLRL kernel includes */
#include "DLRL_Kernel_private.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_ObjectReader_s
{
    DK_Entity entity;
    LOC_boolean alive;
    u_reader reader;
    u_reader queryReader;/* NOT IN DESIGN */
    DLRL_LS_object ls_reader;
    /* NOT IN DESIGNvoid* userReader; */
    DK_TopicInfo* topicInfo;
    Coll_List collectionReaders;
    Coll_List newSamples;
    Coll_List modifiedSamples;
    Coll_List deletedSamples;
    Coll_Set unresolvedElements;
};

/* ls_reader may be null */
/* NOT IN DESIGN */
DK_ObjectReader*
DK_ObjectReader_new(
    DLRL_Exception* exception,
    u_reader reader,
    u_reader queryReader,
    DLRL_LS_object ls_reader,
    DK_TopicInfo* topicInfo);

void
DK_ObjectReader_us_delete(
    DK_ObjectReader* _this,
    void* userData);

/* NOT IN DESIGNvoid* DK_ObjectReader_us_getUserReader(DK_ObjectReader* _this); */

u_reader
DK_ObjectReader_us_getReader(
    DK_ObjectReader* _this);

Coll_List*
DK_ObjectReader_getNewObjects(
    DK_ObjectReader* _this);

Coll_List*
DK_ObjectReader_getModifiedObjects(
    DK_ObjectReader* _this);

Coll_List*
DK_ObjectReader_getDeletedObjects(
    DK_ObjectReader* _this);

/* NOT IN DESIGN - name changed, param added */
void
DK_ObjectReader_us_enable(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData);

void
DK_ObjectReader_us_createCollectionReader(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    DMM_DLRLMultiRelation* relation);

/* requires update & admin locks on owner home and related homes! */
void
DK_ObjectReader_us_resetObjectModificationInformation(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData);

Coll_List*
DK_ObjectReader_us_getCollectionReaders(
    DK_ObjectReader* _this);

void
DK_ObjectReader_us_markObjectAsModified(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* modifiedObject);

/* array holder internal object array may be null after this operation!. if so the size will be indicated as 0 */
void
DK_ObjectReader_us_getAllObjects(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ObjectArrayHolder* arrayHelper);

/* used by coll reader as well */
/* NOT IN DESIGN */
DK_ReadAction
DK_ObjectReader_us_determineSampleAction(
    LOC_boolean entityExists,
    v_state sampleState,
    v_state instanceState,
    LOC_boolean noWritersCountChanged,
    LOC_boolean disposedCountChanged);

void
DK_ObjectReader_us_processCollectionUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
void
DK_ObjectReader_us_processRelationUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples);

void
DK_ObjectReader_us_collectObjectUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadInfo* info);

/* NOT IN DESIGN */
void
DK_ObjectReader_us_processObjectUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples);

/* NOT IN DESIGN */
void
DK_ObjectReader_us_clearAllRelationsToDeletedObjects(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples);


#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_READER_H */
