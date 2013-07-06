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
 * This file contains all functions for analysing all
 * Splice Database Objects.
 */



#ifndef A_ANL_H
#define A_ANL_H


#include <stdio.h>
#include "a_def.h"
#include "a_lst.h"
#include "a_sts.h"
#include "c_base.h"


/**
 * \brief
 * Type Definition of a pointer to this file's (hidden) context.
 */
typedef struct a_anlContext_s *a_anlContext;



/**
 * \brief
 * Initialises this file's context and returns a new pointer to it.
 *
 * This operation reserves memory for the context, initialises this
 * context and returns a pointer to it. This operation must be
 * sucessfully performed before any other operation in this file.
 *
 * \param out
 * File pointer for 'normal' output, typically stdout
 *
 * \param err
 * File pointer for error messages, typically stderr
 *
 * \param list
 * Pointer to an a_lstList data structure. This list must already
 * have been created. If \a list is NULL, this operation will fail.
 *
 * \param showAnalyseOutput
 * Boolean value for specifying whether analyse output must be
 * displayed. If true (1), file pointer \a out (typically stdout)
 * will be used.
 *
 * \param verboseOutput
 * Boolean value specifying whether some brief information about the
 * analyse stage this module is in. If true (1), file pointer \a out
 * (typically stdout) will be used.
 *
 * \param stsContext
 * Pointer to a stsContext instance. If \a stsContext is NULL, the
 * operation will fail.
 *
 * \return
 * Pointer to the newly created context or NULL if anything failed.
 *
 * \note
 * Remember to de-initialise (\a a_anlDeInit) after use!
 *
 * \see
 * a_anlDeInit a_anlContext
 */
a_anlContext a_anlInit(
	FILE *out, FILE *err, a_lstList list, int showAnalyseOutput,
	int verboseOutput, a_stsContext stsContext);



/**
 * \brief
 * De-initialises the context.
 *
 * This operation de-initialises the context and frees up memory.
 * 
 * \param context
 * The context to de-initialise.
 *
 * \note
 * Remember to call this function before the application terminates.
 *
 * \see a_anlInit, a_anlContext
 */
void a_anlDeInit(a_anlContext context);


/********************************************************************
 
 ********************************************************************/


/**
 * \brief
 * Returns the total size of memory used by all database objects.
 *
 * This operation returns the total size of memory used by all
 * database objects.
 *
 * \param context
 * This file's context.
 *
 * \return
 * The total size of memory used by all database objects, or -1 if
 * context is NULL.
 *
 * \note
 * Be sure all database objects have been previously collected and
 * analysed.
 */
c_long a_anlTotalDataSize(a_anlContext context);



/**
 * \brief
 * Fills the (internal) list with all known database objects.
 *
 * This operation fills the (internal) list with all database
 * objects.
 *
 * \param context
 * This file's context.
 *
 * \param base
 * Database that has been successfully opened.
 *
 * \note
 * This operation uses \a c_baseObjectWalk (from c_base.h)
 * internally, which is only available in Splice's development
 * version.
 *
 * \todo
 * Get this function to return an int, to specify whether this
 * operation was successful and have calling functions check
 * this result value.
 */
void a_anlFillList(a_anlContext context, c_base base);



/**
 * \brief
 * Walks over all objects and determine and analyse its references.
 *
 * This operation walks over all database objects (again) and
 * analyses all references (pointers) it might have. These
 * references will be counted, as well the counters of the referenced
 * object.
 *
 * \param context
 * This file's context.
 *
 * \param base
 * Database that previously has been successfully opened.
 *
 * \param address
 * Shared Memory Start Address
 *
 * \param size
 * Shared Memory Size
 *
 * \note
 * Make sure the list has been filled (using \a a_anlFillList) before
 * calling this function
 *
 * \todo
 * Provide a return value
 *
 * \todo
 * Remove the need for specifying address and size, for those are
 * already known within the context!
 */
void a_anlComputeRefs(a_anlContext context, c_base base, c_address address, c_long size);



/**
 * \brief
 * Wipes (overwrites) all known database objects in Shared Memory.
 *
 * This operation wipes (overwrites) all known database objects in
 * the Shared Memory. Before calling this function, it is assumed
 * that a Shared Memory with a Splice database is currently attached
 * and all database objects are collected.\n
 * After a successful operation of this function, you might want to
 * print a hex dump of the memory.
 *
 * \param context
 * The context of this file (a_anl) that holds all information to
 * perform the wipe.
 *
 * \return
 * This operation returns true (1) if all objects were successfully
 * overwriten, false (0) if anything failed or if the (internal) list
 * of database objects is empty.
 *
 * \note
 * This is (currently) the only AAPI operation that alters any data
 * in the Shared Memory.
 *
 * \warning
 * Use with care! After this operation, no Splice operations can be
 * performed, for the database is no longer valid.
 */
int a_anlWipeObjects(a_anlContext context);


#endif

//END a_anl.h
