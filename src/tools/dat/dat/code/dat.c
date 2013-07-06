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


#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "d_utl.h"
#include "a_api.h"


// Title:
#define APPL_TITLE           "SPLICE-DDS Database Analysis Tool"

// Defaults:
#define D_HEXDUMP_FILENAME   "dat.out"
#define D_MEMORY_FILENAME    "dat.mem"
#define D_HEXDUMP_WIDTH       32
#define D_HEXDUMP_GROUPWIDTH  8
#define D_SHMNAME_DEFAULT    "The default Domain"
#define D_DBNAME_DEFAULT     "c_base_A02"
#define D_KEYFILE_MASK       "spddskey_"

// Actions (commands):
#define A_UNDEFINED           0x0001
#define A_HELP                0x0002
#define A_INFO                0x0004

#define A_DUMP_UNDF           0x0010
#define A_DUMP_CLSS           0x0020
#define A_DUMP_BASE           0x0040
#define A_DUMP_META           0x0080
#define A_DUMP_DATA           0x0100
#define A_DUMP_ALL            0x0200
#define A_QUERY               0x0400
#define A_SHOW_STATS          0x0800

#define A_MEM_SAVE_TO_FILE    0x1000
#define A_HEXDUMP             0x2000
#define A_HEXDUMP_WIPED       0x4000
#define A_TEST                0x8000


// Options:
#define O_UNDEFINED           0x0001
#define O_MEM_LOAD_FROM_FILE  0x0002
#define O_OUTPUT_TO_FILE      0x0004

#define O_SHOW_ANALYSE_OUTPUT 0x0010
#define O_SHOW_VERBOSE_OUTPUT 0x0020

#define O_HEXDUMP_WIDTH       0x0100
#define O_HEXDUMP_GROUPWIDTH  0x0200
#define O_SHOW_RELATIONS      0x0400
#define O_SHOW_OCCURRENCES    0x0800

#define O_NO_HEADER           0x1000



/**
 * \brief
 * Internal Data Structure holding the List Totals
 */
typedef struct datListTotals {
	long objs;                            ///< Number of counted objects
	long refC;                            ///< Sum of all Reference Counts
	long tRef;                            ///< Sum of all Type References
	long dRef;                            ///< Sum of all Data References
	long uRef;                            ///< Sum of all Unknwon References
	long refT;                            ///< Sum of all References as Type
	long refD;                            ///< Sum of all References as Data
	long refU;                            ///< Sum of all References as Unknown
	long diff;                            ///<
	long nDiffs;                          ///< Sum of all negative diff values
	long pDiffs;                          ///< Sum of all positive diff values
	long occrs;                           ///< Sum of all occurrences
	long odiff;                           ///<
} *datListTotals;


/**
 * \brief
 * Context Data Structure for DAT
 */
typedef struct datContext_s {
	int             actions;              ///< Actions to perform (bitwise data)
	int             options;              ///< Options specified (bitwise data)
	int             hexdumpWidth;         ///<
	int             hexdumpGroupwidth;    ///<
	long            queryAddress;         ///<
	char           *memoryFilename;       ///<
	char           *hexdumpFilename;      ///< (not used yet)
	char           *shmName;              ///< Shared Memory Name (Domain Name)
	char           *dbName;               ///< Database Name
	FILE           *out;                  ///< File Pointer for normal output (stdout)
	FILE           *err;                  ///< File Pointer for error messages (stderr)
	a_apiContext    apiContext;           ///< Pointer to AAPI Context
	datListTotals   listTotals;           ///< DAT's computed List Totals
} *datContext;



/****************************************************************
 DISPLAYING USAGE ETC.
 ****************************************************************/


#define DAT_DUMP_HELPENTRY(option, description) printf("    -%-11s %s\n", option, description)


/**
 * \brief
 * Displays a full usage screen (on stdout)
 *
 * \param applName
 * Name of this application as it is used on the command line.
 * typically argv[0] from main().
 */
