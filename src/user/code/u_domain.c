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
#include "u__types.h"
#include "u__user.h"
#include "u__domain.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__usrClock.h"
#include "u__usrReportPlugin.h"
#include "u__waitset.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_config.h"
#include "os_abstract.h"
#include "c_iterator.h"
#include "v_handle.h"

#include "cf_config.h"
#include "cfg_parser.h"
#include "cfg_validator.h"
#include "ut_entryPoint.h"

#include "u_rnr.h"
#include "u__cfElement.h"

#include "v_configuration.h"
#include "v_cfNode.h"
#include "v_cfElement.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"
#include "v_processInfo.h"
#include "v_typeRepresentation.h"
#include "os_atomics.h"

#define IGNORE_THREAD_MESSAGE os_threadMemFree(OS_THREAD_WARNING)
#define PRINT_THREAD_MESSAGE(context) printThreadMessage(context)

#define DATABASE_NAME "defaultDomainDatabase"
#define DATABASE_SIZE (0xA00000)
#define KERNEL_NAME   "defaultDomainKernel"
#define DATABASE_SIZE_MIN_32 (0x200000)
#define DATABASE_SIZE_MIN_64 (0x500000)
#define DATABASE_FREE_MEM_THRESHOLD (0x100000)
#define DATABASE_FREE_MEM_THRESHOLD_MIN (0)

static os_boolean processConfigAlreadySet = OS_FALSE;
static pa_uint32_t domainSerial = PA_UINT32_INIT(0);

static void
printThreadMessage(
    const char *context)
{
    os_char *msg = os_threadMemGet(OS_THREAD_WARNING);
    if (msg) {
        OS_REPORT(OS_ERROR,context,0,"%s",msg);
        os_threadMemFree(OS_THREAD_WARNING);
    }
}

C_STRUCT(u_domainConfig) {
    os_char       *name;
    u_size        dbSize;
    u_size        dbFreeMemThreshold;
    os_address    address;
    os_lockPolicy lockPolicy;
    u_bool        heap;
    u_bool        builtinTopicEnabled;
    u_bool        prioInherEnabled;
    u_domainId_t  id;
    u_bool        idReadFromConfig;
    c_bool        maintainObjectCount;
    u_bool        inProcessExceptionHandling;
    struct v_systemIdConfig systemIdConfig;
};

C_STRUCT(attributeCopyArg) {
    v_configuration configuration;
    v_cfElement element;
};

/**************************************************************
 * Private functions
 **************************************************************/

static void
GetDomainConfigSystemId(
    const cf_element dc,
    C_STRUCT(u_domainConfig) *domainConfig)
{
    cf_element e_systemId, e_range, e_entropy;

    if ((e_systemId = cf_element(cf_elementChild(dc, CFG_SYSTEMID))) == NULL) {
        return;
    }

    if ((e_range = cf_element(cf_elementChild(e_systemId, CFG_SYSTEMIDRANGE))) != NULL) {
        cf_attribute attr;
        os_uint64 m;
        char *endp;
        if ((attr = cf_elementAttribute(e_range, "min")) != NULL) {
            c_value v = cf_attributeValue(attr);
            m = os_strtoull(v.is.String, &endp, 0);
            if (m == 0 || m > 0x7fffffff || *(endp + strspn(endp, " \t")) != 0) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                          "Domain/SystemId/Range[@min] value \"%s\" invalid, using %"PA_PRIu32,
                          v.is.String, domainConfig->systemIdConfig.min);
            } else {
                domainConfig->systemIdConfig.min = (os_uint32) m;
            }
        }
        if ((attr = cf_elementAttribute(e_range, "max")) != NULL) {
            c_value v = cf_attributeValue(attr);
            m = os_strtoull(v.is.String, &endp, 0);
            if (m == 0 || m > 0x7fffffff || *(endp + strspn(endp, " \t")) != 0) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                          "Domain/SystemId/Range[@max] value \"%s\" invalid, using %"PA_PRIu32,
                          v.is.String, domainConfig->systemIdConfig.max);
            } else if (m < domainConfig->systemIdConfig.min) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                          "Domain/SystemId/Range is empty, using %"PA_PRIu32" .. %"PA_PRIu32,
                          domainConfig->systemIdConfig.min, domainConfig->systemIdConfig.max);
            } else {
                domainConfig->systemIdConfig.max = (os_uint32) m;
            }
        }
    }

    if ((e_entropy = cf_element(cf_elementChild(e_systemId, CFG_SYSTEMIDENTROPY))) != NULL) {
        cf_data data;
        if ((data = cf_data(cf_elementChild(e_entropy, "#text"))) != NULL) {
            c_value v = cf_dataValue(data);
            domainConfig->systemIdConfig.entropy = v.is.String;
            domainConfig->systemIdConfig.entropySize = (os_uint32) strlen(v.is.String);
        }
    }
}

static void
GetDomainConfig(
    const cf_element config,
    C_STRUCT(u_domainConfig) *domainConfig,
    os_sharedAttr    *shm_attr)
{
    cf_element dc = NULL;
    cf_element child;
    cf_element id;
    cf_element inProcExcHandling;
    cf_element name;
    cf_data elementData;
    cf_element size;
    cf_element threshold;
    cf_element address;
    cf_element singleProcess;
    cf_element locked;
    cf_element maintainObjectCount;
    c_value value;
    cf_attribute attr;
    os_boolean doAppend;
    os_boolean sizeSet = OS_FALSE;
    os_boolean defaultThresholdsize = OS_TRUE;
    os_int32 cfDomainId = -1;

    assert(config != NULL);
    assert(domainConfig != NULL);
    assert(domainConfig->name != NULL);
    assert(shm_attr != NULL);

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));
    if (dc != NULL) {
        name = cf_element(cf_elementChild(dc, CFG_NAME));
        if (name != NULL) {
            elementData = cf_data(cf_elementChild(name, "#text"));
            if (elementData != NULL) {
                value = cf_dataValue(elementData);
                os_free(domainConfig->name);
                domainConfig->name = os_strdup(value.is.String);

                id = cf_element(cf_elementChild(dc, CFG_ID));

                if (id != NULL) {
                    elementData = cf_data(cf_elementChild(id, "#text"));
                    if (elementData != NULL) {
                        value = cf_dataValue(elementData);
                        sscanf(value.is.String, "%d", &domainConfig->id);
                        domainConfig->idReadFromConfig = TRUE;
                        cfDomainId = domainConfig->id;
                    }
                }

                inProcExcHandling = cf_element(cf_elementChild(dc, CFG_IN_PROC_EXC));
                if (inProcExcHandling != NULL) {
                    elementData = cf_data(cf_elementChild(inProcExcHandling, "#text"));

                    if (elementData != NULL) {
                        value = cf_dataValue(elementData);
                        if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                            domainConfig->inProcessExceptionHandling = TRUE;
                        } else if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                            domainConfig->inProcessExceptionHandling = FALSE;
                        } else {
                            OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_ILL_PARAM,
                                "Incorrect <InProcessExceptionHandling> parameter for Domain: \"%s\","
                                " using default",value.is.String);
                        }
                    }
                }

                child = cf_element(cf_elementChild(dc, CFG_DATABASE));
                if (child != NULL) {
                    size = cf_element(cf_elementChild(child, CFG_SIZE));
                    if (size != NULL) {
                        elementData = cf_data(cf_elementChild(size, "#text"));
                        if (elementData != NULL) {
                            u_size defaultMinDbSize = DATABASE_SIZE_MIN_64;
                            value = cf_dataValue(elementData);
                            if (u_cfDataSizeValueFromString(value.is.String,&domainConfig->dbSize) != FALSE) {
                                sizeSet = OS_TRUE;
                            }
                            if (sizeof(void*) == 4) {
                                /* 32 bits mode, set min db size to 32 bits min size (default was 64).*/
                                defaultMinDbSize = DATABASE_SIZE_MIN_32;
                            }
                            if((domainConfig->dbSize != 0) && (domainConfig->dbSize < defaultMinDbSize))
                            {
                                domainConfig->dbSize = defaultMinDbSize;
                                OS_REPORT_WID(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_UNSUPPORTED, cfDomainId,
                                    "Incorrect database size,"
                                    " using minimal database size 0x%x",(unsigned int)defaultMinDbSize);
                            }
                        }
                    }
                    threshold = cf_element(cf_elementChild(child, CFG_THRESHOLD));
                    if (threshold != NULL) {
                        elementData = cf_data(cf_elementChild(threshold, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            assert(value.kind == V_STRING);
                            if(u_cfDataSizeValueFromString(value.is.String,&domainConfig->dbFreeMemThreshold) != FALSE) {
                                defaultThresholdsize = OS_FALSE;
                            }
                            if(domainConfig->dbFreeMemThreshold <= DATABASE_FREE_MEM_THRESHOLD_MIN)
                            {
                                domainConfig->dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD_MIN;
                            }
                        }
                    }
                    if (defaultThresholdsize == OS_TRUE) {
                        if (domainConfig->dbSize <  DATABASE_SIZE) {
                            /* database size is less then the default db size. Minimize the default threshold size */
                            domainConfig->dbFreeMemThreshold = domainConfig->dbSize / 10;
                        }
                    }
                    address = cf_element(cf_elementChild(child, CFG_ADDRESS));
                    if (address != NULL) {
                        elementData = cf_data(cf_elementChild(address, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            if ( (strlen(value.is.String) > 2) &&
                                 (strncmp("0x", value.is.String, 2) == 0) ) {
                                sscanf(value.is.String, "0x" PA_ADDRFMT, &domainConfig->address);
                            } else {
                                sscanf(value.is.String, PA_ADDRFMT, &domainConfig->address);
                            }
                            shm_attr->map_address = (void*)domainConfig->address;
                        }
                    }
                    locked = cf_element(cf_elementChild(child, CFG_LOCKING));
                    domainConfig->lockPolicy = OS_LOCK_DEFAULT;
                    if (locked != NULL) {
                        elementData = cf_data(cf_elementChild(locked, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                                domainConfig->lockPolicy = OS_LOCKED;
                            } else if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                                domainConfig->lockPolicy = OS_UNLOCKED;
                            } else if (os_strncasecmp(value.is.String, "DEFAULT", 7) == 0) {
                                domainConfig->lockPolicy = OS_LOCK_DEFAULT;
                            } else {
                                OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                                    "Incorrect <Database/Locking> parameter for Domain: \"%s\","
                                    " using default locking",value.is.String);
                            }
                        }
                    } /* else: keep the platform dependent default OS_LOCK_DEFAULT */
                    shm_attr->lockPolicy = domainConfig->lockPolicy;
                    maintainObjectCount = cf_element(cf_elementChild(child, CFG_MAINTAINOBJECTCOUNT));
                    domainConfig->maintainObjectCount = 1;
                    if (maintainObjectCount != NULL) {
                        elementData = cf_data(cf_elementChild(maintainObjectCount, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                                domainConfig->maintainObjectCount = 1;
                            } else if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                                domainConfig->maintainObjectCount = 0;
                            } else {
                                OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                                    "Incorrect <Database/MaintainObjectValue> parameter for Domain: \"%s\","
                                    " using default",value.is.String);
                            }
                        }
                    } /* else: leave enabled */
                }
                singleProcess = cf_element(cf_elementChild(dc, CFG_SINGLEPROCESS));
                if (singleProcess != NULL) {
                   elementData = cf_data(cf_elementChild(singleProcess, "#text"));
                   if (elementData != NULL) {
                      value = cf_dataValue(elementData);
                      if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                         /* A SingleProcess value of True implies that Heap is to be used */
                         domainConfig->heap = TRUE;
                         shm_attr->sharedImpl = OS_MAP_ON_HEAP;
                         os_serviceSetSingleProcess();
                         /* default db size in single process mode is 0 */
                         if (sizeSet == OS_FALSE) {
                             domainConfig->dbSize = 0;
                         }
                      } else if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                         domainConfig->heap = FALSE;
                      }
                   }
                }
                child = cf_element(cf_elementChild(dc, CFG_BUILTINTOPICS));
                if (child != NULL) {
                    attr= cf_elementAttribute(child, "enabled");
                    if (attr != NULL) {
                        value = cf_attributeValue(attr);
                        if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                            domainConfig->builtinTopicEnabled = FALSE;
                        } /* else use default value */
                    } /* No attribute enabled, so use default value */
                } /* No 'BuiltinTopics' element, so use default value */

                child = cf_element(cf_elementChild(dc, CFG_PRIOINHER));
                if (child != NULL) {
                    attr= cf_elementAttribute(child, "enabled");
                    if (attr != NULL) {
                        value = cf_attributeValue(attr);
                        if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                            domainConfig->prioInherEnabled= TRUE;
                        } /* else use default value */
                    } /* No attribute enabled, so use default value */
                } /* No 'PriorityInheritance' element, so use default value */

                GetDomainConfigSystemId(dc, domainConfig);

                child = cf_element(cf_elementChild(dc, CFG_REPORT));
                if (child != NULL)
                {
                    attr = cf_elementAttribute(child, "verbosity");
                    if (attr != NULL)
                    {
                        value = cf_attributeValue(attr);
                        if (os_reportSetVerbosity(value.is.String) == os_resultFail)
                        {
                            OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                                    "Cannot parse report verbosity value \"%s\","
                                    " reporting verbosity remains %s",value.is.String, os_reportTypeText[os_reportVerbosity]);
                        }
                    }

                    attr = cf_elementAttribute(child, "append");
                    if (attr != NULL)
                    {
                        value = cf_attributeValue(attr);
                        if (os_configIsTrue(value.is.String, &doAppend) == os_resultFail)
                        {
                            OS_REPORT(OS_WARNING, OSRPT_CNTXT_USER, U_RESULT_INTERNAL_ERROR,
                                    "Cannot parse report append value \"%s\","
                                    " reporting append mode unchanged",value.is.String);
                        }
                        else
                        {
                            /* Remove log files when not appending. */
                            if (!doAppend) {
                                os_reportRemoveStaleLogs();
                            }
                        }
                    }
                }
            }
        }
    }
}

