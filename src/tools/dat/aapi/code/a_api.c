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
 */



#include <stdio.h>

#include "a_api.h"
#include "a_bse.h"
#include "a_mem.h"
#include "a_shm.h"
#include "a_utl.h"
#include "a_hex.h"
#include "a_lst.h"
#include "a_sts.h"
#include "a_anl.h"
#include "a_key.h"
#include "a_fil.h"



#if 0

#define FUNC(name)  const char *func = name;
#define HERE(msg)   printf("[%-25s] %s\n", func, msg)
#define CHECK(addr) printf("[%-25s] 0x%X\n", func, (unsigned int)addr)
#define MORE(msg)   HERE(msg) ; printf("-More-"); getchar()

#else

#define FUNC(name)  ;
#define HERE(msg)   ;
#define CHECK(addr) ;
#define MORE(msg)   ;

#endif


/* What string to append to the original Shared Memory Name
 * when creating our own, "private" Shared Memory?
 */
#define A_API_SHM_NAME_APPEND "(aapi)"



/* Internal context structure
 */
struct a_apiContext_s {
	c_address         heapMem;
	FILE             *out;
	FILE             *err;
	int               showAnalyseOutput;
	int               verboseOutput;

	char             *shmName;
	a_shmContext      shmContext;
	c_address         address;          // Start Address of shm, typically 0xA0000000
	long              size;             // Size of shm block

	a_bseContext      bseContext;
	char             *dbName;

	a_utlContext      utlContext;
	a_anlContext      anlContext;
	a_lstList         list;
	a_keyContext      keyContext;
	a_filContext      filContext;
	a_stsContext      stsContext;
	
	a_apiListTotals   listTotals;
	a_apiTimerResults timerResults;
	
	int               error;
};



/***********************************************************
 INITIALISATION AND DE_INITIALISATION
 ***********************************************************/


static a_apiListTotals
a_apiCreateListTotals()
{
	a_apiListTotals apiListTotals = a_memAlloc(sizeof(struct a_apiListTotals));
	if (apiListTotals) {
		apiListTotals->objs  = 0;
		apiListTotals->refC  = 0;
		apiListTotals->tRef  = 0;
		apiListTotals->dRef  = 0;
		apiListTotals->uRef  = 0;
		apiListTotals->diff  = 0;
		apiListTotals->occrs = 0;
		apiListTotals->odiff = 0;
	}
	return apiListTotals;
}



static a_apiTimerResults
a_apiCreateTimerResults()
{
	a_apiTimerResults timerResults = a_memAlloc(sizeof(struct a_apiTimerResults));
	if (timerResults) {
		timerResults->shmCopyMilSecs   = 0;
		timerResults->shmCopyMicroSecs = 0;
		timerResults->listFillMilSecs  = 0;
		timerResults->analyseMilSecs   = 0;
	}
	return timerResults;
}



/* Sets a (new) Shared Memory Name in the context
 */
static void
a_apiSetNewShmName(
	a_apiContext context,
	char *newShmName)
{
	if (context) {
		if (context->shmName) {
			a_memFree(context->shmName);
		}
		context->shmName = a_memStrdup(newShmName);
	}
}



/* Sets a (new) Database Name in the context
 */
