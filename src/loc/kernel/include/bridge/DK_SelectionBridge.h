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
#ifndef DLRL_KERNEL_SELECTION_BRIDGE_H
#define DLRL_KERNEL_SELECTION_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Checks if each object provided in the object array matches the criteria of the filter that belongs to the
 * specified selection and returns all matched objects as an array. The kernel will free the allocated memory of the
 * array. The <code>DK_ObjectAdmin</code> objects within the array MUST NOT be reference increased! Doing so will result
 * in a memory leak! So take good note of this!
 *
 * Mutex claims during this operation:<ul>
 * <li>The update and admin mutexes of the OWNING <code>DK_ObjectHomeAdmin</code> object to which the selection in
 * question belong.</li></ul>
 *
 * One should realize it can be quite cumbersome to have the admin mutex of the owning object home locked. As its
 * easy to imagine that during the filter check operation some operations are called that want to claim the admin
 * mutex as well. And the admin mutex protects alot. So to avoid problems here it is recommended to release the admin
 * mutex just before entering the language specific algorithm for checking and re-claiming it just after the check
 * algorithm finished its work. This will prevent a dead lock in those cases where the check operation of the filter
 * access admin mutex protected operations. And it will also remain thread safe, as the update mutex of the home remains
 * locked, ensuring nothing can be deleted and intervere with the entire algorithm. The locking strategy used by the
 * DLRL ensures this is an acceptable solution to this problem.
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param selection The selection for which the objects need to be checked.
 * \param filterCriterion The language specific filter which was provided upon creation of the selection and
 * which contains the checking algortihm.
 * \param objectArray The object array containing <code>DK_ObjectAdmin</code> objects, each object in this array needs
 * to be checked with the checking algorithm defined by the provided filter. (may NOT be <code>NULL</code>)
 * \param size The number of elements contained with the objectArray.
 * \param passedAdminsArraySize An out parameter which should be set to the size of the returned
 * <code>DK_ObjectAdmin</code> array.
 *
 * \return An array containing all objects that match the criteria of the filter or <code>NULL</code> if an exception
 * occured or if the out parameter 'passedAdminsArraySize' is set to zero. The kernel will free the allocated memory of
 * the array. The <code>DK_ObjectAdmin</code> objects within the array MUST NOT be reference increased! Doing so will
 * result in a memory leak! So take good note of this!
 */
typedef DK_ObjectAdmin** (*DK_SelectionBridge_us_checkObjects)(
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DLRL_LS_object filterCriterion,
    DK_ObjectAdmin** objectArray,
    LOC_unsigned_long size,
    LOC_unsigned_long* passedAdminsArraySize);


typedef void (*DK_SelectionBridge_us_triggerListenerInsertedObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);

typedef void (*DK_SelectionBridge_us_triggerListenerModifiedObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);

typedef void (*DK_SelectionBridge_us_triggerListenerRemovedObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);


typedef struct DK_SelectionBridge_s{
     DK_SelectionBridge_us_checkObjects checkObjects;
     DK_SelectionBridge_us_triggerListenerInsertedObject triggerListenerInsertedObject;
     DK_SelectionBridge_us_triggerListenerModifiedObject triggerListenerModifiedObject;
     DK_SelectionBridge_us_triggerListenerRemovedObject triggerListenerRemovedObject;
} DK_SelectionBridge;

extern DK_SelectionBridge selectionBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_SELECTION_BRIDGE_H */