static void
attributeCopy(
    c_object o,
    c_iterActionArg argument)
{
    v_cfAttribute a;
    cf_attribute attr = (cf_attribute)o;
    C_STRUCT(attributeCopyArg) *arg = (C_STRUCT(attributeCopyArg) *)argument;

    assert(attr != NULL);
    assert(arg->configuration != NULL);
    assert(C_TYPECHECK(arg->element, v_cfElement));

    a = v_cfAttributeNew(arg->configuration,
                         cf_nodeGetName(cf_node(attr)),
                         cf_attributeValue(attr));
    v_cfElementAddAttribute(arg->element, a);
    c_free(a);
}

static u_result
copyConfiguration(
    cf_node  cfgNode,
    v_configuration config,
    v_cfNode *node)
{

    cf_node child;
    v_cfNode kChild;
    c_iter i;
    u_result r;
    C_STRUCT(attributeCopyArg) copyArg;

    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));

    r = U_RESULT_OK;
    if (cfgNode != NULL) {
        switch (cfgNode->kind) {
        case CF_ELEMENT:
            *node = v_cfNode(v_cfElementNew(config, cf_nodeGetName(cfgNode)));
            i = cf_elementGetAttributes(cf_element(cfgNode));
            copyArg.configuration = config;
            copyArg.element = v_cfElement(*node);
            c_iterWalk(i, attributeCopy, (c_iterActionArg)&copyArg);
            c_iterFree(i);

            i = cf_elementGetChilds(cf_element(cfgNode));
            child = (cf_node)c_iterTakeFirst(i);
            while (child != NULL) {
                copyConfiguration(cf_node(child), config, &kChild);
                v_cfElementAddChild(v_cfElement(*node), kChild);
                child = c_iterTakeFirst(i);
            }
            c_iterFree(i);
        break;
        case CF_DATA:
            *node = v_cfNode(v_cfDataNew(config,
                                         cf_dataValue(cf_data(cfgNode))));
        break;
        case CF_ATTRIBUTE:
        case CF_COUNT:
        case CF_NODE:
        default:
            assert(FALSE);
            r = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_WARNING,"user::u_domain::copyConfiguration",r,
                        "Unsuitable configuration node kind (%d)",
                        cfgNode->kind);
        }
    } else {
        *node = NULL;
    }
    return r;
}


static int
lockSharedMemory(
    os_sharedHandle shm)
{
    int result;
#ifndef INTEGRITY
    void *address;
    os_result r;
    os_address size;

    address = os_sharedAddress(shm);
    if (address) {
        r = os_sharedSize(shm, &size);
        if (r == os_resultSuccess) {
            r = os_procMLock(address, size);
        }
        if (r == os_resultSuccess) {
            result = 0; /* success */
        } else {
            result = 1; /*fail*/
        }
    } else {
       /* this should not happen, as this would mean
        * we are not attached to shared memory at all */
        assert(0);
        result = 1; /* fail */
    }
#else
    result = 0; /* success */
#endif
    return result;
}

static os_result
unlockSharedMemory(
    os_sharedHandle shm)
{
    os_result result;
#ifndef INTEGRITY
    os_address size;
    void *address;

    address = os_sharedAddress(shm);
    if (address) {
        result = os_sharedSize(shm, &size);
        if (result == os_resultSuccess) {
            result = os_procMUnlock(address, size);
        }
    } else {
       /* this should not happen, as this would mean
          we are not attach to shared memory at all */
        assert(0);
        result = os_resultFail;
    }
#else
    result = os_resultSuccess;
#endif
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR,
                    "user::u_domain::unlockSharedMemory", result,
                    "Could not unlock shared segment from memory."
                    OS_REPORT_NL "Result = \"%s\"",
                    os_resultImage(result));
    }
    return result;
}

static u_result
startSplicedWithinProcess(
    os_threadId *tid,
    const os_char *uri)
{
    os_library libraryHandle;
    os_libraryAttr libraryAttr;
    struct ut_entryPointWrapperArg *mwa;
    os_threadAttr threadAttr;
    os_result osr;
    u_result result;

    char * spliced = "spliced";
    char * entryPoint = "ospl_spliced";

    if (u_splicedInProcess()) {
        result = U_RESULT_PRECONDITION_NOT_MET;
    } else {
        result = U_RESULT_OK;
    }
    if (result == U_RESULT_OK) {
        mwa = os_malloc(sizeof(struct ut_entryPointWrapperArg));

        /* Initialise the library attributes */
        os_libraryAttrInit(&libraryAttr);
    }
    if (result == U_RESULT_OK) {
        /* Now open the library */
        libraryHandle = os_libraryOpen (spliced, &libraryAttr);
        if (libraryHandle == NULL) {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR,"user::u_domain::startSplicedInProcess",result,
                        "Error opening '%s' library\n", spliced);
        }
    }
    if (result == U_RESULT_OK) {
       /* Writing:
        * mwa->entryPoint = (int(*)(int,char **))
        *      os_libraryGetSymbol(libraryHandle, entryPoint);
        * would seem more natural, but the C99 standard leaves
        * casting from "void *" to a function pointer undefined.
        * The assignment used below is the POSIX.1-2003 (Technical
        * Corrigendum 1) workaround; see the Rationale for the
        * POSIX specification of dlsym().
        */
        *(void **)(&mwa->entryPoint) = os_libraryGetSymbol(libraryHandle, entryPoint);
        if (mwa->entryPoint == NULL) {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR,"user::u_domain::startSplicedInProcess",result,
                        "Error opening '%s' entry point\n", entryPoint);
        }
    }
    if (result == U_RESULT_OK) {
        os_threadAttrInit(&threadAttr);

        mwa->argc = 2;
        mwa->argv = os_malloc((unsigned)(mwa->argc + 1)*sizeof(char *));
        mwa->argv[0] = spliced;
        mwa->argv[1] = (char*)uri;
        mwa->argv[2] = NULL;

        /* Invoke the spliced entry point as a new thread in the current process */
        osr = os_threadCreate(tid, spliced, &threadAttr, ut_entryPointWrapper, mwa);
        if (osr != os_resultSuccess) {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR,"user::u_domain::startSplicedInProcess",result,
                        "Error starting thread for '%s'\n", spliced);
        }
    }
    return result;
}

static void
report_startup_message(
    C_STRUCT(u_domainConfig) *domainCfg,
    c_ulong systemId)
{
    char addrStr[100];
    char infoStr[1024];

    os_sprintf(addrStr, "0x" PA_ADDRFMT, (PA_ADDRCAST)domainCfg->address);
    os_sprintf(infoStr,
                "%s"
                "Domain (id)          : %s (%u)" OS_REPORT_NL
                "Storage              : %d Kbytes" OS_REPORT_NL
                "Storage threshold    : %d Kbytes" OS_REPORT_NL
                "Storage address      : %s" OS_REPORT_NL
                "Locking              : %s" OS_REPORT_NL
                "Memory mode          : %s" OS_REPORT_NL
                "Builtin topics       : %s" OS_REPORT_NL
                "Priority inheritance : %s" OS_REPORT_NL
                "SystemId             : %u from [%u,%u]%s",
                "---------------------------------------------------------------" OS_REPORT_NL
                "-- The service is using the following configuration settings --" OS_REPORT_NL
                "---------------------------------------------------------------" OS_REPORT_NL,
                domainCfg->name,
                domainCfg->id,
                domainCfg->dbSize/1024,
                domainCfg->dbFreeMemThreshold/1024,
                (os_serviceGetSingleProcess())         ? "Not available" : addrStr,
                (domainCfg->lockPolicy == OS_LOCKED)   ? "true"          :
                (domainCfg->lockPolicy == OS_UNLOCKED) ? "false"         : "default",
                domainCfg->heap                        ? "Heap memory"   : "Shared memory",
                domainCfg->builtinTopicEnabled         ? "true"          : "false",
                domainCfg->prioInherEnabled            ? "true"          : "false",
                systemId, domainCfg->systemIdConfig.min, domainCfg->systemIdConfig.max,
                domainCfg->systemIdConfig.entropy ? " (user entropy source present)" : "");
    OS_REPORT_WID(OS_INFO,"The OpenSplice domain service", 0, domainCfg->id, "%s", infoStr);
}

