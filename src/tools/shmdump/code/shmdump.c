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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "os.h"


enum destination {
    DEST_NONE,
    DEST_FILE,
    DEST_SHM,
    DEST_COUNT
};

struct arguments {
    enum destination dest;
    char             *fileName;
    char             *shmName;
    os_uint32          size;
};

static int
getArguments(
    int argc,
    char *argv[],
    struct arguments *args)
{
    int i;
    int result;
    int count;
    int tmp;

    if (argc < 7) {
        printf("Usage: %s -size size -shm name [-o file] [-i file]\n", argv[0]);
        return -1;
    }

    result = 0;
    args->dest = DEST_NONE;
    args->fileName = NULL;
    args->shmName = NULL;
    args->size = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-size") == 0) {
            count = sscanf(argv[i+1], "%d", &tmp);
            if ((count == 1) && tmp >= 0) {
                args->size = tmp;
            } else {
                printf("ERROR: Invalid size argument %s\n", argv[i + 1]);
                result = -1;
                args->size = -1;
            }
            i++;
        }
        if (strcmp(argv[i], "-shm") == 0) {
            args->shmName = os_strdup(argv[i+1]);
            i++;
        }
        if (strcmp(argv[i], "-o") == 0) {
            args->dest = DEST_FILE;
            args->fileName = os_strdup(argv[i + 1]);
            i++;
        }
        if (strcmp(argv[i], "-i") == 0) {
            args->dest = DEST_SHM;
            args->fileName = os_strdup(argv[i + 1]);
            i++;
        }
    }

    if (args->dest == DEST_NONE) {
        printf("ERROR: Input or output to file?\n");
        result = -1;
    }
    if (args->fileName == NULL) {
        printf("ERROR: No file name!\n");
        result = -1;
    }
    if (args->shmName == NULL) {
        printf("ERROR: No sharedmemory name!\n");
        result = -1;
    }
    if (args->size == 0) {
        printf("ERROR: Invalid size\n");
        result = -1;
    }
    return result;
}

static int
fileFromShm(
    struct arguments *args)
{
    int r;
    os_sharedHandle shm;
    os_sharedAttr shmAttr;
    os_result result;
    FILE *outFile;
    void *address;
    os_address size;
    size_t written;
    
    written = 0;
    r = 0;
    os_sharedAttrInit(&shmAttr);
    shm = os_sharedCreateHandle(args->shmName, &shmAttr, 0);
    if (shm != NULL) {
        result = os_sharedMemoryAttach(shm);
        if (result == os_resultSuccess) {
            address = os_sharedAddress(shm);
            result = os_sharedSize(shm, &size);
            if (result == os_resultSuccess && (os_uint32)size >= args->size) {
                outFile = fopen(args->fileName, "w");
                if (outFile != NULL) {
                    written = fwrite(address, (size_t)args->size, (size_t)1, outFile);
                    fclose(outFile);
                } else {
                    printf("ERROR: Failed to open file: %s\n", args->fileName);
                    r = -1;
                }
                if (written != (size_t)1) {
                    printf("ERROR: Copy did not complete.\n");
                    r = -1;
                }
            } else {
                printf("ERROR: Copy did not complete. The size is invalid.\n");
                r = -1;
            }
            os_sharedMemoryDetach(shm);
        } else {
            printf("ERROR: Failed to attach to shared memory.\n");
            r = -1;
        }
        os_sharedDestroyHandle(shm);
    } else {
        printf("ERROR: Failed to create shared memory handle.\n");
        r = -1;
    }

    if (r == 0) {
        printf("INFO: Copy shared memory '%s' to file '%s' complete.\n",
               args->shmName, args->fileName);
    }

    return r;
}

static int
shmFromFile(
    struct arguments *args)
{
    int r;
    os_sharedHandle shm;
    os_sharedAttr shmAttr;
    os_result result;
    FILE *inFile;
    void *address;
    size_t read;

    read = 0;
    r = 0;
    os_sharedAttrInit(&shmAttr);
    shm = os_sharedCreateHandle(args->shmName, &shmAttr, 0);
    if (shm != NULL) {
        result = os_sharedMemoryCreate(shm, args->size);
        if (result == os_resultSuccess) {
            result = os_sharedMemoryAttach(shm);
            if (result == os_resultSuccess) {
                address = os_sharedAddress(shm);
                inFile = fopen(args->fileName, "r");
                if (inFile != NULL) {
                    read = fread(address, (size_t)args->size, (size_t)1, inFile);
                    fclose(inFile);
                } else {
                    printf("ERROR: Failed to open file: %s\n", args->fileName);
                    r = -1;
                }
                if (read != (size_t)1) {
                    printf("ERROR: Copy did not complete.\n");
                    r = -1;
                }
                os_sharedMemoryDetach(shm);
                if (r) {
                    os_sharedMemoryDestroy(shm);
                }
	    } else {
                printf("ERROR: Failed to attach to memory.\n");
                os_sharedMemoryDestroy(shm);
                r = -1;
            }
        } else {
            printf("ERROR: Failed to create shared memory.\n");
            r = -1;
        }
        os_sharedDestroyHandle(shm);
    } else {
        printf("ERROR: Failed to create shared memory handle.\n");
        r = -1;
    }

    if (r == 0) {
        printf("INFO: Copy file '%s' to shared memory '%s' complete.\n",
               args->fileName, args->shmName);
    }

    return r;
}

#ifdef INTEGRITY
OPENSPLICE_ENTRYPOINT (ospl_shmdump)
#else
OPENSPLICE_MAIN (ospl_shmdump)
#endif
{
    struct arguments args;
    int result;

    /**
     * Usage: shmdump -shmsize size -shm name [-o file] [-i file]
     */
    result = getArguments(argc, argv, &args);

    os_osInit();

    if (result == 0) {
        switch (args.dest) {
            case DEST_FILE:
                result = fileFromShm(&args);
                break;
            case DEST_SHM:
                result = shmFromFile(&args);
                break;
            default:
                printf("Error: unknown destination %d\n", args.dest);
                break;
        }

        if (args.fileName != NULL) {
            free(args.fileName);
        }
        if (args.shmName != NULL) {
            free(args.shmName);
        }
    }

    return result;
}