void
datDisplayHelp(
	char *applName)
{
	printf("\n");
	printf("Usage: ");
	printf("%s {command} [{option}] <dbName> <shmName>\n", applName);
	printf("\n");
	printf("  Commands:\n");
	DAT_DUMP_HELPENTRY("i",           "information (brief) about current allocated shared memory");
	DAT_DUMP_HELPENTRY("x",           "shared memory hexdump");
	DAT_DUMP_HELPENTRY("y",           "shared memory hexdump after wiping all objects");
	DAT_DUMP_HELPENTRY("c",           "dump of c_class objects (meta-meta-metadata)");
	DAT_DUMP_HELPENTRY("b",           "dump of base objects (meta-metadata)");
	DAT_DUMP_HELPENTRY("m",           "dump of metadata");
	DAT_DUMP_HELPENTRY("d",           "dump of data");
	DAT_DUMP_HELPENTRY("u",           "dump of uncategorised objects");
	DAT_DUMP_HELPENTRY("a",           "dump of all objects (same as -cbmdu)");
	DAT_DUMP_HELPENTRY("q<hex_addr>", "query one object address");
	DAT_DUMP_HELPENTRY("f",           "free space statistics");
	DAT_DUMP_HELPENTRY("s<fname>",    "save memory to file");
	DAT_DUMP_HELPENTRY("h",           "this help");
	printf("\n");
	printf("  Options:\n");
	DAT_DUMP_HELPENTRY("l<fname>",    "load memory from file (instead of copy from shared memory)");
	DAT_DUMP_HELPENTRY("w<n>",        "alternate memory dump column width");
	DAT_DUMP_HELPENTRY("g<n>",        "alternate memory dump grouping width");
	DAT_DUMP_HELPENTRY("n",           "no header or totals with dump");
	DAT_DUMP_HELPENTRY("r",           "show object relations with dump or query");
	DAT_DUMP_HELPENTRY("o",           "show pointer references to objects, scanned from raw data");
	DAT_DUMP_HELPENTRY("e",           "show analyse output (debug mode)");
	DAT_DUMP_HELPENTRY("v",           "verbose aapi output");
	printf("\n");
//	printf("(*) Not implemented (yet)\n");
}

#undef DAT_DUMP_HELPENTRY


/**
 * \brief
 * Displays a short usage message (on stdout)
 *
 * \param applName
 * Name of this application as it is used on the command line.
 * typically argv[0] from main().
 */
void
datDisplayHelpShort(
	char *applName)
{
	fprintf(stdout, "Usage: %s -hixycbmduafnroev -q<hex_addr> -s<fname> -l<fname> -w<n> -g<n> <db name> <shm name>\n", applName);
	fprintf(stdout, "Try '%s -h' for help\n", applName);
}


/**
 * \brief
 * Displays a this Application's Title (on stdout)
 */
void
datDisplayTitle()
{
	printf("%s\n", APPL_TITLE);
}



/****************************************************************
 CREATE INTERNAL VARIABELES, CONTEXT, ETC.
 ****************************************************************/


/* Creates and returns a new instance of datListTotals
 */
datListTotals
datCreateListTotals()
{
	datListTotals listTotals = malloc(sizeof(struct datListTotals));
	if (listTotals) {
		listTotals->objs   = 0;
		listTotals->refC   = 0;
		listTotals->tRef   = 0;
		listTotals->dRef   = 0;
		listTotals->uRef   = 0;
		listTotals->refT   = 0;
		listTotals->refD   = 0;
		listTotals->refU   = 0;
		listTotals->diff   = 0;
		listTotals->nDiffs = 0;
		listTotals->pDiffs = 0;
		listTotals->occrs  = 0;
		listTotals->odiff  = 0;
	}
	return listTotals;
}



/* Destroys an instance of datListTotals, freeing up memory
 */
void
datDestroyListTotals(
	datListTotals listTotals)
{
	free(listTotals);
}



/* Initialize some things
 */
datContext
datInit()
{
	datContext context = malloc(sizeof(struct datContext_s));
	context->actions           = 0;
	context->options           = 0;
	context->hexdumpWidth      = D_HEXDUMP_WIDTH;
	context->hexdumpGroupwidth = D_HEXDUMP_GROUPWIDTH;
	context->memoryFilename    = os_strdup(D_MEMORY_FILENAME);
	context->hexdumpFilename   = os_strdup(D_HEXDUMP_FILENAME);
	context->shmName           = os_strdup(D_SHMNAME_DEFAULT);
	context->dbName            = os_strdup(D_DBNAME_DEFAULT);
	context->out               = stdout;
	context->err               = stderr;
	context->apiContext        = NULL;
	context->listTotals        = datCreateListTotals();
	return context;
}



/* Deinitialises the context
 */
void
datDeInit(
	datContext context)
{
	if (context) {
		if (context->memoryFilename) {
			free(context->memoryFilename);
		}
		if (context->hexdumpFilename) {
			free(context->hexdumpFilename);
		}
		if (context->shmName) {
			free(context->shmName);
		}
		if (context->dbName) {
			free(context->dbName);
		}
		if (context->apiContext) {
			a_apiDeInit(context->apiContext);
			context->apiContext = NULL;
		}
		datDestroyListTotals(context->listTotals);
		free(context);
	}
}



/* Displays all recognised command line *commands*
 */
void
datTestActions(
	datContext context)
{
	if (context->actions & A_HELP)                printf("-h ");
	if (context->actions & A_INFO)                printf("-i ");
	if (context->actions & A_HEXDUMP)             printf("-x ");
	if (context->actions & A_HEXDUMP_WIPED)       printf("-y ");
	if (context->actions & A_DUMP_UNDF)           printf("-u ");
	if (context->actions & A_DUMP_CLSS)           printf("-c ");
	if (context->actions & A_DUMP_BASE)           printf("-b ");
	if (context->actions & A_DUMP_META)           printf("-m ");
	if (context->actions & A_DUMP_DATA)           printf("-d ");
	if (context->actions & A_DUMP_ALL)            printf("-a ");
	if (context->actions & A_QUERY)               printf("-q%8.8X ", (unsigned int)context->queryAddress);
	if (context->actions & A_SHOW_STATS)          printf("-f ");
	if (context->actions & A_MEM_SAVE_TO_FILE)    printf("-s%s ", context->memoryFilename);
	printf("\n");
}