/**************************************************************
 * Public functions
 **************************************************************/

static u_result u__domainDeinitW(void *_this)
{
    OS_UNUSED_ARG(_this);
    assert(0);
    return U_RESULT_OK;
}

static void u__domainFreeW(void *_this)
{
    OS_UNUSED_ARG(_this);
    assert(0);
}

u_result
u_domainNew(
    u_domain *domain,
    const os_char *uri)
{
    os_sharedHandle  shm;
    os_sharedAttr    shm_attr;
    os_address       sharedMemAddress = 0;
    c_base           base;
    v_kernel         kernel = NULL;
    u_result         result;
    cf_element       processConfig;
    v_configuration  configuration;
    v_cfElement      rootElement;
    v_processInfo    procInfo;
    C_STRUCT(u_domainConfig) domainCfg;
#ifdef INCLUDE_PLUGGABLE_REPORTING
    c_iter           reportPlugins = NULL;
#endif
#ifndef INTEGRITY
    os_result        sharedMemLock = os_resultUnavailable;
#endif

    assert(domain != NULL);
    *domain = NULL;

    base = NULL;
    processConfig = NULL;
    shm = NULL;
    os_sharedAttrInit(&shm_attr);
    domainCfg.name = NULL;

    result = u_userInitialise();
    if (result != U_RESULT_OK) {
        OS_REPORT(OS_ERROR, "u_domainNew",result,
                    "implicit u_userInitialise failed, result = %s, uri = %s",
                    u_resultImage(result), uri ? uri : "NULL");
    }
/*
 * Initialize a temporary working set of default configuration values.
 */
    if (result == U_RESULT_OK) {
        domainCfg.lockPolicy = OS_LOCK_DEFAULT;
        domainCfg.builtinTopicEnabled = TRUE;
        domainCfg.prioInherEnabled = FALSE;
        domainCfg.id = U_DOMAIN_ID_DEFAULT;
        domainCfg.address = (os_address) shm_attr.map_address;
        domainCfg.idReadFromConfig = FALSE;
        domainCfg.heap = FALSE;
        domainCfg.dbSize = DATABASE_SIZE;
        domainCfg.dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD;
        domainCfg.maintainObjectCount = 1;
        domainCfg.inProcessExceptionHandling = TRUE;
        domainCfg.name = os_strdup(DOMAIN_NAME);
        domainCfg.systemIdConfig.min = 1;
        domainCfg.systemIdConfig.max = 0x7fffffffu;
        domainCfg.systemIdConfig.entropySize = 0;
        domainCfg.systemIdConfig.entropy = NULL;
        if (domainCfg.name == NULL) {
            result = U_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR, "user::u_domain::u_domainNew",result,
                        "memory allocation failed, result = %s, uri = %s",
                        u_resultImage(result), uri ? uri : "NULL");
        }
    }
/*
 * Read the actual Configuration values specified by the URI.
 */
    if ((result == U_RESULT_OK) && (uri != NULL)) {
        if (strlen(uri) > 0) {
            cfgprs_status s;
            s = cfg_parse_ospl(uri, &processConfig);
            if (s == CFGPRS_OK) {
                s = cfg_validateConfiguration(processConfig);
            }
            if (s == CFGPRS_OK) {
                GetDomainConfig(processConfig, &domainCfg, &shm_attr);
                if ((domainCfg.id >= 0 ) && (domainCfg.id <= 230)) {
                    if (processConfigAlreadySet == OS_FALSE) {
                        u_usrClockInit (processConfig);
    #ifdef INCLUDE_PLUGGABLE_REPORTING
                        result = u_usrReportPluginReadAndRegister(processConfig, domainCfg.id, &reportPlugins);
    #endif
                        if (result != U_RESULT_OK) {
                            OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew",result, domainCfg.id,
                                        "ReportPlugin registration failed for Domain %s - return code %d\n",
                                         domainCfg.name, result);
                        }
                        processConfigAlreadySet = OS_TRUE;
                    }
                } else {
                    result = U_RESULT_ILL_PARAM;
                    OS_REPORT(OS_ERROR, "user::u_domain::u_domainNew", result,
                                "Invalid value for domainId in configuration from URI: \"%s\". Valid values are from 0 to 230, value used = %d", uri, domainCfg.id );
                }
            } else {
                result = U_RESULT_ILL_PARAM;
                OS_REPORT(OS_ERROR, "user::u_domain::u_domainNew", result,
                            "Syntax error in configuration from URI: \"%s\".",uri);
            }
        } else {
            result = U_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "user::u_domain::u_domainNew", result,
                        "Invalid URI specified: \"%s\".",uri);
        }
    }
/*
 * Sanity check, a domain can only be create once so if it already exists this
 * operation is already executed before by this process and this call will be aborted.
 */
    if (result == U_RESULT_OK) {
        u_domain dom;
        if ((dom = u_userLookupDomain(domainCfg.id)) != NULL) {
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew",result, domainCfg.id,
                        "Operation called multiple times, result = %s, uri = %s",
                        u_resultImage(result), uri ? uri : "NULL");
            (void)u_domainClose(dom);
        }
    }
/*
 * Initialize (shared) memory configuration.
 */
    if (result == U_RESULT_OK) {
        if (domainCfg.heap == TRUE) {
            os_serviceSetSingleProcess();
        } else {
            /* start for windows the named pipe : only required when multiple
             * processes require communication.
             */
            if (os_serviceStart(domainCfg.name) == os_resultSuccess)
            {
                shm = os_sharedCreateHandle(domainCfg.name, &shm_attr, domainCfg.id);
                if (shm == NULL) {
                    result = U_RESULT_UNDEFINED;
                    OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew", result, domainCfg.id,
                                "os_sharedCreateHandle failed for domain %s.",
                                domainCfg.name);
                }
            }
            else
            {
                result = U_RESULT_UNDEFINED;
                OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew", result, domainCfg.id,
                            "Create service pipe failed for domain %s, "
                            "the service pipe already exists.",
                            domainCfg.name);
            }

            if (result == U_RESULT_OK) {
                os_result osr;
#ifndef INTEGRITY
                sharedMemLock = os_sharedMemoryLock(shm);
                osr = os_sharedMemoryAttach(shm);
                if (osr == os_resultSuccess) {
                    /* Detected an existing shared memory segment.
                     * Not safe to continue so bail out.
                     */
                    os_sharedMemoryDetach(shm);
                    result = U_RESULT_PRECONDITION_NOT_MET;
                    OS_REPORT_WID(OS_WARNING, "user::u_domain::u_domainNew", result, domainCfg.id,
                                "Create shared memory segment failed for domain %s, "
                                "the segment already exists.",
                                domainCfg.name);
                }
                else
#endif
                {
                    osr = os_sharedMemoryCreate(shm, domainCfg.dbSize);
                    if (osr != os_resultSuccess) {
                        os_sharedDestroyHandle(shm);
                        shm = NULL;
                        PRINT_THREAD_MESSAGE("u_domainNew");
                        result = U_RESULT_UNDEFINED;
                        OS_REPORT_WID(OS_ERROR, "user::u_domainNew",result,domainCfg.id,
                                    "os_sharedMemoryCreate for " PA_SIZEFMT
                                    " bytes failed for domain %s.",
                                    domainCfg.dbSize, domainCfg.name);
                    }
                }
                if (result == U_RESULT_OK) {
                    /* Ignore any message that was generated */
                    IGNORE_THREAD_MESSAGE;
                    osr = os_sharedMemoryAttach(shm);
                    PRINT_THREAD_MESSAGE("u_domainNew");
                    if (osr != os_resultSuccess) {
                        result = U_RESULT_UNDEFINED;
                        OS_REPORT_WID(OS_ERROR, "user::u_domainNew",result, domainCfg.id,
                                    "os_sharedMemoryAttach failed for domain %s.",
                                    domainCfg.name);
                        osr = os_sharedMemoryDestroy(shm);
                        if (osr != os_resultSuccess) {
                            OS_REPORT(OS_ERROR, "user::u_domainNew", result,
                                      "os_sharedMemoryDestroy failed for domain %s.",
                                      domainCfg.name);
                        }
                        os_sharedDestroyHandle(shm);
                        shm = NULL;
                    } else {
                        if (domainCfg.lockPolicy == OS_LOCKED) {
                            if (lockSharedMemory(shm) != 0) {
                                result = U_RESULT_UNDEFINED;
                                OS_REPORT_WID(OS_ERROR, "user::u_domainNew",result,domainCfg.id,
                                            "lockSharedMemory failed for domain %s.",
                                            domainCfg.name);
                            }
                        }
                        if (result != U_RESULT_OK) {
                            os_sharedMemoryDetach(shm);
                            os_sharedMemoryDestroy(shm);
                            os_sharedDestroyHandle(shm);
                            shm = NULL;
                        } else {
                            sharedMemAddress = (os_address)os_sharedAddress(shm);
                        }
                    }
                }
            }
        }
    }
/*
 * Create Domain Database in initialized memory.
 */
    if (result == U_RESULT_OK) {
        base = c_create(DATABASE_NAME,
                        (c_voidp)sharedMemAddress,
                        domainCfg.dbSize,
                        domainCfg.dbFreeMemThreshold);
        if (base == NULL) {
            result = U_RESULT_INTERNAL_ERROR;
        }
    }
#ifndef INTEGRITY
    /*
     * succesfull created file handle is 0 or greater. Failed situation gives -1
     * If file handle isn't -2 anymore, it is used at creation. Always delete to
     * prevent stale files.
     */
    if (sharedMemLock != os_resultUnavailable) {
        os_sharedMemoryUnlock(shm);
    }
#endif
/*
 * Initialize Domain administration (Kernel) in the Domain Database in disabled state. Attaching processes
 * will be blocked until enabled.
 */
    if (result == U_RESULT_OK) {
        C_STRUCT(v_kernelQos) kernelQos;
        c_baseSetMaintainObjectCount (base, domainCfg.maintainObjectCount);
        kernelQos.builtin.v.enabled = domainCfg.builtinTopicEnabled;
        kernelQos.systemIdConfig = domainCfg.systemIdConfig;
        kernel = v_kernelNew(base, KERNEL_NAME, &kernelQos, &procInfo);
        if (kernel == NULL) {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew",result,domainCfg.id,
                        "v_kernelNew failed for domain %s.",
                        domainCfg.name);
        } else {
            os_uint32 serial;

            serial = pa_add32_nv(&domainSerial, 0x100);
            assert(domainCfg.id < 0xFF);
            assert((serial & 0xFF) == 0);
            procInfo->serial = serial | (c_ulong)domainCfg.id;
        }
    }
