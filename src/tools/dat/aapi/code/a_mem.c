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