/* Displays all recognised command line *options*
 */
void
datTestOptions(
	datContext context)
{
	if (context->options & O_MEM_LOAD_FROM_FILE)  printf("-l%s ", context->memoryFilename);
	if (context->options & O_OUTPUT_TO_FILE)      printf("-f ");
	if (context->options & O_HEXDUMP_WIDTH)       printf("-w%d ", context->hexdumpWidth);
	if (context->options & O_HEXDUMP_GROUPWIDTH)  printf("-g%d ", context->hexdumpGroupwidth);
	if (context->options & O_NO_HEADER)           printf("-n ");
	if (context->options & O_SHOW_RELATIONS)      printf("-r ");
	if (context->options & O_SHOW_OCCURRENCES)    printf("-o ");
	if (context->options & O_SHOW_ANALYSE_OUTPUT) printf("-e ");
	if (context->options & O_SHOW_VERBOSE_OUTPUT) printf("-v ");
	printf("\n");
}




/* datTest:
 * Shows current DAT Context vars.
 * (No AAPI Actions will be taken.)
 */
void
datTest(
	datContext context)
{
	datDisplayTitle();
#ifdef __linux
	printf("(Linux version)\n");
#else
	printf("(UNIX version)\n");
#endif
	printf("(Compiled: %s %s)\n", __DATE__, __TIME__);
	printf("\n");
	printf("%-20s : 0x%4.4X  ", "Commands specified",  context->actions);
	datTestActions(context);
	printf("%-20s : 0x%4.4X  ", "Options specified",   context->options);
	datTestOptions(context);
//	printf("%-20s : %s\n",  "Output File Name",    context->hexdumpFilename);
	printf("%-20s : %s\n",  "Shared Memory Name",  context->shmName);
	printf("%-20s : %s\n",  "Database Name",       context->dbName);
}




/****************************************************************
 SETTING THINGS
 ****************************************************************/


void
datSetMemFname(
	datContext context,
	char *newFname)
{
	if (context) {
		if (context->memoryFilename) {
			free(context->memoryFilename);
		}
		context->memoryFilename = os_strdup(newFname);
	}
}



void
datSetOutFname(
	datContext context,
	char *newFname)
{
	if (context) {
		if (context->hexdumpFilename) {
			free(context->hexdumpFilename);
		}
	}
}



void
datSetShmName(
	datContext context,
	char *newShmName)
{
	if (context) {
		if (context->shmName) {
			free(context->shmName);
		}
		context->shmName = os_strdup(newShmName);
	}
}




void
datSetDbName(
	datContext context,
	char *newDbName)
{
	if (context) {
		if (context->dbName) {
			free(context->dbName);
		}
		context->dbName = os_strdup(newDbName);
	}
}





/*
 * Parse the Command Line
 * Do not compute exclusions yet.
 */