/*
 * Copy configuration to Domain administration (Kernel).
 */
    if (result == U_RESULT_OK) {
        configuration = v_configurationNew(kernel);
        if (configuration == NULL) {
            result = U_RESULT_OUT_OF_MEMORY;
            OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew",result,domainCfg.id,
                        "initialization of configuration admin failed for domain %s.",
                        domainCfg.name);
        }
        if (result == U_RESULT_OK) {
            rootElement = NULL;
            result = copyConfiguration(cf_node(processConfig),
                                  configuration,
                                  (v_cfNode *)&rootElement);
            if (result != U_RESULT_OK) {
                OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainNew",result,domainCfg.id,
                            "initialization of configuration admin failed for domain %s.",
                            domainCfg.name);
            }
        }
        if (result == U_RESULT_OK) {
            if (rootElement != NULL) {
                v_configurationSetUri(configuration, uri);
                v_configurationSetRoot(configuration, rootElement);
                result = u_resultFromKernel(v_kernelConfigure(kernel, configuration));
            } else {
                v_configurationFree(configuration);
                OS_REPORT_WID(OS_WARNING,
                            "Create Domain Admin (u_domainNew)",0,domainCfg.id,
                            "No configuration specified for this domain." OS_REPORT_NL
                            "The default configuration will be used.\nDomain      : \"%s\"",
                            domainCfg.name);
            }
        }
    }
/*
 * Enable the kernel, so attaching processes will proceed with a config loaded kernel.
 */
    if (result == U_RESULT_OK) {
        v_kernelEnable(kernel, KERNEL_NAME);
    }


/*
 * Create a local Domain proxy interface.
 */
    if (result == U_RESULT_OK) {
        *domain = u_objectAlloc(sizeof(**domain), U_DOMAIN, u__domainDeinitW, u__domainFreeW);
        u_userSetupSignalHandling(FALSE);
        result = u_entityInit(u_entity(*domain), v_entity(kernel), *domain);
        os_mutexInit(&(*domain)->mutex, NULL);
        os_condInit(&(*domain)->cond, &(*domain)->mutex, NULL);
        os_mutexInit(&(*domain)->deadlock, NULL);
        os_mutexLock(&(*domain)->deadlock); /* Don't unlock; deadlocks on purpose. */
        (*domain)->openCount = 1;
        (*domain)->closing = 0;
        pa_st32(&(*domain)->refCount, 1);
        (*domain)->kernel = kernel;
        (*domain)->shm = shm;
        (*domain)->participants = NULL;
        (*domain)->waitsets = NULL;
        (*domain)->lockPolicy = domainCfg.lockPolicy;
        (*domain)->procInfo = procInfo;
        (*domain)->serial = procInfo->serial;
        (*domain)->inProcessExceptionHandling = domainCfg.inProcessExceptionHandling;
        (*domain)->y2038Ready = c_baseGetY2038Ready(c_getBase(kernel));

        if (uri == NULL) {
            (*domain)->uri = NULL;
        } else {
            (*domain)->uri = os_strdup(uri);
        }
        (*domain)->name = os_strdup(domainCfg.name);
        (*domain)->id = domainCfg.id;
        (*domain)->owner = TRUE;
        pa_st32(&(*domain)->state, U_DOMAIN_STATE_ALIVE);
        pa_st32(&(*domain)->claimed, 0);
        (*domain)->isService = TRUE;
        (*domain)->threadWithAccess = OS_THREAD_ID_NONE;
#ifdef INCLUDE_PLUGGABLE_REPORTING
        (*domain)->reportPlugins = reportPlugins;
            reportPlugins = NULL;
#endif

        u_userAddDomain(*domain);
        report_startup_message(&domainCfg, kernel->GID.systemId);

        if((*domain)->inProcessExceptionHandling == FALSE){
            OS_REPORT_WID(OS_INFO,"The OpenSplice domain service", 0, domainCfg.id,
                    "In-process exception handling has been disabled.");
        }
    }
    if ((result != U_RESULT_OK) && (shm != NULL)) {
        /* Don't detach or destroy shared memory when another domain has
         * created it. */
        if (result != U_RESULT_PRECONDITION_NOT_MET) {
            os_sharedMemoryDetach(shm);
            os_sharedMemoryDestroy(shm);
        }
        os_sharedDestroyHandle(shm);
    }
    if (processConfig != NULL) {
        cf_elementFree(processConfig);
    }
    if (domainCfg.name != NULL) {
        os_free(domainCfg.name);
    }
#ifdef INCLUDE_PLUGGABLE_REPORTING
    if (reportPlugins != NULL) {
       /* report plugins not registered on domain ? */
        u_usrReportPluginUnregister(reportPlugins);
        c_iterFree(reportPlugins);
    }
#endif
    return result;
}

static void
onSharedMemoryServerDied(
    os_sharedHandle sharedHandle,
    void *args)
{
    u_domain domain = (u_domain)args;
    os_uint32 idx;

    OS_UNUSED_ARG(sharedHandle);

    assert(domain);
    assert(sharedHandle == domain->shm);

    if (!domain->isService) {
        OS_REPORT(OS_INFO,
                  "user::u_domain::onSharedMemoryServerDied", 0,
                  "Shared memory server not running anymore, detaching from domain \"%s\".", domain->name);
        idx = u__userDomainIndex(domain);
        if (idx > 0) {
            u_result result;
            result = u__userDomainDetach(idx, U_USER_DELETE_ENTITIES);
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR,
                          "user::u_domain::onSharedMemoryServerDied",result,
                          "Detaching from domain failed, result = %s", u_resultImage(result));
            }
        }
    } else {
        OS_REPORT(OS_INFO,
                  "user::u_domain::onSharedMemoryServerDied", 0,
                  "Shared memory server not running anymore for domain \"%s\".", domain->name);
    }
}


static void
u__userDetachWrapper(void)
{
    u_userDetach(U_USER_DELETE_ENTITIES);
}

