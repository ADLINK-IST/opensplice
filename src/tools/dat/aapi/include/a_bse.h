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
 * This file contains the functions for opening and closing
 * the database.
 */


#ifndef A_BSE_H
#define A_BSE_H

#include "c_base.h"
#include "a_def.h"


/**
 * \brief
 * Context for a_bse
 *
 * Hidden data structure for holding this file's context
 */
typedef struct a_bseContext_s *a_bseContext;


/**
 * \brief
 * State Kind for retrieving a base's property:
 */
typedef enum a_bseStateKind {
	A_BSE_STATE_UNDEFINED,    ///< Undefined a_bseStateKind value, used internally
	A_BSE_STATE_SIZE,         ///< c_mmStatus property \a size
	A_BSE_STATE_USED,         ///< c_mmStatus property \a used
	A_BSE_STATE_MAXUSED,      ///< c_mmStatus property \a maxUsed
	A_BSE_STATE_FAILS,        ///< c_mmStatus property \a fails
	A_BSE_STATE_GARBAGE,      ///< c_mmStatus property \a garbage
	A_BSE_STATE_COUNT         ///< Number of a_bseStatekind values, used internally
} a_bseStateKind;


/**
 * \brief
 * Initialises the context
 *
 * This operation creates and returns a new context
 *
 * \param db_name
 * The name of the database to use
 *
 * \see
 * a_bseContext a_bseDeInit
 */
a_bseContext a_bseInit(char *db_name);


/**
 * \brief
 * De-initialises the context
 *
 * This operation de-initialises the context and frees up memory
 *
 * \param context
 * The context to de-initialise
 *
 * \return
 * Boolean value specifying whether the operation was successful
 *
 * \see
 * a_bseContext a_bseInit
 */
int a_bseDeInit(a_bseContext context);


/**
 * \brief
 * Checks if the database was successfully opened
 *
 * This operation checks if the database was successfully opened
 *
 * \param context
 * This file's context
 *
 * \return
 * Boolean value specifying whether the database was successfully
 * opened
 *
 * \see
 * a_bseContext
 */
int a_bseIsOpened(a_bseContext context);


/**
 * \brief
 * Returns a pointer to the database name
 *
 * This operation returns a pointer to the database name
 *
 * \param context
 * This file's context
 *
 * \return
 * Pointer to the database name, or NULL if context is NULL.
 *
 * \note
 * Be careful with the returned pointer as it references directly to
 * the char pointer in the context holding the database name, i.e.
 * there will be no string duplicated.
 *
 * \see
 * a_bseContext
 */
char *a_bseGetBaseName(a_bseContext context);


/**
 * \brief
 * Returns the start address of the database
 *
 * This operation returns the value of the database's memory start
 * address.
 *
 * \param context
 * This file's context
 *
 * \return
 * Value of the database's memory start address, or 0 is context is
 * NULL or the database is not opened.
 *
 * \see
 * a_bseContext
 */
c_address a_bseGetBaseAddr(a_bseContext context);


/**
 * \brief
 * Returns a property value of the currently opened database
 *
 * This operation returns a property value (that of a
 * c_mmStatus member) of the currently opened database.
 *
 * \param context
 * This file's context
 *
 * \param stateKind
 * Kind of property that must be retrieved
 *
 * \return
 * Value of the specified property member, or -1 if the operation
 * failed, for example if context is NULL or the database is not
 * opened.
 *
 * \see
 * a_bseContext a_bseStateKind
 */
c_long a_bseGetStateProperty(a_bseContext context, a_bseStateKind stateKind);


/**
 * \brief
 * Returns a pointer to the c_base instance of this file's context
 *
 * This operation returns a pointer to the c_base value, holded by
 * this file's context, of the currently opened database.
 *
 * \param context
 * This file's context
 *
 * \return
 * Pointer to the c_base value of the currently opened database or
 * NULL if the database is not opened or if context is NULL.
 *
 * \see
 * a_bseContext a_bseOpenBase
 */
c_base a_bseGetBase(a_bseContext context);


/**
 * \brief
 * Sets a (new) database name to attach to
 *
 * This operation sets a new database name in the context,
 * overwriting an existing one.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail
 *
 * \param newDbName
 * The (new) database name. Internally, the string will be
 * duplicated. If newDbName is NULL, this operation will \b not fail,
 * but a call to a_bseOpenBase probably will.
 *
 * \return
 * Boolean value specifying whether the operation was successful. If
 * context is NULL or the database is currently opened, the operation
 * will fail.
 *
 * \see
 * a_bseContext a_bseOpenBase
 */
int a_bseSetDatabaseName(a_bseContext context, char *newDbName);


/**
 * \brief
 * Opens a database
 *
 * This operation opens a database, using the name held in the
 * context (that was set at context creation or changed through
 * a_bseSetDatabaseName).
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \param shmAddress
 * Memory Start Address of the currently attached Shared Memory
 * segment.
 *
 * \return
 * Boolean value specifying whether the operation was succesful.
 *
 * \see
 * a_bseContext
 */
int a_bseOpenBase(a_bseContext context, c_address shmAddress);


#endif  /* A_BSE_H */

//END a_bse.h
