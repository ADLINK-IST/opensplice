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
 /* AAPI is built on both SPLICE-DDS's os- and database-layers. In
 * general, AAPI will create an exact copy of some currently existing
 * Shared Memory Segment, in which a SPLICE-DDS Database is running.
 * With this copy action, a new Shared Memory Segment is created,
 * that will now behave as a static snapshot, in which we have all
 * the time to analyse the data.\n
 * \n
 * Some extra functionality, like saving and loading the Shared
 * Memory to or from a file, is available. This will expand the
 * analysing possibilities.
 *
 * \note
 * All operations in this file are designed to be fail safe.
 */


#ifndef A_API_H
#define A_API_H

#include <stdio.h>


/**
 * \brief AAPI Errors
 *
 * Definition of AAPI's error numbers.
 */
typedef enum a_error {
	A_ERR_OK,                    ///< No errors
	A_ERR_NO_CONTEXT,            ///< No context specified in \a a_apiLastError
	A_ERR_SHM_ATTACH_FAIL,       ///< Shared Memory could not be attached to
	A_ERR_SHM_CREATE_FAIL,       ///< Shared Memory could not be created
	A_ERR_SHM_DETACH_FAIL,       ///< Shared Memory could not be detached from
	A_ERR_SHM_NOT_ATTACHED,      ///< Shared Memory is not attached
	A_ERR_SHM_COPY_TO_HEAP_FAIL, ///< Shared Memory could not be copied to Heap
	A_ERR_SAVE_TO_FILE_FAIL,     ///< Shared Memory could not be saved to file
	A_ERR_LOAD_FROM_FILE_FAIL,   ///< Shared Memory could not be loaded from file
	A_ERR_BASE_OPEN_FAIL,        ///< Database could not be opened
	A_ERR_MALLOC_FAIL,           ///< malloc function failed
	A_ERR_MEMCOPY_FAIL,          ///< memcpy function failed
	A_ERR_COUNT                  ///< Number of errors defined (used internally)
} a_error;



/**
 * \brief
 * Kind of List References, per object entry.
 *
 * Definition of references type, i.e. the kind of
 * sublist of an entry.
 */
typedef enum a_apiRefsKind {
	A_REF_UNDEFINED,             ///< Undefined sublist, used internally
	A_REF_TYPEREF,               ///< Specifying the sublist \a Type \a References
	A_REF_DATAREF,               ///< Specifying the sublist \a Data \a References
	A_REF_UNKNREF,               ///< Specifying the sublist \a Unknown \a References
	A_REF_REFTOTYPE,             ///< Specifying the sublist \a References \a to \a Type
	A_REF_REFTODATA,             ///< Specifying the sublist \a References \a to \a Data
	A_REF_REFTOUNKN,             ///< Specifying the sublist \a References \a to \a Unknown
	A_REF_OCCURRENCES,           ///< Specifying the sublist \a Occurrences
	A_REF_COUNT                  ///< Number of kinds of references, used internally
} a_apiRefsKind;





/**
 * \brief
 * AAPI Context (hidden data structure)
 */
typedef struct a_apiContext_s *a_apiContext;



/**
 * \brief
 * Data Structure holding the totals counted by \a a_apiListWalk.
 *
 * Data structure holding the totals counted by \a a_apiListWalk.\n
 *
 * \note
 * These are NOT the totals of the complete list, per definition.
 */
typedef struct a_apiListTotals {
	long objs;                   ///< Nr of objects counted
	long refC;                   ///< Sum of all Reference Counts
	long tRef;                   ///< Sum of all Type references
	long dRef;                   ///< Sum of all Data References
	long uRef;                   ///< Sum of all Other, Unknown References
	long diff;                   ///< Sum of all differences (\a refC - \a tRef - \a dRef)
	long occrs;                  ///< Sum of all occurrences
	long odiff;                  ///< Sum of all Occurrence Differences (\a occrs - \a *Ref)
} *a_apiListTotals;



/**
 * \brief
 * AAPI Timer Results
 */