u_result
u__domainOpen(
    u_domain *domain,
    const os_char *uri,
    const u_domainId_t id,
    const u_bool isService,
    os_int32 timeout)
{
    os_sharedHandle  shm = NULL;
    os_sharedAttr    shm_attr;
    u_result         result;
    cf_element       processConfig;
    os_threadId      spliced_thread = OS_THREAD_ID_NONE;
    C_STRUCT(u_domainConfig) domainCfg;
    v_processInfo    procInfo;
#ifdef INCLUDE_PLUGGABLE_REPORTING
    /** @todo - Fix properly. See: OSPL-1222 */
    static os_boolean singleProcessLoggerRegDoneHack = OS_FALSE;
    c_iter reportPlugins = NULL;
#endif
    os_char *name;
    os_char *config_source = "specified by process";
    assert(domain != NULL);
    *domain = NULL;

/*
 * Initialize a temporary working set of default configuration values.
 */
    if ( (id != U_DOMAIN_ID_ANY) && ((id < 0) || (id > 230))) {
        OS_REPORT(OS_ERROR, "user::u_domain::u_domainOpen",U_RESULT_ILL_PARAM,
                    "Invalid value for domainId passed to domain open. Valid values are from 0 to 230, value used = %d", id );
        return U_RESULT_ILL_PARAM;
    }
    domainCfg.lockPolicy = OS_LOCK_DEFAULT;
    domainCfg.builtinTopicEnabled = TRUE;
    domainCfg.prioInherEnabled = FALSE;
    domainCfg.id = id;
    domainCfg.address = 0; /* NULL */
    domainCfg.idReadFromConfig = FALSE;
    domainCfg.heap = FALSE;
    /* DATABASE_SIZE is default in shm mode.
     * In singleprocess mode this is set back to 0 as default if no size is configured*/
    domainCfg.dbSize = DATABASE_SIZE;
    domainCfg.dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD;
    domainCfg.inProcessExceptionHandling = TRUE;
    domainCfg.name = os_strdup(DOMAIN_NAME);
    domainCfg.systemIdConfig.min = 1;
    domainCfg.systemIdConfig.max = 0x7fffffffu;
    domainCfg.systemIdConfig.entropySize = 0;
    domainCfg.systemIdConfig.entropy = NULL;
    if (domainCfg.name == NULL) {
        OS_REPORT(OS_ERROR, "user::u_domain::u_domainOpen",U_RESULT_OUT_OF_MEMORY,
                    "memory allocation failed, result = U_RESULT_OUT_OF_MEMORY");
        return U_RESULT_OUT_OF_MEMORY;
    }

    result = u_userInitialise();
    if (result != U_RESULT_OK) {
        OS_REPORT(OS_ERROR,
                    "user::u_domain::u_domainOpen",result,
                    "u_userInitialise failed, result = %s", u_resultImage(result));
        return result;
    }
    processConfig = NULL;
    os_sharedAttrInit(&shm_attr);

/*
 * If uri is NULL then try to get uri from environment
 */
    if (uri == NULL) {
        uri = os_getenv ("OSPL_URI");
    }
    if (uri) {
        if (strlen(uri) == 0) {
            uri = NULL;
        } else {
            config_source = "specified by OSPL_URI";
        }
    }
#ifdef CONF_PARSER_NOFILESYS
    if ( uri == NULL )
    {
       uri = os_strdup("osplcfg://ospl.xml");
    }
#endif

/*
 * If a uri is specified then read the actual configuration.
 */
    if (result == U_RESULT_OK) {
        if (uri != NULL) {
            cfgprs_status s;
            /* Do we have to free processConfig just like in u_domainNew()??? */
            s = cfg_parse_ospl(uri, &processConfig);
            if (s == CFGPRS_OK) {
                GetDomainConfig(processConfig, &domainCfg, &shm_attr);
                if ( (domainCfg.id == U_DOMAIN_ID_ANY) || ( (domainCfg.id >= 0) && (domainCfg.id <= 230))) {
                    if (processConfigAlreadySet == OS_FALSE) {
                       u_usrClockInit (processConfig);
                        if (domainCfg.prioInherEnabled) {
                            os_mutexSetPriorityInheritanceMode(OS_TRUE);
                        }
    #ifdef INCLUDE_PLUGGABLE_REPORTING
                        /** @todo - Fix properly. See: OSPL-1222 */
                        if (!domainCfg.heap || singleProcessLoggerRegDoneHack == OS_FALSE) {
                            result = u_usrReportPluginReadAndRegister(processConfig, domainCfg.id, &reportPlugins);
                            if (result != U_RESULT_OK) {
                                OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                                            "ReportPlugin registration failed for Domain %s - return code %d\n",
                                             domainCfg.name, result);
                            }
                        }
    #endif
                        processConfigAlreadySet = OS_TRUE;
                    }
                } else {
                    result = U_RESULT_ILL_PARAM;
                    OS_REPORT(OS_ERROR, "user::u_domain::u_domainNew", result,
                                "Invalid value for domainId in configuration from URI: \"%s\". Valid values are from 0 to 230, value used = %d", uri, domainCfg.id );
                }
                cf_elementFree(processConfig);
            } else {
                result = U_RESULT_ILL_PARAM;
                OS_REPORT(OS_ERROR, "user::u_domain::u_domainOpen",result,
                            "Error in configuration file \'%s\' %s\n",
                             uri, config_source);
            }
        } else {
            /* No uri specified so domainCfg.id could not be found so reset it to any. */
            domainCfg.id = U_DOMAIN_ID_ANY;
        }
    }

    if (result == U_RESULT_OK) {
        if (id == U_DOMAIN_ID_ANY) {
            /* Any id is ok to connect to. */
            if (domainCfg.id == U_DOMAIN_ID_ANY) {
                /* domain not specified neither by parameter nor by configuration.
                 * So use default domain.
                 */
                domainCfg.id = U_DOMAIN_ID_DEFAULT;
            } else {
                /* A domain id is found in the given configuration
                 * so will connect to that domain.
                 */
            }
        } else {
            /* A specific domain id is requested. */
            if (domainCfg.id != id) {
                /* did not yet found a domain configuration for the requested id. */
                /* So create a temporary shm handle and try to discover the id in running domains. */
                domainCfg.id = id;
                shm = os_sharedCreateHandle(domainCfg.name, &shm_attr,domainCfg.id);
                if (os_sharedMemoryGetNameFromId(shm,&name) == os_resultSuccess) {
                    if ((name != NULL) && (strlen(name)>0)){
                        os_free(domainCfg.name);
                        domainCfg.name = name;
                    } else {
                        os_free(name);
                    }
                } else {
                    /* The requested domain id is also not found in running domains
                     * so cannot connect to the domain.
                     */
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
                (void)os_sharedDestroyHandle(shm);
            }
        }
    }

/*
 * Check if the domain is already attached to the process.
 */
    if (result == U_RESULT_OK) {
        *domain = u_userLookupDomain(domainCfg.id);
        if (*domain != NULL) {
            if ((*domain)->kernel->splicedRunning == FALSE) {
                /* Found domain is not running, mark as state
                 * and detach from shared memory. */
                 result = u__userDomainDetach(u__userDomainIndex(*domain), TRUE);
                 if (result != U_RESULT_OK) {
                     OS_REPORT_WID(OS_INFO,
                               "user::u_domain::u_domainOpen",result,domainCfg.id,
                               "Detaching from already attached domain failed, result = %s", u_resultImage(result));
                     result = U_RESULT_OK; /* Ignore failure for now. */
                 }
                *domain = NULL;
            } else {
                os_free(domainCfg.name);
                return U_RESULT_OK;
            }
        }
    }
/*
 * Start attaching to the domain.
 */
    if (result == U_RESULT_OK) {
        if (domainCfg.prioInherEnabled) {
            os_mutexSetPriorityInheritanceMode(OS_TRUE);
        }
        if (domainCfg.heap) {
/*
 * Startup Single process Domain.
 */
            result = startSplicedWithinProcess(&spliced_thread, uri);
            if (result == U_RESULT_OK) {
                int count = 0;
                os_duration delay_100ms = OS_DURATION_INIT(0, 100000000);

                /* wait for up to 5 seconds for the domain to be available in this process */
                while (count < 50) {
                    os_sleep(delay_100ms);
                    if (*domain == NULL) {
                        *domain = u_userLookupDomain(domainCfg.id);
                    }
                    if ((*domain != NULL) && (count > 10)) {
                        os_free(domainCfg.name);
                        (*domain)->spliced_thread = spliced_thread;
#ifdef INCLUDE_PLUGGABLE_REPORTING
                        (*domain)->reportPlugins = reportPlugins;
#endif
                        return U_RESULT_OK;
                    }
                    count++;
                }
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT_WID(OS_ERROR,"user::u_domain::u_domainOpen",result,domainCfg.id,
                            "Domain '%s' has not been started successfully\n", domainCfg.name);
            } else {
                OS_REPORT_WID(OS_ERROR,"user::u_domain::u_domainOpen",result,domainCfg.id,
                            "Starting Domain '%s' failed, result = %s\n",
                            domainCfg.name, u_resultImage(result));
            }
        } else {
/*
 * Attach to federated Domain (shared memory connection).
 */

           /* On a regular exit of an application, all SHM entities should
            * be properly cleaned up (and more importantly, there should be
            * no more threads accessing SHM). This is a fallback case for
            * when the application doesn't properly clean up the resources.
            *
            * This only needs to be done at first connection to a SHM domain,
            * but that's a minor optimisation. */
            os_procAtExit(&u__userDetachWrapper);

            /* set pipename for windows */
            os_createPipeNameFromDomainName(domainCfg.name);
            shm = os_sharedCreateHandle(domainCfg.name, &shm_attr,domainCfg.id);
            if (shm == NULL) {
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                          "Operation os_sharedCreateHandle failed");
            }
        }
/*
 * Try to attach to existing shared memory segment.
 */
        if (result == U_RESULT_OK) {
            os_duration pollDelay = OS_DURATION_INIT(1,0);
            os_result osr;

            osr = os_sharedMemoryAttach(shm);
            IGNORE_THREAD_MESSAGE;
            while ((timeout > 0) && (osr != os_resultSuccess)) {
                os_sleep(pollDelay);
                osr = os_sharedMemoryAttach(shm);
                timeout--;
                PRINT_THREAD_MESSAGE("u_domainOpen");
            }
            if (osr != os_resultSuccess) {
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT_WID(OS_ERROR,
                            "user::u_domain::u_domainOpen",result,domainCfg.id,
                            "os_sharedMemoryAttach failed for domainId (%d) and domainName (%s)",
                            domainCfg.id, domainCfg.name?domainCfg.name:"No name specified");
            }
/*
 * Try to open existing Domain Database.
 */
            if (result == U_RESULT_OK) {
                c_base base;
                v_kernel kernel;

                base = c_open(DATABASE_NAME, os_sharedAddress(shm));
                while ((timeout > 0) && (base == NULL)) {
                    os_sleep(pollDelay);
                    base = c_open(DATABASE_NAME, os_sharedAddress(shm));
                    timeout--;
                }
                if (base == NULL) {
                    result = U_RESULT_INTERNAL_ERROR;
                    OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                              "Creation of internal Domain Database failed.");
                }
/*
 * Try to attach to existing Domain administration (kernel).
 */
                if (result == U_RESULT_OK) {
                    kernel = v_kernelAttach(base, KERNEL_NAME, &procInfo);
                    while ((timeout > 0) && (kernel == NULL)) {
                        os_sleep(pollDelay);
                        kernel = v_kernelAttach(base, KERNEL_NAME, &procInfo);
                        timeout--;
                    }
                    if (kernel == NULL) {
                        result = U_RESULT_INTERNAL_ERROR;
                        OS_REPORT_WID(OS_ERROR,
                                  "user::u_domain::u_domainOpen", result,domainCfg.id,
                                  "v_kernelAttach failed");
                    } else {
                        os_uint32 serial;

                        serial = pa_add32_nv(&domainSerial, 0x100);
                        assert(domainCfg.id < 0xFF);
                        assert((serial & 0xFF) == 0);
                        procInfo->serial = serial | (c_ulong)domainCfg.id;
                    }
                }

/*
 * Try to find uri from shared memory when not set.
 */
                if ((result == U_RESULT_OK) &&
                    (uri == NULL)) {
                    v_configuration config = v_getConfiguration(kernel);
                    uri = v_configurationGetUri(config);
                    if (uri != NULL && (strlen(uri) > 0)) {
                        cfgprs_status s;
                        s = cfg_parse_ospl(uri, &processConfig);
                        if (s == CFGPRS_OK) {
#ifdef INCLUDE_PLUGGABLE_REPORTING
                            /** @todo - Fix properly. See: OSPL-1222 */
                            if (singleProcessLoggerRegDoneHack == OS_FALSE) {
                                result = u_usrReportPluginReadAndRegister(processConfig, domainCfg.id, &reportPlugins);
                                if (result != U_RESULT_OK) {
                                    OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                                                "ReportPlugin registration failed for Domain %s - return code %d\n",
                                                 domainCfg.name, result);
                                }
                            }
#endif
                            cf_elementFree(processConfig);
                        }
                    }
                }
/*
 * Create a local Domain proxy interface.
 */
                if (result == U_RESULT_OK) {
                    *domain = u_objectAlloc(sizeof(**domain), U_DOMAIN, u__domainDeinitW, u__domainFreeW);
                    if (*domain == NULL) {
                        result = U_RESULT_OUT_OF_MEMORY;
                        OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                                    "initialization of configuration admin failed for domain %s.",
                                    domainCfg.name);
                    } else {
                        u_userSetupSignalHandling(!isService);
                        result = u_entityInit(u_entity(*domain), v_entity(kernel), *domain);
                        os_mutexInit(&(*domain)->mutex, NULL);
                        os_condInit(&(*domain)->cond, &(*domain)->mutex, NULL);
                        os_mutexInit(&(*domain)->deadlock, NULL);
                        os_mutexLock(&(*domain)->deadlock); /* Don't unlock; deadlocks on purpose. */
                        (*domain)->openCount = 1;
                        (*domain)->closing = 0;
                        pa_st32(&(*domain)->refCount, 1);
                        (*domain)->kernel = kernel;
                        (*domain)->shm = shm;
                        (*domain)->procInfo = procInfo;
                        (*domain)->protectCount = 0;
                        (*domain)->serial = procInfo->serial;
                        (*domain)->participants = NULL;
                        (*domain)->waitsets = NULL;
                        (*domain)->lockPolicy = OS_LOCK_DEFAULT; /* don't care! */
                        (*domain)->inProcessExceptionHandling = domainCfg.inProcessExceptionHandling;
                        (*domain)->y2038Ready = c_baseGetY2038Ready(c_getBase(kernel));
                        (*domain)->id = domainCfg.id;
                        (*domain)->owner = FALSE;
                        pa_st32(&(*domain)->state, U_DOMAIN_STATE_ALIVE);
                        pa_st32(&(*domain)->claimed, 0);
                        (*domain)->isService = isService;
                        (*domain)->spliced_thread = spliced_thread;
#ifdef INCLUDE_PLUGGABLE_REPORTING
                        (*domain)->reportPlugins = reportPlugins;
#endif
                        /* Recheck 'uri', maybe it is retrieved from v_configuration */
                        if (uri != NULL) {
                            (*domain)->uri = os_strdup(uri);
                        } else {
                            (*domain)->uri = NULL;
                        }
                        (*domain)->name = os_strdup(domainCfg.name);
                        u_userAddDomain(*domain);
                    }
                }
                if (result != U_RESULT_OK) {
                    (void)os_sharedMemoryDetach(shm);
                }
            }
            if (result != U_RESULT_OK) {
                (void)os_sharedDestroyHandle(shm);
            }
        }
    }

    if (result == U_RESULT_OK) {
        os_result osres = os_sharedMemoryRegisterServerDiedCallback(shm, onSharedMemoryServerDied, *domain);
        if (osres != os_resultSuccess && osres != os_resultUnavailable) {
            OS_REPORT_WID(OS_ERROR, "user::u_domain::u_domainOpen",result,domainCfg.id,
                      "Failed to register server died callback for domain %s.",
                      domainCfg.name);
        }
    }

    if (domainCfg.name != NULL) {
        os_free(domainCfg.name);
    }

    return result;
}

