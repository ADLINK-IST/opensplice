/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/**
 * This file contains the operations for reading the
 * /<temporary directory>/spddskey_* files. For now, this is the only
 * way to retrieve information about the currently attached shared
 * memory, like its name and size.\n
 * \a key \a files that ca not be read (no access) are considered
 * not interesting for AAPI purposes and will be skipped.\n
 *
 * \note
 * This is a workaround and is only tested on Sun Solaris and should
 * probably work on Linux as well.
 *
 * \todo
 * Accessing the SPLICE OS-Layer to retrieve this information
 * directly, thus without the use of operations in this file.
 */


#ifndef A_KEY_H
#define A_KEY_H

#include "a_def.h"


/**
 * \brief
 * Type Definition for the (hidden) context
 *
 * \see
 * a_keyInit a_keyDeInit
 */
typedef struct a_keyContext_s *a_keyContext;


/**
 * \brief
 * Initialises the context
 *
 * This operation initialises the context, reads all  available
 * \a key \a files and stores the retrieved information internally.
 * Several \a Get \a Commands can then be issued to retrieve the
 * information.\n
 *
 * \note
 * Remember to de-initialise the context after use!
 *
 * \param dir
 * Directory where the key files reside. Typically "/tmp".
 *
 * \param mask
 * Start of file name of a \a key \a file. Do not use wild cards
 * here! Typical value: "spddskey_".
 *
 * \return
 * Pointer to the context, or NULL if anything failed.
 *
 * \see
 * a_keyContext a_keyDeInit
 */
a_keyContext a_keyInit(const char *dir, const char *mask);


/**
 * \brief
 * Deinitialises the context and frees up memory.
 *
 * This operation de-initialises this file's context.
 * Remember to call this function before program termination.
 *
 * \param context
 * This file's context
 *
 * \see
 * a_keyContext a_keyInit
 */
void a_keyDeInit(a_keyContext context);


/**
 * \brief
 * Searches for a key file with the specified Shared Memory Name and
 * returns the Start Address the key file holds
 *
 * This operation searches for a key file with the specified
 * Shared Memory Name (Domain Name) and returns its memory
 * start address. Internally, it searches within an internal
 * data structure. The collection of the information from these
 * files is done at context creation.
 *
 * \param context
 * This file's context
 *
 * \param shm_name
 * The Shared Memory Name to search for
 *
 * \return
 * Memory Start Address of the specified Shared Memory Name, or
 * 0 if not found.
 *
 * \see
 * a_keyInit
 */
c_address a_keyGetStartAddress(a_keyContext context, char *shm_name);


/**
 * \brief
 * Searches for a key file with the specified Shared Memory Name and
 * returns the Memory Size the key file holds
 *
 * This operation searches for a key file with the specified
 * Shared Memory Name (Domain Name) and returns its memory size.
 * Internally, it searches within an internal data structure. The
 * collection of the information from these files is done at context
 * creation.
 *
 * \param context
 * This file's context
 *
 * \param shm_name
 * The Shared Memory Name to search for
 *
 * \return
 * Memory Size of the specified Shared Memory Name, or 0 if the key
 * file was not found.
 *
 * \see
 * a_keyInit
 */
c_long a_keyGetSize(a_keyContext context, char *shm_name);


/**
 * \brief
 * Searches for a key file with the specified Shared Memory Name and
 * returns the Version Text the key file holds
 *
 * This operation searches for a key file with the specified
 * Shared Memory Name (Domain Name) and returns its version string.
 * Internally, it searches within an internal data structure. The
 * collection of the information from these files is done at context
 * creation.
 *
 * \param context
 * This file's context
 *
 * \param shm_name
 * The Shared Memory Name to search for
 *
 * \return
 * Version String of the specified Shared Memory Name, or NULL if the
 * key file was not found.
 *
 * \note
 * A pointer to an internal data structure is returned. Do not alter
 * its value or free this pointer yourself.
 *
 * \see
 * a_keyInit
 */
char *a_keyGetVersion(a_keyContext context, char *shm_name);


/**
 * \brief
 * Searches for a key file with the specified Shared Memory Name and
 * returns the Process ID the key file holds
 *
 * This operation searches for a key file with the specified
 * Shared Memory Name (Domain Name) and returns its Process ID.
 * Internally, it searches within an internal data structure. The
 * collection of the information from these files is done at context
 * creation.
 *
 * \param context
 * This file's context
 *
 * \param shm_name
 * The Shared Memory Name to search for
 *
 * \return
 * Process ID of the specified Shared Memory Name, or 0 if the key
 * file was not found.
 *
 * \see
 * a_keyInit
 */
int a_keyGetPid(a_keyContext context, char *shm_name);


#endif   /* A_KEY_H */

//END a_key.h