typedef struct a_apiTimerResults {
	unsigned long  shmCopyMilSecs;   ///< Duration of copying the 'live' shared memory in milliseconds
	unsigned long  shmCopyMicroSecs; ///< Duration of copying the 'live' shared memory in microseconds
	unsigned long  listFillMilSecs;  ///< Duration of filling the internal aapi list in milliseconds
	unsigned long  analyseMilSecs;   ///< Duration of analysing all internal aapi data in milliseconds
} *a_apiTimerResults;





/**
 * \brief
 * Data Structure for holding gathered information about the database
 * and computed values during the database's analyse.
 */
typedef struct a_apiInfoData_s {
	char *shmName;    ///< Shared Memory Name
	char *dbName;     ///< Database Name
	long shmAddress;  ///< Shared Memory Start Address
	long shmSize;     ///< Shared Memory Size
	long bseAddress;  ///< Database Start Address
	long mmSize;      ///< mmState's Size
	long mmUsed;      ///< mmState's Used
	long mmMaxUsed;   ///< mmState's MaxUsed
	long mmFails;     ///< mmState's Fails
	long mmGarbage;   ///< mmState's Garbage
	long mmCount;     ///< mmState's Count
	long clssObjs;    ///< Total number of objects of \a CLSS (C) Type
	long baseObjs;    ///< Total number of objects of \a BASE (B) Type
	long metaObjs;    ///< Total number of objects of \a META (M) Type
	long dataObjs;    ///< Total number of objects of \a DATA (D) Type
	long undfObjs;    ///< Total number of objects of \a UNDF (?) Type
	long dataSize;    ///< Total amount of memory used for all database objects
} *a_apiInfoData;




/**
 * \brief
 * Data Structure for returning information about a database object.
 */
typedef struct a_apiObject {
	long    objectAddress;    ///< Object's Memory Start Address (skipping the header)
	long    referenceCount;   ///< Value of the \a Reference \a Count for this object, returned by SPLICE
	long    typeRefsCount;    ///< Number of times this object has a \a Reference \a as \a Type
	long    dataRefsCount;    ///< Number of times this object has a \a Reference \a as \a Data
	long    unknRefsCount;    ///< Number of times this object has an \a Uncategorised \a Reference
	long    refsDifference;   ///< referenceCount - typeRefsCount - DataRefsCount
	long    refsToTypeCount;  ///< Number of times this object references to another object as type
	long    refsToDataCount;  ///< Number of times this object references to another object as data
	long    refsToUnknCount;  ///< Number of times this object references to another object uncategorised
	long    occurrencesCount; ///< Number of times pointers were found in the Shared Memory that point to this object
	long    occurrenceDiff;   ///< occurrencesCount - refsTo*Count, should be zero at all times
	long    alignment;        ///< Value of the object's alignment, kept by this object's \a type \a object in SPLICE
	long    size;             ///< Value of the object's size, kept by this object's \a type \a object in SPLICE
	long    ourSize;          ///< Value of the object's size, computed by AAPI (differs with c_strings)
	char   *objectName;       ///< This object's name, if any
	char   *typeName;         ///< The name of this object's type (if any)
	char   *typeDesc;         ///< Short description of this object's type (if any)
	char   *value;            ///< Human Readable representation of this object's value, like strings and longs
	char   *note;             ///< AAPI produced note(s) for this object
	char   *objectKind;       ///< Single Character, specifying this object's kind ('C', 'B', 'M', 'D' or '?')
} *a_apiObject;


/***********************************************************
 INITIALISATION AND DE_INITIALISATION
 ***********************************************************/

