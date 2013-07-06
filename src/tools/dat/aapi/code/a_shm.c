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
	FILE            *out;
	FILE            *err;
    char            *shm_name;
	os_sharedHandle  handle;
	int              isAttached;
};




/* Initialises the context and returns a new pointer to it.
 * Must be called before any other operation in this file.
 * Remember to DeInit after use!
 */
a_shmContext
a_shmInit(
	FILE *out,
	FILE *err,
	char *shm_name)
{
	a_shmContext context = a_memAlloc(sizeof(struct a_shmContext_s));
	
	context->out         = out;
	context->err         = err;
	context->shm_name    = a_memStrdup(shm_name);
	context->handle      = NULL;
	context->isAttached  = 0;
	return context;
}



/* De-initialises the context
 * Make sure to have detached the shm before calling this de-init.
 * Remember to call this function before the application terminates.
 */
void
a_shmDeInit(
	a_shmContext context)
{
	if (context) {
		if (context->handle) {
			os_sharedDestroyHandle(context->handle);
		}
		if (context->shm_name) {
			a_memFree(context->shm_name);
		}
		a_memFree(context);
	}
}



/* Returns a pointer to the shm name
 */
char *
a_shmGetShmName(
	a_shmContext context)
{
	return context->shm_name;
}



/* Returns the Start Address of Shared Memory of
 * current attached memory, otherwise NULL.
 */
c_address
a_shmGetShmAddress(
	a_shmContext context)
{
	return context ? (c_address)os_sharedAddress(context->handle) : (c_address)NULL;
}




/* Returns whether the Shared Memory is
 * currently (successfully) attached (by us!).
 */
int
a_shmIsAttached(
	a_shmContext context)
{
	return context ? context->isAttached : 0;
}



/* Set a new Shared Memory Name to attach to
 * Returns 1 if change was successful.
 */
int
a_shmSetShmName(
	a_shmContext context,
    char *newShmName)
{
	int result;
	if (context) {
		if (!a_shmIsAttached(context)) {
			if (context->shm_name) {
				a_memFree(context->shm_name);
			}
			context->shm_name = a_memAlloc(strlen(newShmName) + 1);
			strcpy(context->shm_name, newShmName);
			result = 1;
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}



/* Appends appName to the shm name
 */
int
a_shmAppShmName(
	a_shmContext context,
	char *appName)
{
	int result;
	if (context) {
		if (!a_shmIsAttached(context)) {
			if (context->shm_name) {
				char *name = a_memAlloc(strlen(context->shm_name) + 1 + strlen(appName));
				strcpy(name, context->shm_name);
				strcat(name, appName);
				result = a_shmSetShmName(context, name);
				a_memFree(name);
			} else {
				result = 0;
			}
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}




/* Attaches to Shared Memory
 * Returns >0 if all went ok.
 */
int
a_shmAttach(
	a_shmContext context)
{
	os_result osResult;
	if (context) {
		if (context->shm_name) {
			if (!a_shmIsAttached(context)) {
				os_sharedAttr shm_attr;
				os_sharedAttrInit(&shm_attr);
				context->handle = os_sharedCreateHandle(context->shm_name, &shm_attr, 0);
				osResult = os_sharedMemoryAttach(context->handle);
			} else {
				osResult = os_resultFail;
			}
		} else {
			osResult = os_resultFail;
		}
	} else {
		osResult = os_resultFail;
	}
	context->isAttached = (osResult == os_resultSuccess) ? 1 : 0;
	return a_shmIsAttached(context);
}



/* Detaches from Shared Memory.
 * Returns >0 if all went ok.
 */
int
a_shmDetach(
	a_shmContext context)
{
	int result;
	os_result osResult;
	if (context) {
		osResult = a_shmIsAttached(context) ? os_sharedMemoryDetach(context->handle) : os_resultFail;
		result = (osResult == os_resultSuccess) ? 1 : 0;
		context->isAttached = result ? 0 : 1;
	} else {
		result = 0;
	}
	return result;
}



/* Creates Shared Memory
 * Precondition: name has been set (via a_shmSetShmName)
 */
int
a_shmCreateShm(
	a_shmContext context,
	c_address map_address,
	c_long size)
{
	int result;
	if (context) {
		if (context->shm_name) {
			os_result osResult;
			os_sharedAttr sharedAttr;
			osResult = os_sharedAttrInit(&sharedAttr);
			if (osResult == os_resultSuccess) {
				sharedAttr.lockPolicy = OS_UNLOCKED;
				sharedAttr.sharedImpl = OS_MAP_ON_SEG;
				sharedAttr.map_address = (void *)map_address;
		
				context->handle = os_sharedCreateHandle(context->shm_name, &sharedAttr, 0);
		
				if (context->handle ) {
					osResult = os_sharedMemoryCreate(context->handle, (os_uint)size);
					if (osResult != os_resultSuccess) {
						fprintf(context->err, "a_shmCreateShm: os_sharedMemoryCreate failed\n");
						fprintf(context->err, "              : size=%ld\n", (long)size);
						fprintf(context->err, "              : map_address=0x%X\n", (unsigned int)map_address);
						fprintf(context->err, "              : handle=0x%X\n", (unsigned int)context->handle);
					}
				} else {
					osResult = os_resultFail;
					fprintf(context->err, "a_shmCreateShm: os_sharedCreateHandle failed\n");
				}
			} else {
				fprintf(context->err, "a_shmCreateShm: os_sharedAttrInit failed\n");
			}
			result = (osResult == os_resultSuccess) ? 1 : 0;
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}




/* Destroys a Shared Memory Segment
 * Make sure to detach first.
 * Returns >0 if destroy was successful.
 */
int
a_shmDestroyShm(
	a_shmContext context)
{
	int result;
	if (context) {
		if (!a_shmIsAttached(context)) {
			result = (os_sharedMemoryDestroy(context->handle) == os_resultSuccess) ? 1 : 0;
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}




//END a_shm.c