void
datParseCmdline(
	int argc,
	char *argv[],
	datContext context)
{
	int c;
	extern char *optarg;
	extern int optind;
	extern int opterr;
	opterr = 0; // don't let getopt write to stderr
	while ((c = getopt(argc, argv, "?hTJixyucbmdafevnroq:s:l:w:g:")) != EOF) {
		switch (c) {
			case '?':
			case 'h': context->actions |= A_HELP;                break;
			case 'J':
			case 'T': context->actions |= A_TEST;                break;  // undocumented  ;-)
			case 'i': context->actions |= A_INFO;                break;
			case 'x': context->actions |= A_HEXDUMP;             break;
			case 'y': context->actions |= A_HEXDUMP_WIPED;       break;
			case 'u': context->actions |= A_DUMP_UNDF;           break;
			case 'c': context->actions |= A_DUMP_CLSS;           break;
			case 'b': context->actions |= A_DUMP_BASE;           break;
			case 'm': context->actions |= A_DUMP_META;           break;
			case 'd': context->actions |= A_DUMP_DATA;           break;
			case 'a': context->actions |= A_DUMP_ALL;            break;
			case 'f': context->actions |= A_SHOW_STATS;          break;
			case 'n': context->options |= O_NO_HEADER;           break;
			case 'r': context->options |= O_SHOW_RELATIONS;      break;
			case 'o': context->options |= O_SHOW_OCCURRENCES;    break;
			case 'e': context->options |= O_SHOW_ANALYSE_OUTPUT; break;
			case 'v': context->options |= O_SHOW_VERBOSE_OUTPUT; break;

			case 's':
				context->actions |= A_MEM_SAVE_TO_FILE;
				if (strlen(optarg)) {
					datSetMemFname(context, optarg);
				} else {
					context->actions &= !A_MEM_SAVE_TO_FILE;
				}
				break;

			case 'q':
				context->actions |= A_QUERY;
				if (strlen(optarg)) {
					unsigned int queryAddress;
					sscanf(optarg, "%x", &queryAddress);
					context->queryAddress = (long)queryAddress;
				} else {
					context->actions &= !A_QUERY;
				}
				break;

			case 'l':
				context->options |= O_MEM_LOAD_FROM_FILE;
				if (strlen(optarg)) {
					datSetMemFname(context, optarg);
				} else {
					context->options &= !O_MEM_LOAD_FROM_FILE;
				}
				break;

			case 'w':
				context->options |= O_HEXDUMP_WIDTH;
				if (strlen(optarg)) {
					context->hexdumpWidth = atoi(optarg);
				} else {
					context->options &= !O_HEXDUMP_WIDTH;
				}
				break;

			case 'g':
				context->options |= O_HEXDUMP_GROUPWIDTH;
				if (strlen(optarg)) {
					context->hexdumpGroupwidth = atoi(optarg);
				} else {
					context->options &= !O_HEXDUMP_GROUPWIDTH;
				}
				break;

			default:
				fprintf(stderr, "Ignoring unrecognised option '%c'\n", c);
				break;
		}
	}
	if (optind < argc) {
		datSetDbName(context, argv[optind]);
	}
	if (optind + 1 <= argc) {
		datSetShmName(context, argv[optind + 1]);
	}

}




int
datCheckExclusions(
	datContext context)
{
	int result = 1;

	// prohibit loading and saving in one run:
	if ((context->actions & A_MEM_SAVE_TO_FILE) && (context->options & O_MEM_LOAD_FROM_FILE)) {
		fprintf(stderr, "Cannot use -l and -s at the same time. Aborting...\n");
		result = 0;
	}

	// detect if no commands have been specified:
	if (!context->actions) {
		context->actions |= A_UNDEFINED;
	}

	// -a implies -u -c -b -m -d:
	if (context->actions & A_DUMP_ALL) {
		context->actions |= (A_DUMP_UNDF | A_DUMP_CLSS | A_DUMP_BASE | A_DUMP_META | A_DUMP_DATA);
	}

	return result;
}



/****************************************************************
 LIST OUTPUT - Utility Functions
 ****************************************************************/


/* Returns whether one of the dump commands was specified
 */
int
datDumpRequested(
	datContext context)
{
	return (context->actions & (A_DUMP_UNDF | A_DUMP_CLSS |  A_DUMP_BASE | A_DUMP_META | A_DUMP_DATA | A_QUERY) ) ? 1 : 0;
}



/* Returns whether we want to dump this object entry, specified
 * by its objectKind Character, checked against the datContext.
 */
int
datDumpThisEntry(
	datContext context,
	char *objectKind)
{
	int result;
	if (context->actions & A_DUMP_ALL) {
		result = 1;
	} else if (objectKind) {
		if (strlen(objectKind)) {
			if (strcmp(objectKind, "?") == 0) {  // match!
				result = (context->actions & A_DUMP_UNDF);
			} else if (strcmp(objectKind, "C") == 0) {
				result = (context->actions & A_DUMP_CLSS);
			} else if (strcmp(objectKind, "B") == 0) {
				result = (context->actions & A_DUMP_BASE);
			} else if (strcmp(objectKind, "M") == 0) {
				result = (context->actions & A_DUMP_META);
			} else if (strcmp(objectKind, "D") == 0) {
				result = (context->actions & A_DUMP_DATA);
			}
		}
	} else {
		result = 0;
	}
	return result;
}



/****************************************************************
 LIST OUTPUT - Headers, Footers, Totals Results
 ****************************************************************/


void
datDumpDashes(
	FILE *out,
	int length,
	int doubleDash)
{
	fprintf(out, " ");
	while (length) {
		fprintf(out, "%1s", doubleDash ? "=" : "-");
		length--;
	}
}



void
datDumpDashedLine(
	FILE *out,
	int doubleDash)
{
	datDumpDashes(out, 10, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  1, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out,  5, doubleDash);
	datDumpDashes(out, 30, doubleDash);
	datDumpDashes(out, 25, doubleDash);
	datDumpDashes(out,  2, doubleDash);
	datDumpDashes(out,  3, doubleDash);
	datDumpDashes(out, 30, doubleDash);
	datDumpDashes(out, 20, doubleDash);
	fprintf(out, "\n");
}