/**
 * \brief
 * Initialises the context for this file
 *	  
 * This operation creates a new context for this file's use and must
 * be executed before any other operation in this file. Remember to
 * call the de-init operation after use, and before the application
 * terminates.
 *
 * \param out
 * Pointer to where output must be redirected to (typically stdout).
 * This value is used for outputting debug info, verbose output (see
 * below) and hex dumps.
 *
 * \param err
 * Pointer to where error messages must be redirected to (typically
 * stderr). This value is used for outputting debug info, verbose
 * output (see below) and hex dumps.
 *
 * \param shm_name
 * The Shared Memory Name to attach to.
 *
 * \param db_name
 * The Database Name (within the shared memory) to open.
 *
 * \param dir
 * The directory where SPLICE's temporary key files are stored for
 * the AAPI to parse. Typically "/tmp".
 *
 * \param mask
 * File Mask of SPLICE's temporary key files. Typically "spddskey_".
 * Do not use wildcards here, AAPI will asume a "*" as a suffix
 * itself.
 *
 * \param showAnalyseOutput
 * Boolean setting whether to show how all object info is gathered
 * and processed (debug info). It uses \a out for redirection.
 *
 * \param verboseOutput
 * Boolean setting whether to show some progress info about AAPI's
 * analyse stage. With databases with more than - say - 5000 objects,
 * this setting might become useful.
 *
 * \return
 * Pointer to the new context, or NULL if anything failed.
 *
 * \see
 * a_apiDeInit
 */
a_apiContext a_apiInit(FILE *out, FILE *err, char *shm_name, char *db_name,
	char *dir, char *mask, int showAnalyseOutput, int verboseOutput);


/**
 * \brief
 * De-initialises the context.
 *
 * This operation must be executed before application's termination.
 * It will free up all memory used.
 *
 * \param context
 * The context for this file, which must have been created with
 * a_apiInit. If context is NULL, this operation will fail.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_apiContext a_apiInit
 */
int a_apiDeInit(a_apiContext context);



/***********************************************************
 ERROR QUERY FUNCTIONS
 ***********************************************************/

/**
 * \brief
 * Returns AAPI's last Error.
 *
 * This operation returns the (internal) error number of the
 * last error occurred.
 *
 * \param context
 * This file's context.
 *
 * \return
 * Number of AAPI's last occurred error, or -1 if context is NULL.
 *
 * \see
 * a_error a_apiErrorStr
 */
int a_apiLastError(a_apiContext context);


/**
 * \brief
 * Returns an error description
 *
 * \param errno
 * The Error Number of which a description is requested.
 *
 * \return
 * A human readable (static) string representation of \a errno.
 *
 * \see
 * a_apiLastError
 */
char *a_apiErrorStr(a_error errno);



/***********************************************************
 AAPI "PREPARE"
 ***********************************************************/


/**
 * \brief
 * Prepares the AAPI.
 *
 * This operation prepares the AAPI for analysing. It will take
 * the following actions:\n
 * \arg Attaches to Shared Memory;
 * \arg Copies the Shared Memory to Heap,
 * \arg Detaches the Shared Memory,
 * \arg Creates a "Private" Shared Memory Segment and
 *      Copies from Heap to the Private Shared Memory.
 *
 * \param context
 * This file's context
 *
 * \param memFname
 * File name of which a copy of the memory should be copied to. The
 * file may not already exist. If it does, this function will fail.
 * Specify NULL if you do not wish to save a copy.
 *
 * \param loadInsteadOfSave
 * Boolean setting for specifying whether you wish to load the shared
 * memory portion from a file (specified by \a memFname) instead of
 * copying from a currently existing shared memory segment. If the
 * file does not exist, this function will fail. If the file you are
 * trying to load with this operation was not created by this same
 * function, the outcome is undetermined. If \a memFname is NULL,
 * this parameter will be ignored.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). In case of failure, the internal \a errorno will be set.
 *
 * \see
 * a_apiContext
 */
int a_apiPrepare(a_apiContext context, char *memFname, int loadInsteadOfSave);



/***********************************************************
 ANALYSE
 ***********************************************************/


/**
 * \brief
 * Analyses the database.
 *
 * This operation analyses the database and gathers all object
 * information into an internal data structure. It uses settings
 * that were specified at context creation. This operation
 * should be called right after a (successful) \a a_apiPrepare
 * (unless only a Hexdump is required). After this operation,
 * the results of the analysis can be queried through several
 * other operations in this file.
 *
 * \param context
 * This file's context.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). In case of failure, the internal \a errno will be set.
 *
 * \see
 * a_apiContext a_apiPrepare
 */
int a_apiAnalyse(a_apiContext context);