static void
a_apiSetNewDbName(
	a_apiContext context,
	char *newDbName)
{
	if (context) {
		if (context->dbName) {
			a_memFree(context->dbName);
		}
		context->dbName = a_memStrdup(newDbName);
		if (context->bseContext) {
			a_bseSetDatabaseName(context->bseContext, newDbName);
		}
	}
}




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
a_apiContext
a_apiInit(
	FILE *out,
	FILE *err,
	char *shmName,
	char *dbName,
	char *dir,
	char *mask,
	int showAnalyseOutput,
	int verboseOutput)
{
	a_apiContext context = a_memAlloc(sizeof(struct a_apiContext_s));
	if (context) {
		context->heapMem           = (c_address)NULL;
		context->out               = out;
		context->err               = err;
		context->showAnalyseOutput = showAnalyseOutput;
		context->verboseOutput     = verboseOutput;

		context->shmName           = a_memStrdup(shmName);
		context->shmContext        = a_shmInit(context->out, context->err, context->shmName);
		context->address           = 0;
		context->size              = 0;
	
		context->dbName            = a_memStrdup(dbName);
		context->bseContext        = a_bseInit(context->dbName);

		context->utlContext        = a_utlInit();
		context->list              = a_lstCreateList(A_OCCURRENCES_ARRAY_SIZE);
		context->stsContext        = a_stsInit();
		context->anlContext        = a_anlInit(context->out, context->err, context->list,
	    	                         context->showAnalyseOutput, context->verboseOutput,
									 context->stsContext);
	
		context->keyContext        = a_keyInit(dir, mask);
		context->filContext        = (c_address)NULL;
	
		context->listTotals        = a_apiCreateListTotals();
		context->timerResults      = a_apiCreateTimerResults();
	
		context->error             = A_ERR_OK;
	}
	return context;
}



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
int
a_apiDeInit(
	a_apiContext context)
{
	int result;
	FUNC("a_apiDeInit");
	if (context) {
		if (context->keyContext) {		
			HERE("keyDeInit(keyContext):");
			a_keyDeInit(context->keyContext);
		}
		if (context->anlContext) {
			HERE("anlDeInit(anlContext):");
			a_anlDeInit(context->anlContext);
		}
		if (context->stsContext) {
			HERE("stsDeInit(stsContext):");
			a_stsDeInit(context->stsContext);
		}
		if (context->list) {
			HERE("lstDestroyList(list):");
			a_lstDestroyList(context->list);
		}
		if (context->utlContext) {
			HERE("utlDeInit(utlContext):");
			a_utlDeInit(context->utlContext);
		}
		if (context->bseContext) {
			HERE("bseDeInit(bseContext):");
			a_bseDeInit(context->bseContext);
		}
		if (context->shmContext) {
			HERE("shmDetach(shmContext):");
			a_shmDetach(context->shmContext);

			HERE("shmDestroy(shmContext):");
			a_shmDestroyShm(context->shmContext);

			HERE("shmDeInit(shmContext):");
			a_shmDeInit(context->shmContext);
		}
		if (context->shmName) {
			a_memFree(context->shmName);
		}
		if (context->dbName) {
			a_memFree(context->dbName);
		}
		HERE("a_memFree(context):");
		a_memFree(context);
		result = 1;
	} else {
		result = 0;
	}
	return result;
}



/***********************************************************
 TESTING THE AAPI: patch the shared memory with faulty data
 ***********************************************************/


#if 0

// YES!!! The good old "poke"!  ;-)

static void
a_apiPoke(
	c_address atAddress,
	c_address addressValue)
{
	c_address *atAddressPtr = (c_address *)atAddress;
	*atAddressPtr = addressValue;
}


// Patch some values into the shm, to invoke some "unexpected errors"  ;-)

static void
a_apiPatchShm()
{
	a_apiPoke((c_address)0xA000321C, (c_address)0xA00FBD38);   // patch dummy pointer to our own, new object
	a_apiPoke((c_address)0xA00FBD20, (c_address)A_CONFIDENCE); // set the confidence check in the header
	a_apiPoke((c_address)0xA00FBD2C, (c_address)0x00000005);   // set refcount in the header to 5
	a_apiPoke((c_address)0xA00FBD38, (c_address)0x52555953);   // label the object with some stupid name
}

