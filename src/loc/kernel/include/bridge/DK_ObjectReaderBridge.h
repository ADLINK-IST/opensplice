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
#ifndef DLRL_KERNEL_OBJECT_READER_BRIDGE_H
#define DLRL_KERNEL_OBJECT_READER_BRIDGE_H

#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* NOT IN DESIGN - removed */
/* DLRL_LS_object DK_ObjectReaderBridge_us_createTypedObject(DLRL_Exception* exception, void* userData,  */
/*                                                         DK_ObjectHomeAdmin* home, DK_ReadData* data); */

/* \brief Responsible for updating a language specific representative with a new topic sample.
 *
 * This operation should simply update the language specific object with the new topic data. Relations are processed
 * later and shouldn't be dealt within this operation, its simply meant to update topic content within the DLRL object.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin and update mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional language specific user data
 * \param home The home that is the manager of the type of object that needs to be created.
 * \param data Data regarding the object being processed
 */
 /* NOT IN DESIGN and doc - params changed */
typedef void (*DK_ObjectReaderBridge_us_updateObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* object,
    DLRL_LS_object ls_topic);

/* \brief Allows for some specific read preprocessing.
 *
 * This function is intended to fill the 'dstInfo' variable of the 'DK_ReadInfo' struct. Once
 * this is accomplished one MUST call the <code>DK_ObjectReader_us_doRead(...)</code> operation to continue the read
 * action, this is a requirement. This allows for language bindings to allocate the information placed in the
 * 'dstInfo' variable on stack instead of on heap and should thus be seen as a performance enhancement.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin and update mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param readInfo Holder for several fields to be used within the operation.
 * \param objReader The object reader for which some preprocessing is needed.
 */
typedef void (*DK_ObjectReaderBridge_us_doLSReadPreProcessing)(
    DK_ReadInfo* readInfo,
    DK_ObjectReader* objReader);

/* \brief Introduces object to collection navigation on language specific objects level. So that querying the kernel for
 * a specific collection becomes unneccesary.
 *
 * Some languages might not want to pay any overhead which exists for accessing the kernel each time a collection is
 * needed and want to store the data on the language binding level. This operation provides the means to easily
 * accomplish this.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin and update mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param home The home that is the manager of the <code>DK_ObjectAdmin</code> object in question.
 * \param objectAdmin The object admin which is the owner of the collection.
 * \param collection The collection object.
 * \param collectionIndex The index of the collection as it was created within the meta model
 */
typedef void (*DK_ObjectReaderBridge_us_setCollectionToLSObject)(
     DLRL_Exception* exception,
     void* userData,
     DK_ObjectHomeAdmin* home,
     DK_ObjectAdmin* objectAdmin,
     DK_Collection* collection,
     LOC_unsigned_long collectionIndex);

/* NOT IN DESIGN -- removed */
/* DLRL_LS_object DK_ObjectReaderBridge_us_createOIDStructure(DLRL_Exception* exception, void* userData, void* oidData); */

/* NOT IN DESIGN -- removed */
/* /void DK_ObjectReaderBridge_us_deleteOIDStructure(void* userData, DLRL_LS_object oidStructure); */

typedef DLRL_LS_object (*DK_ObjectReaderBridge_us_createLSTopic)(
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin,
    void* dstInfo,
    void (*ls_copyOut)(void*, void*),
    void* sampleData);

/* \brief Resets the modification information when the end of an update round has been reached. 
 * This means the is_xxx_modified operations will no longer return true for attributes that were
 * modified in the preceeding update round.
 *
 * \param objectAdmin The object fopr which the modification info needs to be reset.
 */
typedef void (*DK_ObjectReaderBridge_us_resetLSModificationInfo)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

typedef struct DK_ObjectReaderBridge_s{
    DK_ObjectReaderBridge_us_updateObject updateObject;
    DK_ObjectReaderBridge_us_doLSReadPreProcessing doLSReadPreProcessing;
    DK_ObjectReaderBridge_us_setCollectionToLSObject setCollectionToLSObject;
    DK_ObjectReaderBridge_us_createLSTopic createLSTopic;
    DK_ObjectReaderBridge_us_resetLSModificationInfo resetLSModificationInfo;
} DK_ObjectReaderBridge;

extern DK_ObjectReaderBridge objectReaderBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_READER_BRIDGE_H */