/***********************************************************
 HEXDUMP
 ***********************************************************/

/**
 * \brief
 * Creates a hex dump of the Shared Memory.
 *
 * This operation will start a hex dump and outputs to
 * the filepointer \a out that was specified at context's
 * creation time.
 *
 * \param context
 * This file's context.
 *
 * \param width
 * The number of bytes per line to be displayed.
 *
 * \param group
 * The number of bytes to group before an extra space is inserted. If
 * \a group is equal to or greater than \a width, or if \a group is
 * 0, grouping will be disabled.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0).
 *
 * \see
 * a_apiContext a_apiHexdumpWiped
 */
int a_apiHexdump(a_apiContext context, int width, int group);


/**
 * \brief
 * Wipes all known objects in the Shared Memory with a predefined
 * value and creates a hex dump of the Shared Memory.
 *
 * This operation wipes all known database objetcs, that
 * were collected earlier, with a predefined value and creates
 * a hex dump of the Shared Memory.
 *
 * \warning
 * This operation overwrites the currently attached
 * (by aapi) shared memory. After this operation's use, the
 * Shared Memory is no longer valid by means of SPLICE, and
 * should be detached. This operation should never be used
 * on a \a live SPLICE database, although within current
 * aapi design this is not possible.
 * 
 * \param context
 * This file's context.
 *
 * \param width
 * The number of bytes per line to be displayed.
 *
 * \param group
 * The number of bytes to group before an extra space is inserted. If
 * \a group is equal to or greater than \a width, or if \a group is
 * 0, grouping will be disabled.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0).
 *
 * \see
 * a_apiContext a_apiHexdump
 */
int a_apiHexdumpWiped(a_apiContext context, int width, int group);



/***********************************************************
 WALK FUNCTIONS
 ***********************************************************/

/**
 * \brief
 * Type Definition of the callback action for the Walk and Query
 * Functions.
 *
 * \param apiObject
 * Instance of apiObject that the user defined function will be
 * passed along with.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with.
 *
 * \return
 * Boolean value passing back at the walk, specifying whether the
 * walk should continue (1) or abort (0).
 *
 * \see
 * a_apiListWalk a_apiAddressQuery
 */
typedef int (*a_apiWalkAction)(a_apiObject apiObject, void *actionArg);



/**
 * \brief
 * Walks over all entries in the (internal) list and calls a user
 * defined function for every entry.
 *
 * This operation will \a walk over all entries in the internal list,
 * that were gathered during the \a a_apiAnalyse operation. A copy of
 * \a a_apiObject will be passed along. Remember that this copy of
 * \a a_apiObject will be destroyed upon returning from the user
 * defined function, so make sure you save the information from this
 * \a a_apiObject you may want to use later.\n
 * The user defined function must return true (>0) to continue the
 * walk. If 0 is returned, the walk will abort and return 0 itself,
 * to indicate an aborted walk.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit.
 *
 * \param walkAction
 * Pointer to a user defined function that will be called with every
 * list's entry.
 *
 * \param walkActionArg
 * Pointer to a user defined context that will be passed along with
 * every call to the user defined function. May be NULL.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0).
 *
 * \see
 * a_apiContext a_apiWalkAction a_apiAddressQuery
 */
int a_apiListWalk(a_apiContext context, a_apiWalkAction walkAction, void *walkActionArg);


/**
 * \brief
 * 'Walks' over just one entry in the (internal) list and calls a
 * user defined function for that entry.
 *
 * This operation will call a user defined function, passing along
 * one entry from the list, specified by its address. The callback
 * function must be of the same structure that is used for
 * \a a_apiListWalk, in fact, it might be exactly the same function.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit. If
 * context is NULL, this operation will fail.
 *
 * \param address
 * The address that will be used as a search key in the list, of
 * which an entry must be returned in a callback function. If
 * \a address could not be found in the list, this operation will
 * fail.
 *
 * \param walkAction
 * Pointer to a user defined function that will be called with every
 * list's entry. If \a walkAction is NULL, this operation will fail.
 *
 * \param walkActionArg
 * Pointer to a user defined context that will be passed along with
 * every call to the user defined function. May be NULL.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0).
 *
 * \see
 * a_apiContext a_apiWalkAction a_apiListWalk
 */