#endif


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
int
a_apiLastError(
	a_apiContext context)
{
	return context ? context->error : -1;
}



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
char *
a_apiErrorStr(
	a_error errno)
{
	switch (errno) {
		case A_ERR_OK:
			return "Ok"; break;
		case A_ERR_NO_CONTEXT:
			return "Context with value NULL used"; break;
		case A_ERR_SHM_ATTACH_FAIL:
			return "Failed to attach to Shared Memory (check shm name)"; break;
		case A_ERR_SHM_CREATE_FAIL:
			return "Failed to create Shared Memory (out of memory?)"; break;
		case A_ERR_SHM_DETACH_FAIL:
			return "Failed to detach Shared Memory"; break;
		case A_ERR_SHM_NOT_ATTACHED:
			return "Shared Memory not attached"; break;
		case A_ERR_SHM_COPY_TO_HEAP_FAIL:
			return "Failed to copy shm to heap"; break;
		case A_ERR_SAVE_TO_FILE_FAIL:
			return "Failed to save heap memory to file"; break;
		case A_ERR_LOAD_FROM_FILE_FAIL:
			return "Failed to load file into heap memory"; break;
		case A_ERR_BASE_OPEN_FAIL:
			return "Failed to open database (check db name)"; break;
		case A_ERR_MALLOC_FAIL:
			return "Failed to allocate memory on heap"; break;
		case A_ERR_MEMCOPY_FAIL:
			return "Failed to copy memory block"; break;
		default:
			return "General AAPI Failure."; break;          // lekker vage melding  :-)
	}
}



/***********************************************************
 AAPI "PREPARE"
 ***********************************************************/


/* Part of a_apiPrepare:
 * Saves the memory block on heap to a file. The actual
 * saving is done in a_fil.
 */
static int
a_apiPrepareSaveHeap2File(
	a_apiContext context,
	char *memFname)
{
	int result;
	context->filContext = a_filInit(memFname, context->shmName, context->dbName, context->address, context->size);
	result = a_filHeap2File(context->filContext, context->heapMem);
	return result;
}



/* Part of a_apiPrepare:
 * Loads (fills) the memory on heap from file.
 * The actual loading is done in a_fil.
 */
static int
a_apiPrepareLoadFile2Heap(
	a_apiContext context,
	char *memFname)
{
	FUNC("a_apiPrepareLoadFile2Heap");
	int result = 0;
	context->filContext = a_filInit(memFname, "", "", (c_address)NULL, 0);
	HERE("filContext initialised");
	result = a_filReadHeader(context->filContext);
	if (result) {
		HERE("Header read success");
		char *shmName = a_filGetShmName(context->filContext);
		char *dbName = a_filGetDbName(context->filContext);
		c_address address = a_filGetShmAddress(context->filContext);
		long size = a_filGetShmSize(context->filContext);
		if (shmName) {
			a_apiSetNewShmName(context, shmName);
		}
		if (dbName) {
			a_apiSetNewDbName(context, dbName);
		}
		if (address) {
			context->address = address;
		}
		if (size) {
			context->size = size;
		}
		if ((context->heapMem = (c_address)a_memAlloc(context->size)) != (c_address)NULL) {
			HERE("malloc success");
			result = a_filFile2Heap(context->filContext, context->heapMem);
			if (result) {
				HERE("result of a_filFile2Heap: success");
			} else {
				HERE("result of a_filFile2Heap: fail");
			}
		} else {
			HERE("malloc fail");
			result = 0;
		}
	} else {
		HERE("Header read fail");
	}
	return result;
}



/* Part of a_apiPrepare:
 * Copies Shared Memory to Heap
 */