void
datDumpLegend(
	FILE *out)
{
	fprintf(out, " %-10s", "Obj Addr");
	fprintf(out, " %5s",   "RefC");
	fprintf(out, " %1s",   "T");
	fprintf(out, " %5s",   "->T");
	fprintf(out, " %5s",   "->D");
	fprintf(out, " %5s",   "->U");
	fprintf(out, " %5s",   "Occrs");
	fprintf(out, " %5s",   "oDiff");
	fprintf(out, " %5s",   "T->");
	fprintf(out, " %5s",   "D->");
	fprintf(out, " %5s",   "U->");
	fprintf(out, " %5s",   "Diff");
	fprintf(out, " %-30s", "Object Name");
	fprintf(out, " %-25s", "Type Name");
	fprintf(out, " %2s",   "Al");
	fprintf(out, " %3s",   "Sze");
	fprintf(out, " %-30s", "Value");
	fprintf(out, " %-20s", "Note");
	fprintf(out, "\n");
}


void
datDumpHead(
	FILE *out)
{
	datDumpLegend(out);
	datDumpDashedLine(out, 0);
}



void
datDumpFoot(
	FILE *out)
{
	datDumpDashedLine(out, 0);
	datDumpLegend(out);
}



float
datDumpCalculateScore(
	datListTotals listTotals)
{
	float result;
	if (listTotals) {
		long oDiff = 0 < listTotals->odiff ? listTotals->odiff : -listTotals->odiff;
		result = ((float)listTotals->occrs / (float)(listTotals->occrs + oDiff)) * 100;
	} else {
		result = 0.0;
	}
	return result;
}




int
datDumpListTotals(
	datContext context)
{
	int result = 0;
	if (context->listTotals) {
		datDumpDashedLine(context->out, 1);
		fprintf(context->out, " %10ld", context->listTotals->objs);
		fprintf(context->out, "%6ld",   context->listTotals->refC);
		fprintf(context->out, " %1s",   "");
		fprintf(context->out, "%6ld",   context->listTotals->tRef);
		fprintf(context->out, "%6ld",   context->listTotals->dRef);
		fprintf(context->out, "%6ld",   context->listTotals->uRef);
		fprintf(context->out, "%6ld",   context->listTotals->occrs);
		fprintf(context->out, "%6ld",   context->listTotals->odiff);
		fprintf(context->out, "%6ld",   context->listTotals->refT);
		fprintf(context->out, "%6ld",   context->listTotals->refD);
		fprintf(context->out, "%6ld",   context->listTotals->refU);
		fprintf(context->out, "%6ld/%ld", context->listTotals->nDiffs, context->listTotals->pDiffs);
		fprintf(context->out, " %s", "<--Totals");
		// fprintf(context->out, " (Resolved: %2.2f%%)", datDumpCalculateScore(context->listTotals));
		result++;
	}
	return result;
}


/****************************************************************
 LIST OUTPUT
 ****************************************************************/


/* Outputs one apiObject to stdout.
 */
void
datDumpApiObject(
	FILE *out,
	a_apiObject apiObject)
{
	fprintf(out, " 0x%8.8X", (unsigned int)apiObject->objectAddress);
	fprintf(out, " %5ld",    apiObject->referenceCount);
	fprintf(out, " %1s",     apiObject->objectKind ? apiObject->objectKind : " ");

	if (apiObject->typeRefsCount) {
		fprintf(out, " %5ld", apiObject->typeRefsCount);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->dataRefsCount) {
		fprintf(out, " %5ld", apiObject->dataRefsCount);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->unknRefsCount) {
		fprintf(out, " %5ld", apiObject->unknRefsCount);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->occurrencesCount) {
		fprintf(out, ":%5ld", apiObject->occurrencesCount);
	} else {
		fprintf(out, ":%5s",  "");
	}
	if (apiObject->occurrenceDiff) {
		fprintf(out, " %5ld", apiObject->occurrenceDiff);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->refsToTypeCount) {
		fprintf(out, ":%5ld", apiObject->refsToTypeCount);
	} else {
		fprintf(out, ":%5s",  "");
	}

	if (apiObject->refsToDataCount) {
		fprintf(out, " %5ld", apiObject->refsToDataCount);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->refsToUnknCount) {
		fprintf(out, " %5ld", apiObject->refsToUnknCount);
	} else {
		fprintf(out, " %5s",  "");
	}

	if (apiObject->refsDifference) {
		fprintf(out, ":%5ld", apiObject->refsDifference);
	} else {
		fprintf(out, ":%5s",  "");
	}

	fprintf(out, " %-30s",   apiObject->objectName ? apiObject->objectName : "");
	fprintf(out, " %-25s",   apiObject->typeName ? apiObject->typeName : "");
	fprintf(out, " %2ld",    apiObject->alignment);
	fprintf(out, " %3ld",    apiObject->size);
	fprintf(out, " %-30s",   apiObject->value ? apiObject->value : "");
	fprintf(out, " %s",      apiObject->note ? apiObject->note : "");
}




typedef struct datRefsCallbackContext {
	a_apiRefsKind refsKind;
	FILE *out;
} *datRefsCallbackContext;