int a_apiAddressQuery(
	a_apiContext context, long address, a_apiWalkAction walkAction,
	void *walkActionArg);



/**
 * \brief
 * Type Definition of the callback action for the seperate Addresses
 * Walk Functions that query sublist of an instance of
 * \a a_apiObject.
 *
 * \param address
 * An address out of the sublist that is walked over.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with.
 *
 * \param refsKind
 * Value of the entry's sublist type that the calling walk operation
 * is walking over.
 *
 * \return
 * Boolean value passing back at the walk, specifying whether the
 * walk should continue (1) or abort (0).
 *
 * \see
 * a_apiAddressReferencesWalk a_apiRefsKind
 */
typedef int (*a_apiAddressWalkAction)
	(long address, void *actionArg, a_apiRefsKind refsKind);


/**
 * \brief
 * Walk Function for walking over all object's reference's, specified
 * by its address and reference kind.
 *
 * This operation will walk over all references of one entry from
 * the list. \a walkAction (a user defined function) will be called
 * for every reference found in that sublist and it must return
 * true (1) to continue the walk. If false (0) is returned, the walk
 * will abort and return 0 as well, indicating an aborted walk.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit. If
 * context is NULL, this operation will fail.
 *
 * \param address
 * The address that will be used as a search key in the list, of which
 * an entry must be returned in a callback function. If \a address
 * could not be found in the list, this operation will fail.
 *
 * \param walkAction
 * Pointer to a user defined function that will be called with every
 * list's entry. If \a walkAction is NULL, this operation will fail.
 *
 * \param walkActionArg
 * Pointer to a user defined context that will be passed along with
 * every call to the user defined function. May be NULL.
 *
 * \param refsKind
 * Value of the entry's sublist type that this operation should walk
 * over.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). If no references were found, 0 is returned also, indicating
 * no callbacks have been made.
 *
 * \see
 * a_apiAddressWalkAction
 */
int a_apiAddressReferencesWalk(
	a_apiContext context, long address, a_apiAddressWalkAction walkAction,
	void *walkActionArg, a_apiRefsKind refsKind);



/***********************************************************
 INFORMATION GATHERING
 ***********************************************************/

/**
 * \brief
 * Type Definition of the user defined function that will be called
 * by \a a_apiPushMeListTotals.
 *
 * \param listTotals
 * An instance of \a a_apiListTotals, holding all aapi-computed
 * totals of the internal list
 *
 * \param actionArg
 * User defined context to be passed along
 *
 * \return
 * The user defined function is expected to return false (0) or true
 * (1), which might be passed back in the original function.
 *
 * \see
 * a_apiPushMeListTotals a_apiListTotals
 */
typedef int (*a_apiListTotalsAction)
	(a_apiListTotals listTotals, void *actionArg);


/**
 * \brief
 * Calls a user defined callback function to "push" the list totals,
 * computed by aapi.
 *
 * This operation will call a user defined function, of that of
 * an \a a_apiListTotalsAction type, to pass back list totals,
 * as they were computed during the \a a_apiAnalyse.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit. If
 * context is NULL, this operation will fail.
 *
 * \param action
 * Pointer to a user defined function that will be called. If
 * \a action is NULL, this operation will fail.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with
 * the call to the user defined function. May be NULL.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0).
 *
 * \see
 * a_apiListTotalsAction a_apiListTotals
 */
int a_apiPushMeListTotals(
	a_apiContext context, a_apiListTotalsAction action, void *actionArg);



/**
 * \brief
 * Type Definition of the user defined function that will be called
 * by \a a_apiPushMeTimerResults.
 *
 * \param listTotals
 * An instance of \a a_apiTimerResults, holding all timer results
 * that were administrated during several aapi actions.
 *
 * \param actionArg
 * User defined context to be passed along
 *
 * \see
 * a_apiPushMeTimerResults a_apiTimerResults
 */