static int
a_apiPrepareCopyShm2Heap(
	a_apiContext context)
{
	FUNC("a_apiPrepareCopyShm2Heap");
	/* Use origShmContext for attaching the original shm: */
	a_shmContext origShmContext;
	
	context->error = A_ERR_OK;
	origShmContext = a_shmInit(context->out, context->err, context->shmName);
	
	if ( a_shmAttach(origShmContext) ) {
		HERE("attached");
		
		// shm is attached, now determine start address:
		context->address = a_shmGetShmAddress(origShmContext);
		HERE("startAddress acquired");
		assert(context->address);

		if ( a_bseOpenBase(context->bseContext, context->address) ) {
			HERE("base opened");

			context->size = (long)a_keyGetSize(context->keyContext, context->shmName);
			// printf("size: %ld (0x%X)\n", context->size, (unsigned int)context->size);
		
			HERE("size acquired, going to copy shm mem to heap:");
			//printf("[a_apiPrepare] address = 0x%X, size = %ld (0x%X)\n",
			//	(unsigned int)context->address, context->size, (unsigned int)context->size);
		
			// copy shm to heap:
			assert(context->size);
			if ((context->heapMem = (c_address)a_memAlloc(context->size)) != (c_address)NULL){
				HERE("malloc succesful");
				int cpyMemResult;
				a_utlStopWatchStart(context->utlContext);
				cpyMemResult = a_memCopyMem((void *)context->heapMem, (void *)context->address, context->size);
				a_utlStopWatchStop(context->utlContext);
				context->timerResults->shmCopyMicroSecs = a_utlGetStopWatchTimeMicroSecs(context->utlContext);
				context->timerResults->shmCopyMilSecs = a_utlGetStopWatchTimeMilSecs(context->utlContext);
				if (cpyMemResult) {
					HERE("mem cloned on heap");
					// detach (original) shm
					if ( a_shmDetach(origShmContext) ) {
						HERE("original shm detached");
						//printf("[a_apiPrepare] startAddress = 0x%X, size = 0x%X (%ld)\n",
						//	(unsigned int)startAddress, (unsigned int)size, size);
					} else {
						context->error = A_ERR_SHM_DETACH_FAIL;
						HERE("?could not detach original shm");
					}
				} else {
					context->error = A_ERR_MEMCOPY_FAIL;
					HERE("?copy from heap to shm failed");
				}
			} else {
				HERE("?malloc failed");
				context->error = A_ERR_MALLOC_FAIL;
			}
		} else {
			context->error = A_ERR_BASE_OPEN_FAIL;
			HERE("?base not opened");
		}
		a_shmDeInit(origShmContext);
		HERE("original shm deinitialised");
	} else {
		HERE("?could not attach to original shm");
		context->error = A_ERR_SHM_ATTACH_FAIL;
	}
	return (context->error == A_ERR_OK) ? 1 : 0;
}



/* Part of a_apiPrepare:
 * Create a new Shared Memory Segment, with a new name, that
 * will not interfere with the original shared memory.
 */
static int
a_apiPrepareCreateOwnShm(
	a_apiContext context)
{
	FUNC("a_apiPrepareCreateOwnShm");
	context->error = A_ERR_OK;
	
	a_shmAppShmName(context->shmContext, A_API_SHM_NAME_APPEND);
	HERE("shm name set");
					
	assert(context->address);
	if (a_shmCreateShm(context->shmContext, context->address, context->size) ) {
		HERE("shm created");
				
		if (a_shmAttach(context->shmContext) ) {
			HERE("shm attached");
					
			if ( a_memCopyMem((void *)context->address, (void *)context->heapMem, context->size) ) {
				HERE("heap copied to our own shm");
			} else {
				context->error = A_ERR_MEMCOPY_FAIL;
				HERE("copy from heap to our own shm failed");
			}
		} else {
			context->error = A_ERR_SHM_ATTACH_FAIL;
			HERE("could not attach to shm");
		}
	} else {
		context->error = A_ERR_SHM_CREATE_FAIL;
		HERE("could not create our own shm");
	}
	return (context->error == A_ERR_OK) ? 1 : 0;
}



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
 * Boolean setting for specifying you wish to load the shared memory
 * portion from a file (specified by \a memFname) instead of copying
 * from a currently existing shared memory segment. If the file does
 * not exist, this function will fail. If the file you are trying to
 * load with this operation was not created by this same function,
 * the outcome is undetermined. If \a memFname is NULL, this
 * parameter will be ignored.
 *
 * \return
 * Boolean value specifying this operation's success (1) or failure
 * (0). In case of failure, the internal \a errorno will be set.
 *
 * \see
 * a_apiContext
 */