/* Prints an address, with the appropriate prefix or suffix
 * (determined by refsType).
 * Used as part of a call back procedure.
 * Returns 0 if callbackContext == NULL, otherwise >0.
 */
int
datDumpRefsCallback(
	long address,
	struct datRefsCallbackContext *callbackContext)
{
	int result = 0;
	if (callbackContext) {
		FILE *out = callbackContext->out;
		switch (callbackContext->refsKind) {
			case A_REF_OCCURRENCES:    fprintf(out, "O:%8.8X\t",  (unsigned int)address); break;
			case A_REF_TYPEREF:        fprintf(out, "%8.8X->T\t", (unsigned int)address); break;
			case A_REF_DATAREF:        fprintf(out, "%8.8X->D\t", (unsigned int)address); break;
			case A_REF_UNKNREF:        fprintf(out, "%8.8X->U\t", (unsigned int)address); break;
			case A_REF_REFTOTYPE:      fprintf(out, "T->%8.8X\t", (unsigned int)address); break;
			case A_REF_REFTODATA:      fprintf(out, "D->%8.8X\t", (unsigned int)address); break;
			case A_REF_REFTOUNKN:      fprintf(out, "U->%8.8X\t", (unsigned int)address); break;
			default: break;
		}
		result++;
	}
	return result;
}




/* Starts a walk (from the AAPI) to return all References from the
 * specified (object) address by the specified references kind.
 */
int
datDumpRefs(
	datContext context,
	long address,
	a_apiRefsKind refsKind)
{
	int result = 0;
	if (context) {
		a_apiAddressWalkAction walkAction = (a_apiAddressWalkAction)datDumpRefsCallback;
		struct datRefsCallbackContext callbackContext;
		callbackContext.refsKind = refsKind;
		callbackContext.out = context->out;
		result = a_apiAddressReferencesWalk(context->apiContext, address, walkAction, &callbackContext, refsKind);
	}
	return result;
}



int
datIncCounters(
	datListTotals listTotals,
	a_apiObject apiObject)
{
	int result = 0;
	if (listTotals && apiObject) {
		const long diff  = apiObject->refsDifference;

//		const long diff  = apiObject->referenceCount - apiObject->typeRefsCount
//							- apiObject->dataRefsCount - apiObject->unknRefsCount;
//		const long odiff = occrs - apiObject->typeRefsCount - apiObject->dataRefsCount - apiObject->unknRefsCount;

		listTotals->objs++;
		listTotals->refC   += apiObject->referenceCount;

		listTotals->tRef   += apiObject->typeRefsCount;
		listTotals->dRef   += apiObject->dataRefsCount;
		listTotals->uRef   += apiObject->unknRefsCount;
		listTotals->occrs  += apiObject->occurrencesCount;
		listTotals->odiff  += apiObject->occurrenceDiff;

		listTotals->refT   += apiObject->refsToTypeCount;
		listTotals->refD   += apiObject->refsToDataCount;
		listTotals->refU   += apiObject->refsToUnknCount;

		listTotals->diff   += diff;
		if (diff < 0) {
			listTotals->nDiffs += diff;
		} else {
			listTotals->pDiffs += diff;
		}
		// listTotals->nDiffs += 0 < diff ? diff : -diff;
		result++;
	}
	return result;
}



/* Callback Function to display an entry from the list.
 * If displaying occurrences was requested, a new walk
 * for this (sub)list will be started.
 * If displaying all object relations for this entry
 * was requested, the corresponding walks for these
 * (sub)lists will be started.
 * This function will always return true (1), to not
 * to abort the walk.
 */
int
datDumpEntriesCallback(
	a_apiObject apiObject,
	void *contextPtr)
{
	int result = 1;  // always return TRUE to continue the walk
	datContext context = (datContext)contextPtr;
	if (datDumpThisEntry(context, apiObject->objectKind)) {
		if (context->options & O_SHOW_OCCURRENCES) {
			if (apiObject->occurrencesCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_OCCURRENCES);
				fprintf(context->out, "\n");
			}
		}
		if (context->options & O_SHOW_RELATIONS) {
			if (apiObject->typeRefsCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_TYPEREF);
			}
			if (apiObject->dataRefsCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_DATAREF);
			}
			if (apiObject->unknRefsCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_UNKNREF);
			}
			if ((apiObject->typeRefsCount) | (apiObject->dataRefsCount) | (apiObject->unknRefsCount)) {
				fprintf(context->out, "\n");
			}
		}

		datDumpApiObject(context->out, apiObject);
		fprintf(context->out, "\n");
		datIncCounters(context->listTotals, apiObject);

		if (context->options & O_SHOW_RELATIONS) {
			if (apiObject->refsToTypeCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_REFTOTYPE);
			}
			if (apiObject->refsToDataCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_REFTODATA);
			}
			if (apiObject->refsToUnknCount) {
				datDumpRefs(context, apiObject->objectAddress, A_REF_REFTOUNKN);
			}
			if (apiObject->refsToTypeCount | apiObject->refsToDataCount | apiObject->refsToUnknCount) {
				fprintf(context->out, "\n");
			}
		}

		if (context->options & (O_SHOW_RELATIONS | O_SHOW_OCCURRENCES)) {
			fprintf(context->out, "\n");
		}

	}
	return result;
}





