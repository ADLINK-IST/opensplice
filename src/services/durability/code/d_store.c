/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "os.h"

#include "d__store.h"
#include "d_store.h"

#include "d_storeXML.h"

#include "d_misc.h"
#include "d_actionQueue.h"
#include "d_nameSpace.h"
#include "d_configuration.h"
#include "d_lock.h"
#include "os_heap.h"
#include "os_report.h"

static void
d_storePrint(
    d_store store,
    const char* format,
    va_list args)
{
    char description[512];

    if(store->config->tracingOutputFile){
        vsnprintf(description, sizeof(description)-1, format, args);
        description [sizeof(description)-1] = '\0';
        fprintf(store->config->tracingOutputFile, "%s", description);
    }
}

static void
d_storePrintState(
    d_store store)
{
    os_time time;
    const c_char* state;

    if(store->config->tracingOutputFile){
        switch(store->type){
            case D_STORE_TYPE_XML:
                state = "XML";
                break;
            case D_STORE_TYPE_BIG_ENDIAN:
                state = "BIG ENDIAN";
                break;
            default:
                state = "<<UNKNOWN>>";
                break;
        }

        if(store->config->tracingTimestamps == TRUE){
            time = os_timeGet();

            if(store->config->tracingRelativeTimestamps == TRUE){
                time = os_timeSub(time, store->config->startTime);
            }
            fprintf(store->config->tracingOutputFile,
                        "%d.%9.9d PersistentStore (%s) -> ",
                        time.tv_sec, time.tv_nsec, state);
        } else {
            fprintf(store->config->tracingOutputFile,
                        "PersistentStore (%s) -> ", state);
        }
    }
}

void
d_storeInit(
    d_store store,
    d_objectDeinitFunc deinit)
{
    d_lockInit(d_lock(store), D_STORE, deinit);
}

void
d_storeFree(
    d_store store)
{
    d_lockFree(d_lock(store), D_STORE);
}

d_store
d_storeOpen(
    const d_configuration config,
    d_storeType storeType)
{
    d_store store;
    d_storeResult result;

    store = NULL;

    switch(storeType){
        case D_STORE_TYPE_XML:
            store = d_store(d_storeNewXML());
            break;
        case D_STORE_TYPE_BIG_ENDIAN:
            store = NULL;
            break;
        default:
            OS_REPORT(OS_ERROR, "durability", 0,
                "Supplied persistent store type unknown.");
            store = NULL;
            break;
    }
    if(store != NULL) {
        store->type        = storeType;
        store->config      = config;

        if(store->openFunc) {
            result = store->openFunc(store);

            if(result != D_STORE_RESULT_OK){
                switch(storeType){
                    case D_STORE_TYPE_XML:
                        d_storeFreeXML(d_storeXML(store));
                        store = NULL;
                        break;
                    case D_STORE_TYPE_BIG_ENDIAN:
                        store = NULL;
                        break;
                    default:
                        store = NULL;
                        assert(FALSE);
                        break;
                }
            }
        }
    }
    return store;
}