int
a_apiPrepare(
	a_apiContext context,
	char *memFname,
	int loadInsteadOfSave)
{
    int result;
	FUNC("a_apiPrepare");
	HERE("start");
	assert(context);
	context->error = A_ERR_OK;

	if (memFname && loadInsteadOfSave) {
		HERE("want to load file into heap (instead of copy from shm)");
		result = a_apiPrepareLoadFile2Heap(context, memFname);
		if (!result) {
			context->error = A_ERR_LOAD_FROM_FILE_FAIL;
			HERE("failed to load file into heap");
		} else {
			HERE("file loaded into heap");
		}
	} else {
		HERE("want to copy from shm");
		result = a_apiPrepareCopyShm2Heap(context);
		if (!result) {
			HERE("failed to copy from shm to heap");
			context->error = A_ERR_SHM_COPY_TO_HEAP_FAIL;
		} else {
			HERE("shm copied to heap");
		}
	}
	
	if (result) {
		HERE("(we now have a copy on heap)");
		if (memFname && !loadInsteadOfSave) {
			HERE("want to save heap to file");
			result = a_apiPrepareSaveHeap2File(context, memFname);
			if (!result) {
				HERE("save heap to file failed");
				context->error = A_ERR_SAVE_TO_FILE_FAIL;
			} else {
				HERE("heap to file saved");
			}
		}
		if (result) {
			HERE("about to create our own shm");
			if ( a_apiPrepareCreateOwnShm(context) ) {
				HERE("own shm created");
				if ( a_bseOpenBase(context->bseContext, context->address) ) {
					HERE("base opened");
					a_lstSetNewOccurrencesArraySize(context->list,
						a_bseGetStateProperty(context->bseContext, A_BSE_STATE_MAXUSED));
					HERE("about to fill list");

//					a_apiPatchShm();
					
					a_utlStopWatchStart(context->utlContext);
					a_anlFillList(context->anlContext, a_bseGetBase(context->bseContext));
					a_utlStopWatchStop(context->utlContext);
					context->timerResults->listFillMilSecs = a_utlGetStopWatchTimeMilSecs(context->utlContext);
				} else {
					context->error = A_ERR_BASE_OPEN_FAIL;
					HERE("open base failed");
				}
			} else {
				context->error = A_ERR_SHM_CREATE_FAIL;
				HERE("creation of own shm failed");
			}
		}
	}
	if (context->filContext) {
		a_filDeInit(context->filContext);
	}
	result = context->error == A_ERR_OK ? 1 : 0;
	return result;
}




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
int
a_apiAnalyse(
	a_apiContext context)
{
	a_utlStopWatchStart(context->utlContext);
	a_anlComputeRefs(context->anlContext, a_bseGetBase(context->bseContext), context->address, context->size);
	a_utlStopWatchStop(context->utlContext);
	context->timerResults->analyseMilSecs = a_utlGetStopWatchTimeMilSecs(context->utlContext);
	return 1;
}



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
 * a_apiHexdumpWiped
 */
int
a_apiHexdump(
	a_apiContext context,
	int width,
	int group)
{
	int result;
	if (a_shmIsAttached(context->shmContext)) {
		unsigned int *startAddrPtr = (unsigned int *)a_shmGetShmAddress(context->shmContext);
		a_hexDump(context->out, startAddrPtr, context->size, width, group);
		result = 1;
	} else {
		context->error = A_ERR_SHM_NOT_ATTACHED;
		result = 0;
	}
	return result;
}


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
int
a_apiHexdumpWiped(
	a_apiContext context,
	int width,
	int group)
{
	int result;
	if (a_shmIsAttached(context->shmContext)) {
		result = a_anlWipeObjects(context->anlContext);
		if (result) {
			result = a_apiHexdump(context, width, group);
		} else {
			result = 0;
		}
	} else {
		context->error = A_ERR_SHM_NOT_ATTACHED;
		result = 0;
	}
	return result;
}



/***********************************************************
 WALK FUNCTIONS
 ***********************************************************/

/* Structure aid for the ListWalk Callbacks
 */
typedef struct a_apiCallbackContext {
	a_apiWalkAction userAction;
	void *userArg;
	a_apiListTotals listTotals;
} *a_apiCallbackContext;