u_result
u_domainOpen(
    u_domain *domain,
    const os_char *uri,
    const u_domainId_t id,
    os_int32 timeout)
{
    return u__domainOpen(domain,uri,id,FALSE,timeout);
}

u_result
u_domainOpenForService(
    u_domain *domain,
    const os_char *uri,
    const u_domainId_t id,
    os_int32 timeout)
{
    return u__domainOpen(domain,uri,id,TRUE,timeout);
}

static void
u__domainWakeThreads (
    _In_ u_domain _this)
{
    assert(os_threadIdToInteger(_this->threadWithAccess) == os_threadIdToInteger(os_threadIdSelf()));

    v_processInfoWakeThreads(_this->procInfo);
}

static void
u__domainReportThreads (
    _In_ u_domain _this)
{
    assert(os_threadIdToInteger(_this->threadWithAccess) == os_threadIdToInteger(os_threadIdSelf()));

    v_processInfoReportThreads(_this->procInfo);
}

#ifndef NDEBUG
static void u_domainCloseTimeout (c_ulong protectCount)
{
    fprintf (stderr, "u_domainCloseTimeout: %u threads did not detach\n", protectCount);
}
#endif

static void u__domainDeinitWait (_In_ u_domain _this)
{
    const os_duration relTimeout = OS_DURATION_INIT(10,0);
    const os_timeM absTimeout = os_timeMAdd(os_timeMGet(), relTimeout);
#ifndef NDEBUG
    c_ulong lProtectCount = 0;
#endif
    c_ulong protectCount;
    u_bool first = TRUE;

    /* Check the claimed counter which indicates if a thread is currently
     * performing a u_domainProtect call. The u_domainProtect will access
     * the processInfo which will become invalid when the shared memory
     * is detached from the process.
     */
    while (pa_ld32(&_this->claimed) > 0) {
        static const os_duration d = OS_DURATION_INIT(0, 10 * 1000 * 1000); /* 0.01 s */
        os_sleep(d);
    }

    protectCount = u_domainProtectCount(_this);
    while (protectCount > 0 && os_timeMCompare(os_timeMGet(), absTimeout) != OS_MORE) {
        static const os_duration pollPeriod = OS_DURATION_INIT(0,100 * 1000 * 1000); /* 0.1 s */
#ifndef NDEBUG
        if (protectCount != lProtectCount) {
            fprintf(stderr, "%d: u_domainClose: waiting for %u threads to detach\n", os_procIdSelf(), protectCount);
            lProtectCount = protectCount;
        }
#endif
        /* First time don't try to wake the threads. This has already happened,
         * but we didn't yield our processing yet. */
        if(!first && (pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_KERNEL)) {
            u__domainWakeThreads(_this);
        }
        first = FALSE;
        os_sleep(pollPeriod);
        protectCount = u_domainProtectCount(_this);
    }
    if (protectCount > 0) {
        OS_REPORT(OS_WARNING,"u_domainClose", 0, "%u threads did not detach from domain \"%s\" (%u).", protectCount, _this->name, _this->id);
        u__domainReportThreads(_this);
#ifndef NDEBUG
        u_domainCloseTimeout(protectCount);
#endif
    } else if (os_threadIdToInteger(_this->spliced_thread) != os_threadIdToInteger(OS_THREAD_ID_NONE)) {
        /* If spliced is in-process and we timed out, we have no idea whether or not spliced will
           terminate or not. Calling threadWaitExit unconditionally would therefore defeat the
           purpose of the timeout */
        (void) os_threadWaitExit(_this->spliced_thread, NULL);
    }
}

static void
u__domainDeadlock(
    _Inout_ u_domain _this)
{
    os_mutexLock(&_this->deadlock);
    /* This line is not reached, since the lock is always in locked state. */
    os_mutexUnlock(&_this->deadlock);
}

static os_uint32
u__domainThreadProtectCount (
    _In_ u_domain _this)
{
    return v_kernelThreadProtectCount(_this->serial);
}


_Must_inspect_result_
static u_bool
u__domainMustDelete(
    _Inout_ u_domain _this,
    _In_ os_uint32 isDetach)
{
    u_bool mustDelete;

    os_mutexLock(&_this->mutex);
    if ((--_this->openCount == 0 && !(pa_ld32(&_this->state) & (U_DOMAIN_STATE_DETACHING | U_DOMAIN_STATE_DELETE)))
            || (isDetach && !(pa_ld32(&_this->state) & U_DOMAIN_STATE_DELETE))) {
        _this->threadWithAccess = os_threadIdSelf();
        _this->closing = 1;
        pa_or32(&_this->state, U_DOMAIN_STATE_DELETE);

        if (c_iterLength(_this->participants) != 0) {
            OS_REPORT(OS_INFO,
                      "user::u_domain::u__domainMustDelete", os_resultBusy,
                      "note: %u participants still connected to domain \"%s\" (%u).",
                      c_iterLength(_this->participants),
                      _this->name, _this->id);
        }

        mustDelete = TRUE;
    } else {
        mustDelete = FALSE;
    }
    os_mutexUnlock(&_this->mutex);

    return mustDelete;
}

static u_result
u__domainDelete(
    _Inout_ u_domain _this)
{
    u_result r = U_RESULT_OK;
    c_bool clean = TRUE;
    os_result osResult;

    assert(os_threadIdToInteger(_this->threadWithAccess) == os_threadIdToInteger(os_threadIdSelf()));

    if(pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_KERNEL) {
        u__domainWakeThreads(_this);
    }
    (void) u_userRemoveDomain(_this);
    u__domainDeinitWait(_this);
    {
        v_kernel kernel = _this->kernel;
        os_sharedHandle shm = _this->shm;
        os_uint32 deleteEntities;
        int destroy;

        deleteEntities = pa_ld32(&_this->state) & U_DOMAIN_DELETE_ENTITIES;

        (void)u__entityDeinitW(u_entity(_this));
        if(deleteEntities) {
            if(v_kernelDetach(kernel, os_procIdSelf()) != V_RESULT_OK) {
                /* The process was still accessing shared resources.
                 * Let the shared memory monitor cleanup these resources.
                 */
                clean = FALSE;
            }
        } else if (v_kernelMyProtectCount(kernel) != 0) {
            clean = FALSE;
        }

        if (_this->owner) {
            destroy = (v_kernelUserCount(kernel) == 0);
        } else {
            destroy = 0;
        }

        if (destroy) {
            /* If no users left for the kernel (including the Spliced)
             * then the kernel and database can be destroyed and the
             * memory segment can be detached.
             */
            c_destroy(c_getBase(kernel));
        } else {
            c_mmSuspend(c_baseMM(c_getBase(kernel)));
        }

        if (shm != NULL) {
            if (destroy && _this->lockPolicy == OS_LOCKED) {
                unlockSharedMemory(shm);
            }
            if (clean && deleteEntities) {
                osResult = os_sharedMemoryDetach(shm);
            } else {
                (void)os_sharedMemoryRegisterServerDiedCallback(shm, NULL, NULL);
                osResult = os_sharedMemoryDetachUnclean(shm);
            }
            if (osResult != os_resultSuccess) {
                OS_REPORT(OS_ERROR,
                          "user::u_domain::u_domainClose", osResult,
                          "Operation os_sharedMemoryDetach failed."
                          OS_REPORT_NL "Domain = \"%s\" (%u)"
                          OS_REPORT_NL "result = \"%s\"",
                          _this->name, _this->id,
                          os_resultImage(osResult));
                r = U_RESULT_INTERNAL_ERROR;
            }
            if (destroy) {
                osResult = os_sharedMemoryDestroy(shm);
                if (osResult != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,
                              "user::u_domain::u_domainClose", osResult,
                              "Operation os_sharedMemoryDestroy failed."
                              OS_REPORT_NL "Domain = \"%s\" (%u)"
                              OS_REPORT_NL "result = \"%s\"",
                              _this->name, _this->id,
                              os_resultImage(osResult));
                    r = U_RESULT_INTERNAL_ERROR;
                }
            }
            os_sharedDestroyHandle(shm);
        }
#ifdef INCLUDE_PLUGGABLE_REPORTING
        u_usrReportPluginUnregister(_this->reportPlugins);
#endif
        processConfigAlreadySet = FALSE;
    }

    os_mutexLock(&_this->mutex);
    _this->threadWithAccess = OS_THREAD_ID_NONE;
    os_mutexUnlock(&_this->mutex);

    return clean ? r : U_RESULT_TIMEOUT;
}

u_result
u_domainClose (
    _Inout_ u_domain _this)
{
    u_bool mustDelete;
    u_result r = U_RESULT_OK;

    assert(_this != NULL);

    mustDelete = u__domainMustDelete(_this, FALSE);
    if(mustDelete) {
        /* Set the flag that drives the cleanup behaviour. On closing the last
         * participant entities are normally deleted. */
        pa_or32(&_this->state, U_DOMAIN_DELETE_ENTITIES);
        r = u__domainDelete(_this);
    }

    if(pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_USER) {
        os_uint32 flags;

        flags = u_domainThreadFlags(V_KERNEL_THREAD_FLAGS_GET, V_KERNEL_THREAD_FLAGS_GET);
        if( !(flags & V_KERNEL_THREAD_FLAG_SERVICETHREAD)) {
            (void)u_domainProtectAllowed(_this);
        }
    }

    if(mustDelete) {
        u_domainFree(_this);
    }

    return r;
}

u_result
u_domainFree (
    u_domain _this)
{
    assert(_this != NULL);

    assert(pa_ld32(&_this->refCount) > 0);
    if (pa_dec32_nv(&_this->refCount) != 0) {
        return U_RESULT_OK;
    }

    os_mutexLock(&_this->mutex);
    assert(_this->openCount == 0);

    c_iterFree(_this->participants);
    c_iterFree(_this->waitsets);
#ifdef INCLUDE_PLUGGABLE_REPORTING
    c_iterFree(_this->reportPlugins);
#endif
    os_free(_this->uri);
    os_free(_this->name);
    os_mutexUnlock(&_this->mutex);
    os_mutexDestroy(&_this->mutex);
    os_condDestroy(&_this->cond);
    os_mutexUnlock(&_this->deadlock);
    os_mutexDestroy(&_this->deadlock);
    /* free the domain object (u__observableFreeW ignores the embedded domain pointer, so nothing magic happens here) */
    u__entityFreeW(_this);

    return U_RESULT_OK;
}

os_uint32
u_domainThreadFlags(
    _In_ os_uint32 mask,
    _In_ os_uchar enable)
{
    return v_kernelThreadFlags(mask, enable);
}

