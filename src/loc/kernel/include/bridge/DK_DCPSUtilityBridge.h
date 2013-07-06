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
#ifndef DLRL_KERNEL_DCPS_UTILITY_BRIDGE_H
#define DLRL_KERNEL_DCPS_UTILITY_BRIDGE_H

#include "DLRL_Kernel.h"
/* collection includes */
#include "Coll_List.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Register a specific topic type to DCPS.
 *
 * If registration fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param home The home to which the topic being registered belongs
 * \param cache The cache to which the home is registered
 * \param topicName The name of the topic within the application scope
 * \param typeName The name of the type (fully qualified IDL name)
 */
typedef void (*DK_DCPSUtilityBridge_us_registerType)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_CacheAdmin* cache,
    LOC_char* topicName,
    LOC_char* typeName);

/* \brief Creates a DCPS topic (proxy)
 *
 * If creation fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param home The home to which the topic being created belongs
 * \param topicName The name of the topic within the application scope
 * \param typeName The name of the type (fully qualified IDL name)
 * \param topicUserData An out parameter to be filled with any required topic related user data that the language
 * binding might need. This user data will be accessible through the <code>DK_TopicInfo</code> object. May be left
 * <code>NULL</code>
 *
 * \return <code>NULL</code> if and only if an exception occured or returns the created topic
 */
/* NOT IN DESIGN - param */
typedef C_STRUCT(u_topic)* (*DK_DCPSUtilityBridge_us_createTopic)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    LOC_char* topicName,
    LOC_char* typeName,
    void** topicUserData,
    void** ls_topic,
    LOC_boolean isMainTopic);

/* \brief Creates a DCPS DataReader
 *
 * If creation fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param topicInfo The topic info which describes the topic for which a reader must be created.
 * \param ls_reader An out parameter to be set to the language specific representative of the created DCPS data reader
 *
 * \return <code>NULL</code> if and only if an exception occured or returns the created DataReader
 */
typedef C_STRUCT(u_reader)* (*DK_DCPSUtilityBridge_us_createDataReader)(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_reader);

/* \brief Creates a DCPS DataWriter
 *
 * If creation fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param topicInfo The topic info which describes the topic for which a writer must be created.
 * \param ls_writer An out parameter to be set to the language specific representative of the created DCPS DataWriter
 *
 * \return <code>NULL</code> if and only if an exception occured or returns the created DataWriter
 */
typedef C_STRUCT(u_writer)* (*DK_DCPSUtilityBridge_us_createDataWriter)(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_writer);

/* \brief Deletes a data reader within DCPS.
 *
 * If deletion fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param cache The cache to which the to-be-deleted reader belongs.
 * \param reader The to-be-deleted reader
 * \param ls_reader The language specific representative of the to-be-deleted reader
 */
typedef void (*DK_DCPSUtilityBridge_us_deleteDataReader)(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_reader reader,
    DLRL_LS_object ls_reader);

/* \brief Deletes a data writer within DCPS.
 *
 * If deletion fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param cache The cache to which the to-be-deleted writer belongs.
 * \param writer The to-be-deleted writer
 * \param ls_writer The language specific representative of the to-be-deleted writer
 */
typedef void (*DK_DCPSUtilityBridge_us_deleteDataWriter)(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_writer writer,
    DLRL_LS_object ls_writer);

/* \brief Deletes a topic within DCPS.
 *
 * If deletion fails for any reason an exception is thrown.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param cache The cache to which the to-be-deleted topic belongs.
 * \param topicInfo The kernel 'holder' of all topic information.
 */
typedef void (*DK_DCPSUtilityBridge_us_deleteTopic)(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    DK_TopicInfo* topicInfo);

/* \brief Deletes language specific topic user data which was created during topic creation.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param topicUserData The language specific topic user data.
 */
typedef void (*DK_DCPSUtilityBridge_us_releaseTopicUserData)(
    void* userData,
    void* topicUserData);

/* NOT IN DESIGN */
typedef void (*DK_DCPSUtilityBridge_us_enableEntity)(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_entity);

typedef struct DK_DCPSUtilityBridge_s{
    DK_DCPSUtilityBridge_us_registerType registerType;
    DK_DCPSUtilityBridge_us_createTopic createTopic;
    DK_DCPSUtilityBridge_us_createDataReader createDataReader;
    DK_DCPSUtilityBridge_us_createDataWriter createDataWriter;
    DK_DCPSUtilityBridge_us_deleteDataReader deleteDataReader;
    DK_DCPSUtilityBridge_us_deleteDataWriter deleteDataWriter;
    DK_DCPSUtilityBridge_us_deleteTopic deleteTopic;
    DK_DCPSUtilityBridge_us_releaseTopicUserData releaseTopicUserData;
    DK_DCPSUtilityBridge_us_enableEntity enableEntity;
} DK_DCPSUtilityBridge;

extern DK_DCPSUtilityBridge dcpsUtilityBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_DCPS_UTILITY_BRIDGE_H */