/* Creates an apiObject and copies data from the lstObject
 * into it. Returns a pointer to this new object, or NULL
 * if the malloc failed.
 * This copy construction is required for keeping lstObject
 * (and a_lst for that matter) hidden to the outside.
 * Remember to free (using a_memfree()) the apiObject after
 * use.
 */
static a_apiObject
a_apiLstObjectToApiObject(
	a_lstObject lstObject)
{
	a_apiObject apiObject = a_memAlloc(sizeof(struct a_apiObject));
	if (apiObject) {
		apiObject->objectAddress    = lstObject->address;
		apiObject->referenceCount   = lstObject->refCount;
		apiObject->alignment        = lstObject->alignment;
		apiObject->size             = lstObject->size;
		apiObject->ourSize          = lstObject->ourSize;
		apiObject->objectName       = a_memStrdup(lstObject->objectName);
		apiObject->typeName         = a_memStrdup(lstObject->typeName);
		apiObject->typeDesc         = a_memStrdup(lstObject->typeDesc);
		apiObject->value            = a_memStrdup(lstObject->value);
		apiObject->note             = a_memStrdup(lstObject->note);
		apiObject->occurrencesCount = lstObject->occurrencesCount;
		apiObject->typeRefsCount    = lstObject->typeRefsCount;
		apiObject->dataRefsCount    = lstObject->dataRefsCount;
		apiObject->unknRefsCount    = lstObject->unknRefsCount;
		apiObject->refsDifference   = lstObject->refsDifference;
		apiObject->refsToTypeCount  = lstObject->refsToTypeCount;
		apiObject->refsToDataCount  = lstObject->refsToDataCount;
		apiObject->refsToUnknCount  = lstObject->refsToUnknCount;
		apiObject->occurrenceDiff   = lstObject->occurrenceDiff;
		apiObject->objectKind       = a_lstGetObjectKindChar(lstObject->kind);
	}
	return apiObject;
}



/* General Callback function for both the ListWalk and
 * EntryQuery. The function to eventually call back to
 * is encapsulated in the callbackContext.
 */
static int
a_apiListWalkCallback(
	void *valuePtr,
	struct a_apiCallbackContext *callbackContext)
{
	int result;
	a_lstObject lstObject = (a_lstObject)valuePtr;
	a_apiObject apiObject = a_apiLstObjectToApiObject(lstObject);
	if (apiObject) {
		result = (callbackContext->userAction)(apiObject, callbackContext->userArg);
		if (result) {
			callbackContext->listTotals->objs++;
			callbackContext->listTotals->refC  += apiObject->referenceCount;
			callbackContext->listTotals->tRef  += apiObject->typeRefsCount;
			callbackContext->listTotals->dRef  += apiObject->dataRefsCount;
			callbackContext->listTotals->uRef  += apiObject->unknRefsCount;
			callbackContext->listTotals->diff  += apiObject->refsDifference;
			callbackContext->listTotals->occrs += apiObject->occurrencesCount;
			callbackContext->listTotals->odiff += apiObject->occurrenceDiff;
		}
		a_memFree(apiObject);
	} else {
		result = 0;
	}
	return result;
}



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
 * a_apiContext a_apiWalkAction
 */
int
a_apiListWalk(
	a_apiContext context,
	a_apiWalkAction walkAction,
	void *walkActionArg)
{
	int result;
	struct a_apiCallbackContext callbackContext;
	callbackContext.userAction = walkAction;
	callbackContext.userArg = walkActionArg;
	callbackContext.listTotals = context->listTotals;
	result = a_lstListWalk(context->list, (a_lstWalkAction)a_apiListWalkCallback, &callbackContext);
	return result;
}


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
int
a_apiAddressQuery(
	a_apiContext context,
	long address,
	a_apiWalkAction walkAction,
	void *walkActionArg)
{
	int result;
	struct a_apiCallbackContext callbackContext;

	callbackContext.userAction = walkAction;
	callbackContext.userArg = walkActionArg;
	callbackContext.listTotals = context->listTotals;
	result = a_lstEntryQuery(context->list, address, (a_lstWalkAction)a_apiListWalkCallback, &callbackContext);
	return result;
}