u_result
u_domainProtect(
    const u_domain _this)
{
    u_result result = U_RESULT_ALREADY_DELETED;

    /* The state flag is set when the process calls u_domainDetach so that
     * access to the kernel becomes disabled.
     * This call rejects access and returns ALREADY_DELETED when the state flag is set,
     * except for the thread executing the deletion.
     * This test doesn't need to be MT-safe! order of conditions is relevant.
     * The claim counter is used to protect access to the procInfo.
     */
    if (_this) {
        pa_inc32(&_this->claimed);
        if (u_domainProtectAllowedClaimed(_this))
        {
            assert(_this->id == (u_domainId_t)(_this->procInfo->serial & V_KERNEL_THREAD_FLAG_DOMAINID));
            result = u_resultFromKernel(v_kernelProtect(_this->procInfo, &_this->state, U_DOMAIN_BLOCK_IN_KERNEL, &_this->deadlock, _this));
        }
        pa_dec32(&_this->claimed);
    }
    return result;
}

void
u_domainUnprotect(void)
{
    u_domain _this;
    os_uint32 flags;

    _this = (u_domain)v_kernelUnprotect();
    if(_this) {
        flags = u_domainThreadFlags(V_KERNEL_THREAD_FLAGS_GET, V_KERNEL_THREAD_FLAGS_GET);
        if( !(flags & V_KERNEL_THREAD_FLAG_SERVICETHREAD)) {
            (void)u_domainProtectAllowed(_this);
        }
    }
}

os_uint32
u_domainProtectCount(
    _In_ u_domain _this)
{
    os_uint32 pc, bc = 0;

    pc = pa_ld32(&_this->procInfo->protectCount);
    if(pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_KERNEL) {
        assert(pa_ld32(&_this->state) != U_DOMAIN_STATE_ALIVE);
        /* If blockWaits is set, and threads have no access, the protectCount
         * can only go down and waitCount can only go up (but never higher
         * than protectCount). That means we can safely subtract both even
         * though they weren't read atomically. */
         bc = pa_ld32(&_this->procInfo->blockedCount);
    }
    return pc - bc;
}

u_bool
u_domainIsService(
    _In_ u_domain _this)
{
    return _this->isService;
}

u_result
u_domainAddWaitset(
    const u_domain _this,
    const u_waitset w)
{
    u_result result = U_RESULT_INTERNAL_ERROR;
    os_result osResult;

    assert(_this != NULL);
    assert(w != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        if (!c_iterContains(_this->waitsets, w)) {
            _this->waitsets = c_iterInsert(_this->waitsets, w);
            u_waitsetIncUseCount(w);
        }
        os_mutexUnlock(&_this->mutex);
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_domainRemoveWaitset(
    const u_domain _this,
    const u_waitset w)
{
    u_waitset o;
    u_result result = U_RESULT_INTERNAL_ERROR;
    os_result osResult;

    assert(_this != NULL);
    assert(w != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        o = c_iterTake(_this->waitsets, w);
        os_mutexUnlock(&_this->mutex);
        if (o == NULL) {
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainRemoveWaitset", result,
                      "Precondition not met: "
                      "Given Waitset is not registered for this domain \"%s\" (%u).",
                      _this->name, _this->id);
        } else {
            u_waitsetDecUseCount(w);
            result = U_RESULT_OK;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "user::u_domain::u_domainRemoveWaitset", result,
                  "Operation os_mutexLock_s failed");
    }
    return result;
}

u_bool
u_domainContainsWaitset(
    const u_domain _this,
    const u_waitset w)
{
    u_bool result = FALSE;
    os_result osResult;

    assert(_this != NULL);
    assert(w != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        result = c_iterContains(_this->waitsets, w);
        os_mutexUnlock(&_this->mutex);
    }
    return result;
}

os_uint32
u_domainWaitsetCount(
    const u_domain _this)
{
    os_uint32 result = 0;
    os_result osResult;

    assert(_this != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        result = c_iterLength(_this->waitsets);
        os_mutexUnlock(&_this->mutex);
    }
    return result;
}

u_result
u_domainAddParticipant(
    const u_domain _this,
    const u_participant p)
{
    os_result osResult;
    assert(_this != NULL);
    assert(p != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        _this->participants = c_iterInsert(_this->participants,p);
        u_participantIncUseCount(p);
        os_mutexUnlock(&_this->mutex);
    }
    return U_RESULT_OK;
}

struct count_non_service_participants_helper_arg {
    u_spliced spliced;
    os_uint32 count;
};

static void
count_non_service_participants_helper(
    void *object,
    void *varg)
{
    struct count_non_service_participants_helper_arg *arg = varg;
    if (u_objectKind(object) == U_PARTICIPANT) {
        arg->count++;
    }
    if (u_objectKind(object) == U_SPLICED) {
        arg->spliced = u_spliced(object);
    }
}

u_result
u_domainRemoveParticipant(
    const u_domain _this,
    const u_participant p)
{
    u_participant o = NULL;
    u_result result;
    os_result osResult;

    assert(_this != NULL);
    assert(p != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        o = c_iterTake(_this->participants,p);
        if (o == NULL) {
            os_mutexUnlock(&_this->mutex);
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "user::u_domain::u_domainRemoveParticipant", result,
                        "Precondition not met: "
                        "Given Participant (0x%"PA_PRIxADDR") is not registered for this domain \"%s\" (%u).", (os_address)p, _this->name, _this->id);
        } else {
            struct count_non_service_participants_helper_arg arg;
            int request_termination = 0;

            assert(_this->openCount > 0);

            u_participantDecUseCount(p);

            arg.count = 0;
            arg.spliced = NULL;
            c_iterWalk(_this->participants, count_non_service_participants_helper, &arg);
/* Special case for vxworks kernel, we are "part of spliced" */
/* and a user app removed its participant, don't terminate spliced. */
#ifndef _WRS_KERNEL
            if (arg.count == 0 && arg.spliced != NULL && !_this->closing) {
                /* Setting "closing" prevents new participants from opening the domain */
                _this->closing = 1;
                request_termination = 1;
            }
#endif
            os_mutexUnlock(&_this->mutex);
            if (request_termination) {
                /* FIXME: And what if spliced itselfs decides to stop and wins the race? */
                (void)u_splicedPrepareTermination(arg.spliced);

                /* FIXME: shouldn't be polling, need a timeout */
                /* FIXME: wait for openCount to drop to 1, or wait for participants to become empty? */
                /* Services mustn't wait because spliced waits for the services to terminate */
                if (u_objectKind (p) == U_PARTICIPANT) {
                    os_mutexLock(&_this->mutex);
                    while (c_iterLength(_this->participants) > 0) {
                        const os_duration d = OS_DURATION_INIT( 0, 100000000 );
                        os_mutexUnlock(&_this->mutex);
                        os_sleep(d);
                        os_mutexLock(&_this->mutex);
                    }
                    os_mutexUnlock(&_this->mutex);
                }
            }

            result = U_RESULT_OK;
        }
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "user::u_domain::u_domainRemoveParticipant", result,
                  "Internal error: Acquire lock failed.");
    }

    return result;
}

u_bool
u_domainContainsParticipant(
    const u_domain _this,
    const u_participant participant)
{
    u_bool result = FALSE;
    os_result osResult;

    assert((_this != NULL) && (participant != NULL));

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        result = c_iterContains(_this->participants,participant);
        os_mutexUnlock(&_this->mutex);
    }
    return result;
}

c_ulong
u_domainParticipantCount(
    const u_domain _this)
{
    os_result osResult;
    os_uint32 result = 0;

    assert(_this != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        result = c_iterLength(_this->participants);
        os_mutexUnlock(&_this->mutex);
    }
    return result;
}

struct collect_participants_arg {
    const os_char *name;
    c_iter participants;
};

static void
collect_participants(
    void *object,
    void *arg)
{
    struct collect_participants_arg *a = (struct collect_participants_arg *)arg;
    u_participant p = (u_participant)object;
    os_char *name;

    assert(a != NULL);
    assert(p != NULL);

    if (a->name == NULL) {
        /* name == NULL is treated as wildcard '*' */
        a->participants = c_iterInsert(a->participants, p);
    } else {
        name = u_entityName(u_entity(p));
        if (strcmp(name, a->name) == 0)
        {
            a->participants = c_iterInsert(a->participants, p);
        }
    }
}

c_iter
u_domainLookupParticipants(
    const u_domain _this,
    const os_char *name)
{
    os_result osResult;
    struct collect_participants_arg arg;

    assert(_this != NULL);

    /* name == NULL is treated as wildcard '*' */
    arg.name = name;
    arg.participants = NULL;

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        c_iterWalk(_this->participants, collect_participants, &arg);
        os_mutexUnlock(&_this->mutex);
    }

    return arg.participants;
}

u_result
u_domainWalkParticipants(
    const u_domain _this,
    const u_participantAction action,
          void *actionArg)
{
    os_result osResult;

    assert(_this != NULL);
    assert(action != NULL);

    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        c_iterWalkUntil(_this->participants, (c_iterAction)action, actionArg);
        os_mutexUnlock(&_this->mutex);
    }
    return U_RESULT_OK;
}

u_domain
u_domainKeep (
    const u_domain _this)
{
    pa_inc32(&_this->refCount);
    return _this;
}

u_domainId_t
u_domainId(
    _In_ const u_domain _this)
{
    if (_this == NULL) {
        return -1;
    }
    return _this->id;
}

const char *
u_domainName(
    _In_ const u_domain _this)
{
    if (_this == NULL) {
        return "<NULL>";
    }
    return _this->name;
}

u_bool
u_domainCompareId(
    const u_domain _this,
    const u_domainId_t id)
{
    assert(_this != NULL);
    return (_this->id == id);
}

_Check_return_
u_bool
u_domainSetDetaching(
    _Inout_ u_domain _this,
    _In_ os_uint32 flags)
{
    u_bool result = FALSE;
    os_uint32 state;

    /* When this operation returns true, then u_domainDetach should be
     * called to detach and close the domain.
     * When this operation returns false then u_domainWaitDetaching should
     * be called which waits until the domain is detached and then closes
     * the domain.
     */

    assert(_this != NULL);

    os_mutexLock(&_this->mutex);
    _this->openCount++;
    pa_inc32(&_this->refCount);
    if (pa_ld32(&_this->state) == U_DOMAIN_STATE_ALIVE) {
        state = U_DOMAIN_STATE_DETACHING;
        if(flags & U_USER_BLOCK_OPERATIONS) {
            state |= U_DOMAIN_BLOCK_IN_USER;
        }
        if(flags & U_USER_DELETE_ENTITIES) {
            state |= U_DOMAIN_DELETE_ENTITIES;
        } else if((flags & U_USER_EXCEPTION) && _this->inProcessExceptionHandling) {
            state |= U_DOMAIN_DELETE_ENTITIES;
        } else {
            state |= U_DOMAIN_BLOCK_IN_KERNEL;
        }
        /* Either one of the flags must be set if state is not alive */
        assert(!(state & U_DOMAIN_BLOCK_IN_KERNEL) != !(state & U_DOMAIN_DELETE_ENTITIES));
        pa_st32(&_this->state, state);

        result = TRUE;
    }
    os_mutexUnlock(&_this->mutex);

    return result;
}


