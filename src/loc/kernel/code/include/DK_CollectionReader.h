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
#ifndef DLRL_KERNEL_COLLECTION_READER_H
#define DLRL_KERNEL_COLLECTION_READER_H

#if defined (__cplusplus)
extern "C" {
#endif

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLMultiRelation.h"

/* DLRL Kernel includes */
#include "DK_Entity.h"
#include "DK_ObjectAdmin.h"
#include "DK_Types.h"

/* \brief Responsible for correctly processing DCPS updates to collections.
 */
struct DK_CollectionReader_s
{
    /* The base class of the <code>DK_CollectionReader</code> class which manages the reference count.
     */
    DK_Entity entity;
    /* Indicates whether this <code>DK_CollectionReader</code> object is 'alive' (<code>TRUE</code>) or not
     * (<code>FALSE</code>). Objects that are no longer alive have been properly deleted and accessing operations
     * on these objects will raise <code>DLRL_ALREADY_DELETED</code> exceptions.
     */
    LOC_boolean alive;
    /* The DCPS DataReader object from which the DLRL can read the updates and then process the read samples into
     * the corresponding collections. Never <code>NULL</code>.
     */
    u_reader reader;
    /* NOT IN DESIGN */
    u_reader queryReader;
/* NOT IN DESIGNvoid* userReader; */
    /* The same DCPS DataReader object as represented by the 'reader' attribute. But then on the language specific level
     */
    DLRL_LS_object ls_reader;
    /* The <code>DK_TopicInfo</code> object registered with this <code>DK_CollectionReader</code> object. This object
     * contains all relevant topic info required by the <code>DK_CollectionReader</code> object. The
     * <code>DK_TopicInfo</code> object also contains a reference to the owning object home.
     */
    DK_TopicInfo* topicInfo;
    /* Each  <code>DK_CollectionReader</code> object is responsible for reading a specific multi relation as defined
     * in the MetaModel. The multi relation for which this  <code>DK_CollectionReader</code> object must process updates
     * is represented by a <code>DMM_DLRLMultiRelation</code> object. This object is always present, take note that
     * MetaModel objects are not reference counted. They are owned by the 'owner' home, and will become invalid when the
     * 'owning' object home is deleted (not the same as destroyed)
     */
    DMM_DLRLMultiRelation* metaRelation;
};

/* \brief Constructs a new <code>DK_CollectionReader</code> object.
 *
 * Using the release/duplicate operations defined on the base entity class will ensure that the memory is automatically
 * freed.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If creation failed due to a lack of resources.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must delete the returned <code>DK_CollectionReader</code> object using the destructor of this class (if not
 * <code>NULL</code>).</li>
 * <li>Must release the returned pointer (if not <code>NULL</code> and after its destroyed).</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param reader The DCPS DataReader used to read samples from DCPS of the topic managed by this collection reader.
 * \param ls_reader The language specific representative of the DCPS DataReader.
 * \param userLayerReader The DCPS 'user' layer representative of the DCPS DataReader
 * \param topicInfo The <code>DK_TopicInfo</code> object which contains all relevant topic info for this collection
 * reader.
 * \param relation The MetaModel object representing the multi relation for which this collection reader is used.
 *
 * \return <code>NULL</code> if and only if an exception occurred during the construction of the
 * <code>DK_CollectionReader</code> object. Otherwise returns the created <code>DK_CollectionReader</code> object.
 */
/* NOT IN DESIGN */
DK_CollectionReader*
DK_CollectionReader_new(
    DLRL_Exception* exception,
    u_reader reader,
    u_reader queryReader,
    DLRL_LS_object ls_reader,
    DK_TopicInfo* topicInfo,
    DMM_DLRLMultiRelation* relation);

/* \brief Cleans up all resources managed by this <code>DK_CollectionReader</code> object.
 *
 * Preconditions:<ul>
 * <li>Must claim a lock on the owning object. Which is the <code>DK_ObjectReader</code>. This object however is
 * protected by the update and/or admin mutex of it's corresponding home. Therefore that home must be locked. Both the
 * admin and update mutex should be locked.</li></ul>
 * <li>Must claim the administrative and update (either is fine) lock on the corresponding <code>DK_CacheAdmin</code>.
 * </li></ul>
 *
 * After this operation the <code>DK_CollectionReader</code> object in question becomes invalid and will throw a
 * <code>DLRL_ALREADY_DELETED</code> exception when trying to execute operations on it. After this operation it is
 * safe to release the pointer to the <code>DK_CollectionReader</code> object. Memory is freed automatically when the
 * reference count of the <code>DK_CollectionReader</code> object reaches zero.
 *
 * \param _this The <code>DK_CollectionReader</code> entity that is the target of this operation.
 * \param userData Optional user data.
 */
void
DK_CollectionReader_us_delete(
    DK_CollectionReader* _this,
    void* userData);

/* \brief Processes all updates waiting at the reader managed by this object.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the object in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the object in question belong.</li></ul>
 *
 * \param _this The <code>DK_CollectionReader</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param collectionIndex The index of the collection being processed in the list of all collection for the
 * type of object as managed by the owning object home. Note that this index indicates the position of the
 * meta model multi relation in the list of multi relations as managed by the meta model.
 */
void
DK_CollectionReader_us_processDCPSUpdates(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long collectionIndex);

DK_ObjectHolder*
DK_CollectionReader_us_doModificationRemoval(
    DLRL_Exception* exception,
    DK_Collection* collection,
    DMM_Basis type,
    void* targetKeysArray,
    LOC_unsigned_long targetKeysSize,
    DK_ObjectHolder* holder);

void
DK_CollectionReader_us_commitElementModification(
    DLRL_Exception* exception,
    DK_ObjectHolder* holder,
    DK_ObjectAdmin* target,
    DK_ObjectAdmin* oldTarget,
    DMM_Basis type,
    DK_Collection* collection);

/* NOT IN DESIGN */
void
DK_CollectionReader_us_enable
    (DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData);

u_reader
DK_CollectionReader_us_getReader(
    DK_CollectionReader* _this);

DK_TopicInfo*
DK_CollectionReader_us_getTopicInfo(
    DK_CollectionReader* _this);

DLRL_LS_object
DK_CollectionReader_us_getLSReader(
    DK_CollectionReader* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_COLLECTION_READER_H */