/* Structure aid for the Walk Callbacks
 */
typedef struct a_apiAddrRefCallbackContext {
	a_apiAddressWalkAction userAction;
	void *userArg;
	a_apiRefsKind refsKind;
} *a_apiAddrRefCallbackContext;



/* Converts an a_apiRefsKind to an a_lstRefsKind
 */
static a_lstRefsKind
a_apiApiToLstRefsKind(
	a_apiRefsKind apiRefsKind)
{
	a_lstRefsKind result;
	switch (apiRefsKind) {
		case A_REF_TYPEREF:     result = L_REF_TYPEREF;     break;
		case A_REF_DATAREF:     result = L_REF_DATAREF;     break;
		case A_REF_UNKNREF:     result = L_REF_UNKNREF;     break;
		case A_REF_REFTOTYPE:   result = L_REF_REFTOTYPE;   break;
		case A_REF_REFTODATA:   result = L_REF_REFTODATA;   break;
		case A_REF_REFTOUNKN:   result = L_REF_REFTOUNKN;   break;
		case A_REF_OCCURRENCES: result = L_REF_OCCURRENCES; break;
		default:                result = L_REF_UNDEFINED;   break;
	}
	return result;
}


#if 0
/* Converts an a_lstRefsKind to an a_apiRefsKind
 */
static a_apiRefsKind
a_apiLstToApiRefsKind(
	a_lstRefsKind lstRefsKind)
{
	a_apiRefsKind result;
	switch (lstRefsKind) {
		case L_REF_TYPEREF:     result = A_REF_TYPEREF;     break;
		case L_REF_DATAREF:     result = A_REF_DATAREF;     break;
		case L_REF_UNKNREF:     result = A_REF_UNKNREF;     break;
		case L_REF_REFTOTYPE:   result = A_REF_REFTOTYPE;   break;
		case L_REF_REFTODATA:   result = A_REF_REFTODATA;   break;
		case L_REF_REFTOUNKN:   result = A_REF_REFTOUNKN;   break;
		case L_REF_OCCURRENCES: result = A_REF_OCCURRENCES; break;
		default:                result = A_REF_UNDEFINED;   break;
	}
	return result;
}
#endif


/* General Callback function for all RefsWalks.
 * The function to eventually call back to
 * is encapsulated in the callbackContext.
 */
static int
a_apiAddressCallback(
	long address,
	struct a_apiAddrRefCallbackContext *callbackContext)
{
	int result = (callbackContext->userAction)(address, callbackContext->userArg, callbackContext->refsKind);
	return result;
}



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
int
a_apiAddressReferencesWalk(
	a_apiContext context,
	long address,
	a_apiAddressWalkAction walkAction,
	void *walkActionArg,
	a_apiRefsKind refsKind)
{
	int result = 0;
	if (context->list) {
		a_lstEntryRefsWalkAction refsWalkAction = (a_lstEntryRefsWalkAction)a_apiAddressCallback;
		struct a_apiAddrRefCallbackContext callbackContext;
		callbackContext.userAction = walkAction;
		callbackContext.userArg = walkActionArg;
		callbackContext.refsKind = refsKind;
		a_lstRefsKind lstRefsKind = a_apiApiToLstRefsKind(refsKind);
		result = a_lstEntryReferencesWalk(context->list, address, refsWalkAction, &callbackContext, lstRefsKind);
	}
	return result;
}