void
u_domainWaitDetaching(
    _Inout_ u_domain _this)
{
    os_mutexLock(&_this->mutex);
    while (pa_ld32(&_this->state) & U_DOMAIN_STATE_DETACHING) {
        os_condWait(&_this->cond, &_this->mutex);
    }
    os_mutexUnlock(&_this->mutex);
    (void)u_domainClose(_this);
}

u_bool
u_domainProtectAllowed(
    _In_ u_domain _this)
{
    if (_this == NULL) {
        return FALSE;
    }
    if(os_threadIdToInteger(_this->threadWithAccess) == os_threadIdToInteger(os_threadIdSelf())) {
        return TRUE;
    }

    if(pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_USER) {
        /* If this thread is within a nested protect, then don't deadlock but
         * fail the protect instead. The top-level unprotect will deadlock. */
        if(u__domainThreadProtectCount(_this) == 0) {
            u__domainDeadlock(_this);
        }
    }

    return pa_ld32(&_this->state) == U_DOMAIN_STATE_ALIVE;
}

/* This function is used when the domain has been claimed when performing
 * a u_domainProtect. In case the function blocks the claim count has to
 * be decremented.
 */
u_bool
u_domainProtectAllowedClaimed(
    _In_ u_domain _this)
{
    if(os_threadIdToInteger(_this->threadWithAccess) == os_threadIdToInteger(os_threadIdSelf())) {
        return TRUE;
    }

    if(pa_ld32(&_this->state) & U_DOMAIN_BLOCK_IN_USER) {
        /* If this thread is within a nested protect, then don't deadlock but
         * fail the protect instead. The top-level unprotect will deadlock. */
        if(u__domainThreadProtectCount(_this) == 0) {
            pa_dec32(&_this->claimed);
            u__domainDeadlock(_this);
        }
    }

    return pa_ld32(&_this->state) == U_DOMAIN_STATE_ALIVE;
}

u_result
u_domainDetach (
    _Inout_ u_domain _this)
{
    os_result osResult;
    u_result uresult = U_RESULT_OK;

    assert(_this != NULL);
    osResult = os_mutexLock_s(&_this->mutex);
    if (osResult == os_resultSuccess) {
        _this->threadWithAccess = os_threadIdSelf();
        assert(pa_ld32(&_this->state) != U_DOMAIN_STATE_ALIVE);
        if (pa_ld32(&_this->state) & U_DOMAIN_DELETE_ENTITIES) {
            c_iter list;
            c_iterIter iterator;
            if (_this->waitsets && (c_iterLength(_this->waitsets) > 0)) {
                u_waitset w;
                list = c_iterCopy(_this->waitsets);
                iterator = c_iterIterGet(list);
                while ((w = u_waitset(c_iterNext(&iterator)))) {
                    u_waitsetIncUseCount(w);
                }
                os_mutexUnlock(&_this->mutex);
                while ((w = u_waitset(c_iterTakeFirst(list)))) {
                    u_waitsetDetachFromDomain(w, _this);
                    u_waitsetDecUseCount(w);
                }
                os_mutexLock(&_this->mutex);
                c_iterFree(list);
            }
            if (_this->participants && (c_iterLength(_this->participants) > 0)) {
                u_participant p;
                list = c_iterCopy(_this->participants);
                iterator = c_iterIterGet(list);
                p = u_participant(c_iterNext(&iterator));
                while (p != NULL) {
                    u_participantIncUseCount(p);
                    p = u_participant(c_iterNext(&iterator));
                }
                os_mutexUnlock(&_this->mutex);
                while ((p = u_participant(c_iterTakeFirst(list)))) {
                    /* The following deinit will effectively delete the participant from
                     * the kernel and stop all active user threads but it will not free memory.
                     * Memory will be freed automatically when the process exits.
                     * Memory is not freed so that any user thread accessing this memory
                     * can detect already deleted instead of crashing.
                     */
                    (void)u_objectClose(p);
                    u_participantDecUseCount(p);
                }
                c_iterFree(list);
                os_mutexLock(&_this->mutex);
            }
        }
        _this->threadWithAccess = OS_THREAD_ID_NONE;
        pa_and32(&_this->state, ~U_DOMAIN_STATE_DETACHING);
        os_condBroadcast(&_this->cond);
        os_mutexUnlock(&_this->mutex);
        /* If threads are blocked *within* the kernel, the deletion of the
         * domain must be forced, since the domain-count may never reach 0.
         * Otherwise the last one doing a u_domainClose() (triggered by deletion
         * of kernel-entities) will do the deletion. */
        if(u__domainMustDelete(_this, TRUE)) {
            uresult = u__domainDelete(_this);
        } else {
            assert(FALSE);
        }
    }

    return uresult;
}

u_bool
u_domainCheckHandleServer(
    const u_domain _this,
    const os_uint32 serverId)
{
    assert(_this != NULL);
    return v_kernelCheckHandleServer(_this->kernel,serverId);
}

os_address
u_domainHandleServer(
    _In_ u_domain _this)
{
    assert(_this != NULL);
    return (os_address)_this->kernel->handleServer;
}

os_address
u_domainAddress(
    const u_domain _this)
{
    assert(_this != NULL);
    return (os_address)_this->kernel;
}

void *
u_domainMemoryAddress(
    const u_domain _this)
{
    void *address;
    os_sharedHandle handle;

    assert(_this != NULL);

    if (_this->kernel) {
        handle = u_domainSharedMemoryHandle(_this);
        if(handle)
        {
           address = os_sharedAddress(handle);
        }else {
           address =  NULL;
        }
    } else {
        address =  NULL;
    }
    return address;
}

u_size
u_domainMemorySize(
    const u_domain _this)
{
    os_sharedHandle handle;
    u_size size = 0;

    assert(_this != NULL);

    if (_this->kernel) {
        handle = u_domainSharedMemoryHandle(_this);
        if(handle)
        {
            os_sharedSize(handle, &size);
        }
    }
    return size;
}

u_result
u_domainCreatePersistentSnapshot(
    const u_domain _this,
    const os_char * partition_expression,
    const os_char * topic_expression,
    const os_char * uri)
{
    v_kernel kernel;
    u_result result;
    v_result kResult;

    assert(_this != NULL);
    assert(partition_expression != NULL);
    assert(topic_expression != NULL);
    assert(uri != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public*)(&kernel), C_MM_RESERVATION_LOW);
    if(result == U_RESULT_OK)
    {
        kResult = v_kernelCreatePersistentSnapshot(
            kernel,
            partition_expression,
            topic_expression,
            uri);
        result = u_resultFromKernel(kResult);
        u_observableRelease(u_observable(_this),C_MM_RESERVATION_LOW);
    }

    return result;
}

u_result
u_domainFederationSpecificPartitionName (
    u_domain _this,
    c_char *buf,
    os_size_t bufsize)
{
    u_result result = U_RESULT_OK;
    os_uint32 systemId;
    int n;
    if (bufsize < U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE) {
        return U_RESULT_ILL_PARAM;
    }
/*
    if ((result = u_entityLock (u_entity (_this))) != U_RESULT_OK) {
        return result;
    }
*/
    systemId = _this->_parent._parent.gid.systemId;
    n = snprintf (buf, bufsize, "__NODE%08"PA_PRIx32" BUILT-IN PARTITION__", systemId);
    assert (n < (int) bufsize);
    OS_UNUSED_ARG(n);
/*
    u_entityUnlock (u_entity (_this));
*/
    return result;
}

os_sharedHandle
u_domainSharedMemoryHandle (
    const u_domain _this)
{
    assert(_this != NULL);
    return _this->shm;
}

u_result
u_domain_load_xml_descriptor (
    const u_domain _this,
    const os_char *xml_descriptor)
{
    u_result result;
    v_kernel kernel;
    size_t len;

    assert(_this != NULL);
    assert(xml_descriptor != NULL);
    len = 2 * strlen(xml_descriptor);
    result = u_observableWriteClaim(u_observable(_this), (v_public*)(&kernel), C_MM_RESERVATION_HIGH + len);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_kernel_load_xml_descriptor(kernel, xml_descriptor));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_HIGH + len);
    }

    return result;
}

os_char *
u_domain_get_xml_descriptor (
    const u_domain _this,
    const os_char *type_name)
{
    u_result result;
    v_kernel kernel;
    os_char *description = NULL;

    assert(_this != NULL);
    assert(type_name != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public*)(&kernel),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        description = v_kernel_get_xml_descriptor(kernel, type_name);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }

    return description;
}

c_type
u_domain_lookup_type(
    const u_domain _this,
    const os_char *type_name)
{
    u_result result;
    v_kernel kernel;
    c_type type = NULL;

    assert(_this != NULL);
    assert(type_name != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public*)(&kernel), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        type = v_kernel_lookup_type(kernel, type_name);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }

    return type;
}

/* Note: This function is part of a temporary workaround used by R&R.
 * The function copies heap-configuration (cf_node from parser) to shm-configuration (v_cfNode).
 * This is needed so R&R can use a single operation to process configuration nodes, either from the config file (URI)
 * or a string (R&R config command).
 * Ideally in the future this will be replaced by memory-agnostic XML processing so that it doesn't make a difference
 * if the configuration is on heap or in shared memory.
 */
u_result
u_domainCopyConfiguration(
    cf_node cfgNode,
    u_participant participant,
    u_cfElement *element)
{
    v_cfElement velement = NULL;
    v_configuration config = NULL;
    v_kernel kernel;
    u_domain domain;
    u_result result;


    assert(cfgNode);
    assert(participant);

    if (participant) {
        domain = u_observableDomain(u_observable(participant));
        result = u_observableReadClaim(u_observable(domain),(v_public *)(&kernel), C_MM_RESERVATION_ZERO);
        if ((result == U_RESULT_OK) && (kernel != NULL)) {
            config = v_configurationNew(kernel);
            u_observableRelease(u_observable(domain), C_MM_RESERVATION_ZERO);
        } else {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR, "u_domainCopyConfiguration", result, "Cannot claim u_participant");
        }
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_domainCopyConfiguration", result, "Participant is invalid");
    }

    if (config) {
        result = copyConfiguration(cfgNode, config, (v_cfNode*)&velement);
        if (result == U_RESULT_OK) {
            v_configurationSetRoot(config, velement);
            *element = u_cfElementNew(participant, velement);
        } else {
            *element = NULL;
            v_configurationFree(config);
        }
    }

    return result;
}

u_result
u_domainEnableStatistics(
    const u_domain _this,
    const os_char *categoryName)
{
    v_kernel kernel;
    u_result result;

    assert(_this != NULL);
    assert(categoryName != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public*)(&kernel), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(kernel);
        v_enableStatistics(kernel, categoryName);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_domainEnableStatistics", U_RESULT_INTERNAL_ERROR,
                  "Could not claim service.");
    }

    return result;
}

void
u_domainIdSetThreadSpecific(
    _In_ u_domain domain)
{
    u_result result;

    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        u_domainUnprotect();
    }
}