d_storeResult
d_storeClose(
    d_store store)
{
    d_storeResult result;

    if(store){
        if(store->closeFunc){
            result = store->closeFunc(store);

            if(result == D_STORE_RESULT_OK) {
                switch(store->type){
                    case D_STORE_TYPE_XML:
                        result = d_storeFreeXML(d_storeXML(store));
                        store = NULL;
                        break;
                    case D_STORE_TYPE_BIG_ENDIAN:
                        break;
                    default:
                        break;
                }
            }
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStart(
    const d_store store)
{
    d_storeResult result;

    if(store){
        if(store->actionStartFunc){
            result = store->actionStartFunc(store);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStop(
    const d_store store)
{
    d_storeResult result;

    if(store){
        if(store->actionStopFunc){
            result = store->actionStopFunc(store);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupsRead(
    const d_store store,
    d_groupList *list)
{
    d_storeResult result;

    if(store){
        if(store->groupsReadFunc){
            result = store->groupsReadFunc(store, list);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupInject(
    const d_store store,
    const c_char* partition,
    const c_char* topic,
    const u_participant participant,
    d_group *group)
{
    d_storeResult result;

    if(store){
        if(store->groupInjectFunc){
            result = store->groupInjectFunc(store, partition, topic, participant, group);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupStore(
    const d_store store,
    const d_group group)
{
    d_storeResult result;

    if(store){
        if(store->groupStoreFunc){
            result = store->groupStoreFunc(store, group);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGetQuality(
    const d_store store,
    const d_nameSpace nameSpace,
    d_quality* quality)
{
    d_storeResult result;

    if(store){
        if(store->getQualityFunc){
            result = store->getQualityFunc(store, nameSpace, quality);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

/* Backup current namespace */
d_storeResult
d_storeBackup(
    const d_store store,
    const d_nameSpace nameSpace)
{
    d_storeResult result;

    if(store){
        if(store->backupFunc){
            result = store->backupFunc(store, nameSpace);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

/* Check if namespace is complete */
d_storeResult
d_storeNsIsComplete (
		const d_store store,
		const d_nameSpace nameSpace,
		c_bool* isComplete)
{
	d_storeResult result;

	if (store){
		if(store->nsIsCompleteFunc){
			result = store->nsIsCompleteFunc (store, nameSpace, isComplete);
		}else
		{
			result = D_STORE_RESULT_UNSUPPORTED;
		}
	}else
	{
		result = D_STORE_RESULT_ILL_PARAM;
	}

	return result;
}

/* Mark namespace incomplete or complete */
d_storeResult
d_storeNsMarkComplete (
		const d_store store,
		const d_nameSpace nameSpace,
		c_bool isComplete)
{
	d_storeResult result;

	if (store){
		if(store->nsMarkCompleteFunc){
			result = store->nsMarkCompleteFunc (store, nameSpace, isComplete);
		}else
		{
			result = D_STORE_RESULT_UNSUPPORTED;
		}
	}else
	{
		result = D_STORE_RESULT_ILL_PARAM;
	}

	return result;
}

/* Restore previously stored namespace */
d_storeResult
d_storeRestoreBackup(
		const d_store store,
		const d_nameSpace nameSpace)
{
	d_storeResult result;

	if (store)
	{
		if (store->restoreBackupFunc)
		{
			result = store->restoreBackupFunc(store, nameSpace);
			if (result == D_STORE_RESULT_OK)
			{
				/* After restoring, mark namespace as complete */
				result = d_storeNsMarkComplete (store, nameSpace, TRUE);
			}
		}else
		{
			result = D_STORE_RESULT_UNSUPPORTED;
		}
	}else
	{
		result = D_STORE_RESULT_ILL_PARAM;
	}

	return result;
}

d_storeResult
d_storeMessageStore(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->messageStoreFunc){
            result = store->messageStoreFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceDispose(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->instanceDisposeFunc){
            result = store->instanceDisposeFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceExpunge(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->instanceExpungeFunc){
            result = store->instanceExpungeFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessageExpunge(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->messageExpungeFunc){
            result = store->messageExpungeFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeDeleteHistoricalData(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->deleteHistoricalDataFunc){
            result = store->deleteHistoricalDataFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessagesInject(
    const d_store store,
    const d_group group)
{
    d_storeResult result;

    if(store){
        if(store->messagesInjectFunc){
            result = store->messagesInjectFunc(store, group);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceRegister(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->instanceRegisterFunc){
            result = store->instanceRegisterFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceUnregister(
    const d_store store,
    const v_groupAction message)
{
    d_storeResult result;

    if(store){
        if(store->instanceUnregisterFunc){
            result = store->instanceUnregisterFunc(store, message);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeCreatePersistentSnapshot(
    const d_store store,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri)
{
    d_storeResult result;

    if(store)
    {
        if(store->createPersistentSnapshotFunc)
        {
            result = store->createPersistentSnapshotFunc(store, partitionExpr, topicExpr, uri);
        } else
        {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else
    {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeOptimizeGroup(
    const d_store store,
    const d_group group)
{
    d_storeResult result;

    if(store){
        if(store->optimizeGroupFunc){
            result = store->optimizeGroupFunc(store, group);
        } else {
            result = D_STORE_RESULT_UNSUPPORTED;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

void
d_storeReport(
    const d_store store,
    d_level level,
    const char *format,
    ...)
{
    va_list args;

    if(store->config){
       if(((c_ulong)level) >= ((c_ulong)store->config->tracingVerbosityLevel)){
            d_storePrintState(store);
            va_start (args, format);
            d_storePrint(store, format, args);
            va_end (args);
        }
    }
}

d_storeResult
d_storeCopyFile(
    os_char* fileStorePath,
    os_char* destStorePath)
{
    FILE* source = NULL;
    FILE* destination = NULL;
    d_storeResult result = D_STORE_RESULT_OK;

    assert(fileStorePath);
    assert(destStorePath);

    /* Open the source file for reading */
    source = fopen(fileStorePath, "rb");
    if(source)/* if the source file doesn't exist, we thus return ok, nothing to copy! */
    {
        /* Open the destination file for writing */
        if(result == D_STORE_RESULT_OK)
        {
            destination = fopen(destStorePath, "wb");
            if(!destination)
            {
                result = D_STORE_RESULT_IO_ERROR;
            }
        }
        /* As long as we have not reached the end of the file of the source file
         * continue
         */
        while(result == D_STORE_RESULT_OK && !feof(source))
        {
            char ch;

            /* Get data from the source file */
            ch = fgetc(source);
            if(ferror(source))
            {
                result = D_STORE_RESULT_IO_ERROR;

            } else
            {
                if(!feof(source))
                {
                    fputc(ch, destination);
                    if(ferror(destination))
                    {
                        result = D_STORE_RESULT_IO_ERROR;
                    }
                }
            }
        }
    }
    /* Close the destination file */
    if(destination)
    {
        if(fclose(destination) == EOF)
        {
            result = D_STORE_RESULT_IO_ERROR;
        }
    }
    /* Close the source file */
    if(source)
    {
        if(fclose(source) == EOF)
        {
            result = D_STORE_RESULT_IO_ERROR;
        }
    }
    return result;
}