/***********************************************************
 INFORMATION GATHERING
 ***********************************************************/

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
int
a_apiPushMeListTotals(
	a_apiContext context,
	a_apiListTotalsAction action,
	void *actionArg)
{
	int result = 0;
	a_apiListTotals userListTotals = a_apiCreateListTotals();
	if (userListTotals) {
		a_memCopyMem((void *)userListTotals, (void *)context->listTotals, sizeof(struct a_apiListTotals));
		result = (action)(userListTotals, actionArg);
		a_memFree(userListTotals);
	}
	return result;
}



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
int
a_apiPushMeTimerResults(
	a_apiContext context,
	a_apiTimerResultsAction action,
	void *actionArg)
{
	int result = 0;
	a_apiTimerResults userTimerResults = a_apiCreateTimerResults();
	if (userTimerResults) {
		a_memCopyMem((void *)userTimerResults, (void *)context->timerResults, sizeof(struct a_apiTimerResults));
		result = (action)(userTimerResults, actionArg);
		a_memFree(userTimerResults);
	}
	return result;
}



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
int
a_apiPushMeInfoData(
	a_apiContext context,
	a_apiInfoDataAction action,
	void *actionArg)
{
	int result;
	if (context) {
		a_apiInfoData infoData = a_memAlloc(sizeof(struct a_apiInfoData_s));
		if (infoData) {
			infoData->shmName    = a_memStrdup(a_shmGetShmName(context->shmContext));
			infoData->dbName     = a_memStrdup(a_bseGetBaseName(context->bseContext));
			infoData->shmAddress = (long)a_shmGetShmAddress(context->shmContext);
			infoData->shmSize    = 0;
			infoData->bseAddress = (long)a_bseGetBaseAddr(context->bseContext);
			infoData->mmSize     = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_SIZE);
			infoData->mmUsed     = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_USED);
			infoData->mmMaxUsed  = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_MAXUSED);
			infoData->mmFails    = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_FAILS);
			infoData->mmGarbage  = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_GARBAGE);
			infoData->mmCount    = a_bseGetStateProperty(context->bseContext, A_BSE_STATE_COUNT);
			infoData->clssObjs   = a_lstCount(context->list, L_CLSS);
			infoData->baseObjs   = a_lstCount(context->list, L_BASE);
			infoData->metaObjs   = a_lstCount(context->list, L_META);
			infoData->dataObjs   = a_lstCount(context->list, L_DATA);
			infoData->undfObjs   = a_lstCount(context->list, L_UNDF);
			infoData->dataSize   = a_anlTotalDataSize(context->anlContext);
			result = (action)(infoData, actionArg);
			a_memFree(infoData->shmName);
			a_memFree(infoData->dbName);
			a_memFree(infoData);
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}


/***********************************************************
 FREE SPACE STATS
 ***********************************************************/

/**
 * \brief
 * Local context helper for a_apiFreeSpaceStatsWalk
 *
 * \see
 * a_apiFreeSpaceStatsWalk
 */
typedef struct a_apiFreeSpaceStatsWalkContext {
	a_apiFreeSpaceStatsWalkAction walkAction;
	void *arg;
} *a_apiFreeSpaceStatsWalkContext;


/**
 * \brief
 * Call back function for a_apiFreeSpaceStatsWalk
 *
 * \see
 * a_apiFreeSpaceStatsWalk
 */
static int
a_apiFreeSpaceStatsWalkCallback(
	c_long freeSpaceSize,
	c_long count,
	a_apiFreeSpaceStatsWalkContext walkContext)
{
	a_apiFreeSpaceStatsWalkAction walkAction = walkContext->walkAction;
	return (walkAction)((long)freeSpaceSize, (long)count, walkContext->arg);
}


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
int
a_apiFreeSpaceStatsWalk(
	a_apiContext context,
	a_apiFreeSpaceStatsWalkAction walkAction,
	void *arg)
{
	int result;
	if (context) {
		struct a_apiFreeSpaceStatsWalkContext walkContext;
		walkContext.walkAction = walkAction;
		walkContext.arg = arg;
		a_stsFreeSpaceCountersWalkAction action = (a_stsFreeSpaceCountersWalkAction)a_apiFreeSpaceStatsWalkCallback;
		result = a_stsFreeSpaceCountersWalk(context->stsContext, action, &walkContext);
	} else {
		result = 0;
	}
	return result;
}


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
int
a_apiFreeSpaceStatsCount(
	a_apiContext context)
{
	return context ? a_stsFreeSpaceCountersCount(context->stsContext) : -1;
}


//END a_api.c