int
datDumpTimerResultsCallback(
	a_apiTimerResults timerResults,
	datContext context)
{
	int result = 0;
	if (timerResults && context) {
		fprintf(context->out, " (Shm Copy: %2.2f msecs)",
			(float)timerResults->shmCopyMicroSecs / 1000);
		fprintf(context->out, " (List Fill + Analysis: %ld + %ld = %ld msecs)",
			timerResults->listFillMilSecs, timerResults->analyseMilSecs,
			timerResults->listFillMilSecs + timerResults->analyseMilSecs);
		fprintf(context->out, "\n");
		result++;
	}
	return result;
}



int
datDumpTimerResults(
	datContext context)
{
	int result;
	a_apiTimerResultsAction timerResultsAction = (a_apiTimerResultsAction)datDumpTimerResultsCallback;
	result = a_apiPushMeTimerResults(context->apiContext, timerResultsAction, context);
	return result;
}




/* Starts a Walk in the AAPI to return all entries from the list.
 * A header and footer will be printed as well.
 * Returns >0 if all went fine, 0 if the walk was aborted or
 * could not be started because the context was NULL, or some
 * other stupid program error.
 */
int
datDumpEntries(
	datContext context)
{
	int result;
	a_apiWalkAction walkAction = datDumpEntriesCallback;
	if (!(context->options & O_NO_HEADER)) {
		datDumpHead(context->out);
	}
	if (context->actions & (A_DUMP_UNDF | A_DUMP_CLSS | A_DUMP_BASE | A_DUMP_META | A_DUMP_DATA | A_DUMP_ALL)) {
		result = a_apiListWalk(context->apiContext, walkAction, (void *)context);
	} else {
		result = 1;
	}
	if ( result && (context->actions & A_QUERY)) {
		context->actions |= A_DUMP_ALL;
		result = a_apiAddressQuery(context->apiContext, context->queryAddress, walkAction, (void *)context);
	}

	if (!(context->options & O_NO_HEADER)) {
		datDumpFoot(context->out);
		datDumpListTotals(context);
		datDumpTimerResults(context);
	}
	return result;
}




/****************************************************************
 SHOW FREE SPACE STATS
 ****************************************************************/


static int
datDumpFreeSpaceStatsCallback(
	long freeSpaceSize,
	long count,
	void *dummyArg)
{
	printf("%16ld %5ld\n", freeSpaceSize, count);
	return 1;
}


int
datDumpFreeSpaceStats(
	datContext context)
{
	int result;
	if (context) {
		int freeSpaceCounters = a_apiFreeSpaceStatsCount(context->apiContext);
//		printf("\nfreeSpaceCounters:%d\n", freeSpaceCounters);
		if (freeSpaceCounters) {
			a_apiFreeSpaceStatsWalkAction walkAction = datDumpFreeSpaceStatsCallback;
			if (!(context->options & O_NO_HEADER)) {
				printf("%16s %5s\n", "Free Space Size", "Count");
				printf("%16s %5s\n", "---------------", "-----");
			}
			result = a_apiFreeSpaceStatsWalk(context->apiContext, walkAction, (void *)context);
		} else {
			result = 1;
		}
	} else {
		result = 0;
	}
	return result;
}







/****************************************************************
 BRIEF INFO
 ****************************************************************/


