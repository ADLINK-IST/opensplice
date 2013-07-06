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

#define A_FIL_MAX_NAME_LENGTH 255



/* Info about the memory file
 */
struct a_filMemFileMeta_s {
	char      shm_name[A_FIL_MAX_NAME_LENGTH + 1];
	char      db_name[A_FIL_MAX_NAME_LENGTH + 1];
	c_address address;
	c_long    shm_size;
};



/* Context info
 */
struct a_filContext_s {
	char   *fname;
	struct  a_filMemFileMeta_s memFileMeta;
};



a_filContext
a_filInit(
	char *fname,
	char *shm_name,
	char *db_name,
	c_address address,
	c_long shm_size)
{
	a_filContext context = a_memAlloc(sizeof(struct a_filContext_s));
	context->fname = fname ? a_memStrdup(fname) : NULL;
	if (shm_name) {
		strcpy(context->memFileMeta.shm_name, shm_name);
	}
	if (db_name) {
		strcpy(context->memFileMeta.db_name, db_name);
	}
	context->memFileMeta.address = address;
	context->memFileMeta.shm_size = shm_size;
	return context;
}



void
a_filDeInit(
	a_filContext context)
{
	if (context) {
		if (context->fname) {
			a_memFree(context->fname);
		}
		if (context->memFileMeta.shm_name) {
			a_memFree(context->memFileMeta.shm_name);
		}
		if (context->memFileMeta.db_name) {
			a_memFree(context->memFileMeta.db_name);
		}
		a_memFree(context);
	}
}
	


/* Writes a header with Mem File Meta info to a file.
 * The file is considered to be opened for writing, 
 * i.e. the file descriptor fd > 0.
 * Returns 0 if antyhing fails, otherwise >0.
 */
static int
a_filWriteHeader(
	a_filContext context,
	int fd)
{
	int result = 0;
	if (0 < fd) {
		int n;  // nr of bytes writen
		struct iovec v;
		v.iov_base = (void *)&(context->memFileMeta);
		v.iov_len = sizeof(context->memFileMeta);
		n = writev(fd, &v, 1);
		result = 0 < n;
	}
	return result;
}



/* Writes a memory dump to a file, preceded by a header.
 * It uses heapAddress as the Start Address and takes the
 * size from the context's info.
 * If anything fails, 0 is returned, otherwise >0.
 */
int
a_filHeap2File(
	a_filContext context,
	c_address heapAddress)
{
	int result;
	int fd;  // file descriptor;
	fd = open(context->fname, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP);
	if (0 < fd) {
		if (a_filWriteHeader(context, fd)) {
			int n;
			struct iovec v;
			v.iov_base = (void *)heapAddress;
			v.iov_len = context->memFileMeta.shm_size;
			n = writev(fd, &v, 1);
			result = 0 < n;
		}
		close(fd);
	} else {
		result = 0;
	}
	if (!result) {
		perror("?Could not create file for writing, reason");
	}
	return result;
}



/* Reads the memory file's header into the context.
 * The file is considered to be opened (0 < fd) and
 * the file pointer is at the start of the file. (Nothing
 * has been read yet.)
 * This function will return 0 if anything fails, otherwise >0.
 */
static int
a_filReadHeaderDo(
	a_filContext context,
	int fd)
{
	int result;
	if (0 < fd) {
		int n;
		struct iovec v;
		v.iov_base = (void *)&(context->memFileMeta);
		v.iov_len = sizeof(context->memFileMeta);
		n = readv(fd, &v, 1);
		result = 0 < n;
	} else {
		result = 0;
	}
	return result;
}



/* Reads the memory file into memory.
 * The file is considered to be opened (0 < fd) and the
 * header has been read (for setting the file pointer to
 * the start of the memory data).
 * This function will return 0 if anything fails, otherwise >0.
 */
static int
a_filReadFile2HeapDo(
	a_filContext context,
	int fd,
	c_address heapAddress)
{
	int result;
	if (0 < fd) {
		int n;
		struct iovec v;
		v.iov_base = (void *)heapAddress;
		v.iov_len = context->memFileMeta.shm_size;
		n = readv(fd, &v, 1);
		result = 0 < n;
	} else {
		result = 0;
	}
	return result;
}



/* Reads the header info of a memory file and stores it in the
 * context. Its info can then be read through the various
 * Get Functions.
 * If anything fails, this function will return 0, otherwise >0.
 */
int
a_filReadHeader(
	a_filContext context)
{
	int result;
	int fd;
	fd = open(context->fname, O_RDONLY, 0);
	if (0 < fd) {
		result = a_filReadHeaderDo(context, fd);
		close(fd);
	} else {
		result = 0;
	}
	return result;
}


#if 0

static void
a_filPrintHeader(
	a_filContext context)
{
	printf("[a_filPrintHeader] shm_name=\"%s\"\n", a_filGetShmName(context));
	printf("[a_filPrintHeader]  db_name=\"%s\"\n", a_filGetDbName(context));
	printf("[a_filPrintHeader]  address=0x%X\n", (unsigned int)a_filGetShmAddress(context));
	c_long size = a_filGetShmSize(context);
	printf("[a_filPrintHeader] shm_size=0x%X (%ld)\n", (unsigned int)size, size);
}

#endif



/* Reads a memory file into heap. Before calling this function,
 * the file's header must have been read (by a_filReadHeader) and
 * a (heap) memory block must have been allocated with the size
 * collected from the ReadHeader action.
 * If anything fails, this function will return 0, otherwise >0.
 */
int
a_filFile2Heap(
	a_filContext context,
	c_address heapAddress)
{
	int result;
	int fd;
	fd = open(context->fname, O_RDONLY, 0);
	if (0 < fd) {
		if (a_filReadHeaderDo(context, fd)) {    // dummy read for setting file pointer
			// a_filPrintHeader(context);
			result = a_filReadFile2HeapDo(context, fd, heapAddress);
		}
		close(fd);
	} else {
		result = 0;
	}
	if (!result) {
		perror("?Could not open file for reading, reason");
	}
	return result;
}
	


/* Returns a pointer to the currently known shm_name in the context
 */
char *
a_filGetShmName(
	a_filContext context)
{
	return context ? context->memFileMeta.shm_name : NULL;
}



/* Returns a pointer to the currently known db_name in the context
 */
char *
a_filGetDbName(
	a_filContext context)
{
	return context ? context->memFileMeta.db_name : NULL;
}



/* Returns the currently known shm_size in the context
 */
c_long
a_filGetShmSize(
	a_filContext context)
{
	return context ? context->memFileMeta.shm_size : 0;
}



/* Returns the currently known address pointer in the context
 */
c_address
a_filGetShmAddress(
	a_filContext context)
{
	return context ? context->memFileMeta.address : 0;
}



//END a_fil.c
