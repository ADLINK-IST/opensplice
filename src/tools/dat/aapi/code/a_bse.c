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
	c_base   base;       ///< Pointer to c_base
};



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
a_bseContext
a_bseInit(
	char *db_name)
{
	a_bseContext context = a_memAlloc(sizeof(struct a_bseContext_s));
	context->db_name     = a_memStrdup(db_name);
	context->base        = NULL;
	return context;
}



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
int
a_bseDeInit(
	a_bseContext context)
{
	int result;
	if (context) {
		if (context->db_name) {
			a_memFree(context->db_name);
		}
		a_memFree(context);
		result = 1;
	} else {
		result = 0;
	}
	return result;
}



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
int
a_bseIsOpened(
	a_bseContext context)
{
	return context ? context->base ? 1 : 0 : 0;
}



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
char *
a_bseGetBaseName(
	a_bseContext context)
{
	return context ? context->db_name : NULL;
}



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
c_address
a_bseGetBaseAddr(
	a_bseContext context)
{
	c_address result;
	if (context) {
		if (a_bseIsOpened(context)) {
			result = (c_address)c_mmAddress(c_baseMM(context->base));
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}



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
c_long
a_bseGetStateProperty(
	a_bseContext context,
	a_bseStateKind stateKind)
{
	long result;
	if (a_bseIsOpened(context)) {
		c_mmStatus state = c_mmState(c_baseMM(context->base));
		switch(stateKind) {
			case A_BSE_STATE_SIZE:    result = state->size;    break;
			case A_BSE_STATE_USED:    result = state->used;    break;
			case A_BSE_STATE_MAXUSED: result = state->maxUsed; break;
			case A_BSE_STATE_FAILS:   result = state->fails;   break;
			case A_BSE_STATE_GARBAGE: result = state->garbage; break;
			case A_BSE_STATE_COUNT:   result = state->count;   break;
			default:                  result = -1;             break;
		}
	} else {
		result = -1;
	}
	return result;
}



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
c_base
a_bseGetBase(
	a_bseContext context)
{
	return context->base;
}



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
int
a_bseSetDatabaseName(
	a_bseContext context,
    char *newDbName)
{
	int result;
	if (context) {
		if (!a_bseIsOpened(context)) {
			if (context->db_name) {
				a_memFree(context->db_name);
			}
		    context->db_name = newDbName ? a_memStrdup(newDbName) : newDbName;
			result = 1;
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}




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
int
a_bseOpenBase(
	a_bseContext context,
	c_address shmAddress)
{
	int result;
	if (context) {
		context->base = c_open(context->db_name, (void *)shmAddress);
		result = a_bseIsOpened(context);
	} else {
		context = 0;
	}
	return result;
}


//END a_bse.c
