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
/* format:
 *   file name: /<temporary directory>/spddskey_*, mode: 600
 *   line 1: <shm name>\0
 *   line 2: <shm base mapping address> [hex, printed in chars]
 *   line 3: <shm size>                 [hex, printed in chars]
 *   line 4: <shm version>              [?]
 *   line 5: <pid of creator>
 */



#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "a_key.h"
#include "a_mem.h"
#include "a_tre.h"




#define A_KEY_DUPESALLOWED           0
#define A_KEY_MAX_SHM_NAME_LENGTH  255
#define A_KEY_MAX_VERSION_LENGTH    30
#define A_KEY_MAX_FILE_LINE_LENGTH 256




/* File's Context
 * The real data structure (in this case: a tree) remains
 * hidden to the outside
 */
struct a_keyContext_s {
	a_treTree tree;
};





/* Data Structure for holding the information of one key file
 */
typedef struct a_keyKeyFile {
	char      *fname;       // full file name of key file (typically /tmp/spddskey_...)
	char      *shm_name;    // key file's line# 1: Shared Memory Name (or Domain Name)
	c_address  address;     //            line# 2: Shm Start Address
	c_long     size;        //            line# 3: Shm Size
	char      *version;     //            line# 4: Version (something)
	int        pid;         //            line# 5: Creator PID
} *a_keyKeyFile;







/* Compares the shm_name values of two entries. (uses strcmp)
 * Returns -1 if entry1 <  entry 2
 *          0           ==
 *          1            >
 *        -99 if entry1 or entry2 is NULL
 * This function is needed by a_tre.c, passed on to with a pointer
 * to function type.
 */
static int
a_keyCompareEntries(
	void *entry1Ptr,
	void *entry2Ptr)
{
	int result;
	a_keyKeyFile key1 = (a_keyKeyFile)entry1Ptr;
	a_keyKeyFile key2 = (a_keyKeyFile)entry2Ptr;
	if (key1 && key2) {
		result = strcmp(key2->shm_name, key1->shm_name);
		result = (result < 0) ? -1 : (0 < result) ? 1 : 0;    // force -1, 0 or 1
	} else {
		result = -99;
	}
	return result;
}



/* Creates a new KeyFile entry, specified by its members.
 * Remember to free the instance after use!
 */
static a_keyKeyFile
a_keyNewKeyFile(
	char      *fname,
	char      *shm_name,
	c_address  address,
	c_long     size,
	char      *version,
	int        pid)
{
	a_keyKeyFile newKeyFile = a_memAlloc(sizeof(struct a_keyKeyFile));
	newKeyFile->fname    = a_memStrdup(fname);
	newKeyFile->shm_name = a_memStrdup(shm_name);
	newKeyFile->address  = address;
	newKeyFile->size     = size;
	newKeyFile->version  = a_memStrdup(version);
	newKeyFile->pid      = pid;
	return newKeyFile;
}




/* Destroys (frees) an entry.
 */
static void
a_keyDestroyKeyFilesCallback(
	a_keyKeyFile *keyFile)
{
	if (*keyFile) {
		if ((*keyFile)->fname) {
			a_memFree((*keyFile)->fname);
		}
		if ((*keyFile)->shm_name) {
			a_memFree((*keyFile)->shm_name);
		}
		if ((*keyFile)->version) {
			a_memFree((*keyFile)->version);
		}
		a_memFree(*keyFile);
	}
}




/* Destroys all entries in a list and afterwards the list itself
 */
static int
a_keyDestroyKeyFiles(
	a_keyContext context)
{
	int result;
	if (context) {
		result = a_treDestroyTree(context->tree, (a_treFreeAction)a_keyDestroyKeyFilesCallback);
		context->tree = NULL;
	} else {
		result = 0;
	}
	return result;
}




/* Inserts an entry into the data structure
 */
static int
a_keyInsertKeyFile(
	a_keyContext context,
	a_keyKeyFile keyFile)
{
	return a_treInsertNode(context->tree, (void *)keyFile, (a_treSortAction)a_keyCompareEntries, A_KEY_DUPESALLOWED);
}



/* Finds an entry in the list, specified by its shm name and
 * returns a pointer to that entry, or NULL if not found.
 */
static a_keyKeyFile
a_keyFindKeyFile(
	a_keyContext context,
	char *shm_name)
{
	a_keyKeyFile result;
	a_treSortAction compareAction = a_keyCompareEntries;
	a_keyKeyFile tmpKeyFile = a_keyNewKeyFile("search entry", shm_name, 0, 0, "", 0);
	result = (a_keyKeyFile)a_treFindValue(context->tree, tmpKeyFile, compareAction);
	a_keyDestroyKeyFilesCallback(&tmpKeyFile);
	return result;
}



