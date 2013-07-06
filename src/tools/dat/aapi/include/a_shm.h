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
 * detaching Shared Memory in order to attach to an existing
 * SPLICE-DDS Shared Memory Segment and creating a new shm segment
 * that can be considered a "private segment" (although to the OS it
 * still is shared memory).
 */


#ifndef A_SHM_H
#define A_SHM_H

#include <stdio.h>
#include "a_def.h"


/**
 * \brief
 * This file's context
 *
 * This file's context, with a hidden data structure. This context is
 * needed with every operation in this file. Be sure to create it
 * first (with a_shmInit) and to destroy it afterwards (with
 * a_shmDeInit).
 *
 * \see
 * a_shmInit a_shmDeInit
 */
typedef struct a_shmContext_s *a_shmContext;


/**
 * \brief
 * Initialises the context and returns a new pointer to this context.
 *
 * This operation creates (initialises) a new context. It must be
 * called before any other operation in this file. Remember to
 * de-initialise (DeInit) after use!
 *
 * \param out
 * File Pointer to Standard Output, typically stdout
 *
 * \param err
 * File Pointer to Error Output, typically stderr
 *
 * \param shm_name
 * Name of the Shared Memory Segment, in SPLICE terms. Also known as
 * the Domain Name.
 *
 * \return
 * Pointer to the newly created context, or NULL if anything fails.
 *
 * \see
 * a_shmContext a_shmDeInit
 */
a_shmContext a_shmInit(FILE *out, FILE *err, char *shm_name);


/**
 * \brief
 * De-initialises the context
 *
 * This operation de-initialises the context and frees up memory. The
 * context must have been previously created with a_shmInit.
 *
 * \param context
 * This file's context to de-initialise
 *
 * \see
 * a_shmContext a_shmInit
 */
void a_shmDeInit(a_shmContext context);


/**
 * \brief
 * Returns a pointer to the shm name
 *
 * This operation returns a pointer to the Shared Memory Name (aka
 * Domain Name) held by the context.
 *
 * \return
 * Pointer to the Shared Memory name, held by the context.
 *
 * \note
 * The returned pointer points directly into the context. If the
 * context is destroyed after this operation was performed, the
 * variable holding this return value will lose its validity.
 */
char *a_shmGetShmName(a_shmContext context);


/**
 * \brief
 * Returns the Start Address of the Shared Memory Segment, held by
 * the context
 *
 * This operation returns the Start Address of the Shared Memory
 * Segment, held by the context.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \return
 * Start Address of the Shared Memory Segment, or 0 if anything
 * failed
 */
c_address a_shmGetShmAddress(a_shmContext context);


/**
 * \brief
 * Returns whether the Shared Memory is currently (successfully)
 * attached (through this context).
 *
 * This operation returns a boolean value (0 or 1) specifying
 * whether the Shared Memory was attached by or through this
 * file's context.
 *
 * \param context
 * This file's context
 *
 * \return
 * Boolean value specifying whether the Shared Memory is currently
 * attached.
 *
 * \note
 * This operation only specifies whether the Shared Memory was
 * attached through this file's context. It does not check for any
 * other attatchments in terms of OS-related Shared Memory
 * Attachments.
 */
int a_shmIsAttached(a_shmContext context);


/**
 * \brief
 * Sets a (new) Shared Memory Name to attach to into the context.
 *
 * This operation sets a (new) Shared Memory Name (aka Domain
 * Name) into the context, overriding the name that was specified
 * at context creation.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \param newShmName
 * Pointer to the new Shared Memory Name. Internally, the string will
 * be copied into a new one.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \note
 * If the Shared Memory already has been attached to, the operation
 * will fail.
 */
int a_shmSetShmName(a_shmContext context, char *newShmName);


/**
 * \brief
 * Appends a string to the Shared Memory Name
 *
 * This operation appends a string to the Shared Memory Name (aka
 * Domain Name), held by the context.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \param appName
 * Pointer to the string to append the Shared Memory Name to.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \note
 * If the Shared Memory already has been attached to, the operation
 * will fail.
 */
int a_shmAppShmName(a_shmContext context, char *appName);


/**
 * \brief
 * Attaches to Shared Memory
 *
 * This operation attaches to Shared Memory, using the Shared Memory
 * Name that was specified at context creation, possibly appended
 * with a suffix string. The operation will fail if context is NULL,
 * if the Shared Memory name has not been set (is NULL) or if the
 * Shared Memory alrady has been attached through this context.\n
 * Internally, this operation uses os_sharedMemoryAttach from the
 * SPLICE OS-Layer.
 *
 * \param context
 * This file's context
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \note
 * This operation will (also) fail if os_sharedMemoryAttach fails,
 * which could happen if a shared memory segment with specified
 * name could not have been found or if you're not permitted to
 * attach to the desired shared memory. Unfortunately, at this point
 * these cases can not be determined.
 *
 * \see
 * a_shmDetach
 */
int a_shmAttach(a_shmContext context);


/**
 * \brief
 * Detaches from Shared Memory
 *
 * This operation detaches from Shared Memory, that was previously
 * attached to by a_shmAttach. The operation will fail if context
 * is NULL or if the Shared Memory was not attached.\n
 * Internally, this operation uses os_sharedMemoryDetach from the
 * SPLICE OS-Layer.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_shmAttach
 */
int a_shmDetach(a_shmContext context);


/**
 * \brief
 * Creates a Shared Memory Segment
 *
 * This operation creates a Shared Memory Segment, intentionally for
 * private use. The operation will fail if context is NULL, or if the
 * Shared Memory Name is not set (is NULL).\n
 * Internally, os_sharedCreateHandle, os_sharedCreateAttr and
 * os_sharedMemoryCreate from SPLICE's OS-Layer are used.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail
 *
 * \param map_address
 * Shared Memory's Start Address
 *
 * \param size
 * Size of the Shared Memory Segment
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \note
 * This operation will (also) fail if one of the os_* operations
 * fails. In that case, an error message is sent to the context's
 * \a err file pointer (typically stderr).
 *
 * \note
 * This operation will only \b create a Shared Memory segment. After
 * a successful creation, a seperate call to a_shmAttach is needed
 * before the Shared Memory segment can be used.
 *
 * \see
 * a_shmDestroyShm a_shmAttach a_shmDetach
 */
int a_shmCreateShm(a_shmContext context, c_address map_address, c_long size);


/**
 * \brief
 * Destroys a Shared Memory Segment
 *
 * This operation destroys a Shared Memory Segment that was created
 * by a_shmCreateShm. The operation will fail if context is NULL,
 * if there is no Shared Memory Segment to destroy or if this context
 * is still attached to the Shared Memory.\n
 * Internally, os_sharedMemoryDestoy from SPLICE's OS-Layer is used.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_shmCreateShm a_shmDetach
 */
int a_shmDestroyShm(a_shmContext context);


#endif  /* A_SHM_H */

//END a_shm.h
