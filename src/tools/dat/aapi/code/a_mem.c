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
}



/* Copies memory.
 * Returns 0 if failure, otherwise >0.
 */
int
a_memCopyMem(
	void *toAddr,
	void *fromAddr,
	long size)
{
	return (memcpy(toAddr, fromAddr, (size_t)size)) ? 1 : 0;
}



/* Frees a memory block on heap.
 */
void
a_memFree(
	void *ptr)
{
	if (ptr) {
		os_free(ptr);
	}
}

	



/* Same functionality as os_strdup, but uses os_malloc internally,
 * which makes it platform independent
 */
char *
a_memStrdup(
	char *src)
{
#if 0
	char *target;
	if (src) {
		target = a_memAlloc(strlen(src) + 1);
		strcpy(target, src);
	} else {
		target = NULL;
	}
	return target;
#else
	return src ? os_strdup(src) : NULL;
#endif
}



//END a_mem.c