/* Opens and parses a file with filename <fname> (full file name path!)
 * and stores the contents in the list (internally).
 * Precondition:
 *   - the file exists and is user-readable
 *   - the file is of format:
 *       line# 1: <Shared Memory Name>       (string)
 *       line# 2: <shm start address>        (string in hex notation)
 *       line# 3: <shm size>                 (string in hex notation)
 *       line# 4: <version>                  (string)
 *       line# 5: <creator pid>              (string in decimal notation)
 */
static int
a_keyReadKeyFile(
	a_keyContext context,
	char *fname)
{
	int result = 0;
	const int max_line_length = A_KEY_MAX_FILE_LINE_LENGTH;
	char line[max_line_length + 1];
	FILE *fp;

	char        *shm_name;
	unsigned int hexAddress;
	c_address    address;
	unsigned int hexSize;
	c_long       size;
	char        *version;
	int          pid;

	fp = fopen(fname, "r");

	fgets(line, max_line_length, fp);
	shm_name = a_memStrdup(line);

	fgets(line, max_line_length, fp);
	sscanf(line, "%x", &hexAddress);
	address = (c_address)hexAddress;

	fgets(line, max_line_length, fp);
	sscanf(line, "%x", &hexSize);
	size = (c_long)hexSize;

	fgets(line, max_line_length, fp);
	version = a_memStrdup(line);

	fgets(line, max_line_length, fp);
	sscanf(line, "%d", &pid);

	fclose(fp);

	result = a_keyInsertKeyFile(context, a_keyNewKeyFile(fname, shm_name, address, size, version, pid));
	
	a_memFree(shm_name);
	a_memFree(version);
	
	return result;
}




/* Scans in <dir> for all files with file name starting
 * with <mask> *and* can be accessed (i.e. readable) by
 * current user.
 * Note: function "access" (unistd.h) is used. A similar
 *       function in os_stdlib.h (os_access) is prefered,
 *       but is seems that an enum type for arg2
 *       (permissions) is not exported and therefore
 *       unusable.
 *       Also, the file seperator is hard coded to "/".
 *       Better would be to use os_fileSep (os_stdlib.h).
 */
static int
a_keyScanFiles(
	a_keyContext context,
	const char *dir,
	const char *mask)
{
	int result = 1;
	DIR *dirp = opendir(dir);
	struct dirent *dirEntry;
	while (dirp && result) {
		dirEntry = readdir(dirp);
		if (dirEntry) {
			if (strncmp(dirEntry->d_name, mask, strlen(mask)) == 0) {
				char *fullFname = a_memAlloc(strlen(dir) + 1 + strlen(dirEntry->d_name) + 1);
				strcpy(fullFname, dir);
				strcat(fullFname, "/");
				strcat(fullFname, dirEntry->d_name);
				if (access(fullFname, R_OK) == 0) {
					result = a_keyReadKeyFile(context, fullFname);
				}
				a_memFree(fullFname);
			}
		} else {
			closedir(dirp);
			dirp = NULL;
		}
	}
	return result;
}




/* Initialise the context
 * A list with contents of all key files will be
 * created and stored (internally).
 */
a_keyContext
a_keyInit(
	const char *dir,
	const char *mask)
{
	a_keyContext context = a_memAlloc(sizeof(struct a_keyContext_s));
	context->tree = a_treCreateTree();
	a_keyScanFiles(context, dir, mask);
	return context;
}




/* Deinitialise the context
 * Remember to call this function before program termination
 */
void
a_keyDeInit(
	a_keyContext context)
{
	a_keyDestroyKeyFiles(context);
	a_memFree(context);
}



/* Searches for a (already recorded) key file with the specified
 * shm name and returns its start address, or 0 if not found.
 */
c_address
a_keyGetStartAddress(
	a_keyContext context,
	char *shm_name)
{
	a_keyKeyFile keyFile = a_keyFindKeyFile(context, shm_name);
	return keyFile ? keyFile->address : 0;
}



/* Searches for an (already recorded) key file with the specified
 * shm name and returns its size, or 0 if not found.
 */
c_long
a_keyGetSize(
	a_keyContext context,
	char *shm_name)
{
	a_keyKeyFile keyFile = a_keyFindKeyFile(context, shm_name);
	return keyFile ? keyFile->size : 0;
}



/* Searches for an (already recorded) key file with the specified
 * shm name and returns a pointer to its version string, or NULL
 * if not found.
 */
char *
a_keyGetVersion(
	a_keyContext context,
	char *shm_name)
{
	a_keyKeyFile keyFile = a_keyFindKeyFile(context, shm_name);
	return keyFile ? keyFile->version : NULL;
}




/* Searches for an (already recorded) key file with the specified
 * shm name and returns the pid of the creator, or 0 if not found.
 */
int
a_keyGetPid(
	a_keyContext context,
	char *shm_name)
{
	a_keyKeyFile keyFile = a_keyFindKeyFile(context, shm_name);
	return keyFile ? keyFile->pid : 0;
}



//END a_key.c