typedef int (*a_apiTimerResultsAction)
	(a_apiTimerResults timerResults, void *actionArg);



/**
 * \brief
 * Calls a user defined call back function to "push" several timer
 * results.
 *
 * This operation calls a user defined function, of that of an
 * \a a_apiTimerResultsAction type, to pass back an instance of
 * \a a_apiTimerResults. Upon returning from the user defined
 * function, this instance will be destroyed.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit. If
 * context is NULL, this operation will fail.
 *
 * \param action
 * Pointer to a user defined function that will be called. If
 * \a action is NULL, this operation will fail.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with
 * the call to the user defined function. May be NULL.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). If the user defined function returns 0, this function will
 * return 0 as well.
 *
 * \see
 * a_apiTimerResultsAction a_apiTimerResults
 */
int a_apiPushMeTimerResults(
	a_apiContext context, a_apiTimerResultsAction action, void *actionArg);


/**
 * \brief
 * Type Definition of the user defined function that will be called
 * by \a a_apiPushMeInfoData.
 *
 * \param infoData
 * An instance of \a a_apiInfoData, holding some brief statistics,
 * gathered by \a a_apiAnalyse.
 *
 * \param actionArg
 * User defined context to be passed along
 *
 * \see
 * a_apiPushMeInfoData a_apiInfoData
 */
typedef int (*a_apiInfoDataAction)
	(a_apiInfoData infoData, void *actionArg);


/**
 * \brief
 * Calls a user defined call back function to "push" some statistics
 * about the database.
 *
 * This operation calls a user defined function, of that of an
 * \a a_apiInfoDataAction type, to pass back an instance of
 * \a a_apiInfoData. Upon returning from the user defined function,
 * this instance will be destroyed.
 *
 * \param context
 * This file's context. It must have been created by \a a_apiInit. If
 * context is NULL, this operation will fail.
 *
 * \param action
 * Pointer to a user defined function that will be called. If
 * \a action is NULL, this operation will fail.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with
 * the call to the user defined function. May be NULL.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). If the user defined function returns 0, this function will
 * return 0 as well.
 *
 * \see
 * a_apiInfoDataAction a_apiInfoData
 */
int a_apiPushMeInfoData(
	a_apiContext context, a_apiInfoDataAction action, void *actionArg);



/***********************************************************
 FREE SPACE STATS
 ***********************************************************/

/**
 * \brief
 * Type definition of a user defined function for
 * a_apiFreeSpaceStatsWalk to call back to.
 *
 * \param freeSpaceSize
 * \param count
 * \param arg
 * Pointer to a user defined context to pass along
 *
 * \return
 * The user defined function must return true (1) to let the walk
 * continue. If false (0) is returned, the walk will abort.
 *
 * \see
 * a_apiFreeSpaceStatsWalk
 */
typedef int (*a_apiFreeSpaceStatsWalkAction)
	(long freeSpaceSize, long count, void *arg);


/**
 * \brief
 * Walks over all FreeSpaceCounters
 *
 * This operation walks over all FreeSpaceCounters and calls a user
 * defined function for every counter.
 *
 * \param context
 * This file's context
 *
 * \param walkAction
 * Pointer to a user defined function that will be called for every
 * FreeSpaceCounter
 *
 * \param arg
 * pointer to a user defined context that will be passed along
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_apiContext a_apiFreeSpaceStatsWalkAction
 * a_apiFreeSpaceStatsCount
 */
int a_apiFreeSpaceStatsWalk(
	a_apiContext context, a_apiFreeSpaceStatsWalkAction walkAction, void *arg);


/**
 * \brief
 * Returns the number of FreeSpaceCounters collected
 *
 * This operation returns the number of collected
 * FreeSpaceCounters.
 *
 * \param context
 * This file's context. if context is NULL, the operation will fail.
 *
 * \return
 * The number of collected FreeSpaceCounters, or -1 if the operation
 * failed.
 *
 * \see
 * a_apiContext a_apiFreeSpaceStatsWalk
 */
int a_apiFreeSpaceStatsCount(a_apiContext context);


#endif   /* A_API_H */

//END a_api.h