int
datBriefInfoCallback(
	a_apiInfoData infoData,
	datContext context)
{
	int result;
	FILE *out = context->out;
	if (infoData && out) {   // just to be sure, avoiding null pointers
		long totlObjects = infoData->undfObjs + infoData->clssObjs + infoData->baseObjs + infoData->metaObjs + infoData->dataObjs;
		long percUsedMem = infoData->mmSize ? ((infoData->dataSize * 100) / infoData->mmSize) : -1;

		fprintf(out, "%-30s: %s\n", "Shared Memory Name", infoData->shmName ? infoData->shmName : "?");
		fprintf(out, "%-30s: %s\n", "Database Name", infoData->dbName ? infoData->dbName : "?");
		fprintf(out, "%-30s: 0x%X\n", "Start Address Shared Memory", (unsigned int)infoData->shmAddress);
		fprintf(out, "%-30s: 0x%X\n", "Start Address Database", (unsigned int)infoData->bseAddress);

		fprintf(out, "%s\n", "mmState:");
		fprintf(out, "%-30s: %10ld (0x%8.8X)\n", "  Size", infoData->mmSize, (unsigned int)infoData->mmSize);
		fprintf(out, "%-30s: %10ld (0x%8.8X)\n", "  Used", infoData->mmUsed, (unsigned int)infoData->mmUsed);
		fprintf(out, "%-30s: %10ld (0x%8.8X)\n", "  MaxUsed", infoData->mmMaxUsed, (unsigned int)infoData->mmMaxUsed);
		fprintf(out, "%-30s: %10ld\n", "  Fails", infoData->mmFails);
		fprintf(out, "%-30s: %10ld\n", "  Garbage", infoData->mmGarbage);
		fprintf(out, "%-30s: %10ld\n", "  Count", infoData->mmCount);

		fprintf(out, "%s\n", "aapi computed:");
		fprintf(out, "%-30s: %10ld\n", "  #Clss Objects (C)", infoData->clssObjs);
		fprintf(out, "%-30s: %10ld\n", "  #Base Objects (B)", infoData->baseObjs);
		fprintf(out, "%-30s: %10ld\n", "  #Meta Objects (M)", infoData->metaObjs);
		fprintf(out, "%-30s: %10ld\n", "  #Data Objects (D)", infoData->dataObjs);
		if (infoData->undfObjs) {
			fprintf(out, "%-30s: %10ld\n", "  #Undf Objects (?)", infoData->undfObjs);
		}
		fprintf(out, "%-30s: %10ld\n", "  #Total Objects", totlObjects);
		fprintf(out, "%-30s: %10ld (%ld%%)\n", "  Memory used by all objects", infoData->dataSize, percUsedMem);

		result = 1;
	} else {
		fprintf(context->err, "?Null pointer returned, check a_api.c\n");
		result = 0;
	}
	return result;
}



/* Entry point for dumping some brief info about the shared memory and the
 * database it currently holds.
 * The user defined function "datBriefInfoCallback" should be called by
 * a_api, with an instance of a_apiInfoData, for us (dat.c) to display.
 */
int
datBriefInfo(
	datContext context)
{
	a_apiInfoDataAction action = (a_apiInfoDataAction)datBriefInfoCallback;
	return a_apiPushMeInfoData(context->apiContext, action, context);
}




/****************************************************************
 DO YOUR DAT STUFF   ;-)
 ****************************************************************/


/* Do all requested things
 */
int
datDo(
	datContext context,
	char *argv0)
{
	int result = 0;
	int actions = context->actions;
	int options = context->options;
	if (actions & A_UNDEFINED) {
		datDisplayTitle();
		datDisplayHelpShort(argv0);
		result++;
	} else if (actions & A_TEST) {
		datTest(context);
		result++;
	} else if (actions & A_HELP) {
		datDisplayTitle();
		datDisplayHelp(argv0);
		result++;
	} else {
		char *fname;
		char * dir_name = os_getTempDir();
		context->apiContext = a_apiInit(context->out, context->err, context->shmName, context->dbName,
			dir_name, D_KEYFILE_MASK, options & O_SHOW_ANALYSE_OUTPUT, options & O_SHOW_VERBOSE_OUTPUT);
		if ( (actions & A_MEM_SAVE_TO_FILE) || (options & O_MEM_LOAD_FROM_FILE) ) {
			fname = context->memoryFilename;
		} else {
			fname = NULL;
		}
		result = a_apiPrepare(context->apiContext, fname, options & O_MEM_LOAD_FROM_FILE);

		if (result) {

			result = a_apiAnalyse(context->apiContext);

			if (actions & A_INFO) {
				result = datBriefInfo(context);
			}

			if (actions & A_SHOW_STATS) {
				result = datDumpFreeSpaceStats(context);
			}

			if (actions & A_HEXDUMP) {
				result = a_apiHexdump(context->apiContext, context->hexdumpWidth, context->hexdumpGroupwidth);
			}

			if (datDumpRequested(context)) {
				result = datDumpEntries(context);
			}

			if (actions & A_HEXDUMP_WIPED) {
				result = a_apiHexdumpWiped(context->apiContext, context->hexdumpWidth, context->hexdumpGroupwidth);
			}

		}
		if (!result) {
			fprintf(context->err, "[%s] ?%s\n", argv0, a_apiErrorStr(a_apiLastError(context->apiContext)));
		}
		a_apiDeInit(context->apiContext);
		context->apiContext = NULL;
	}
	return result;
}





/* DAT Main
 */
int
main(
	int argc,
	char *argv[])
{
	int internalErrorLevel = 0;
	int externalErrorLevel = 0;
	datContext context = datInit();

	datParseCmdline(argc, argv, context);

	if (datCheckExclusions(context)) {
		internalErrorLevel = datDo(context, argv[0]);
	}
	datDeInit(context);

	if (!internalErrorLevel) {
		externalErrorLevel = 1;
	}

	if (!externalErrorLevel) {
//		printf("\nNormal program end\n");
	} else {
		printf("\nAbnormal program end with errorlevel: %d\n", externalErrorLevel);
	}

	return(externalErrorLevel);
}



//END dat.c
