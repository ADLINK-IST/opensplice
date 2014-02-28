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
#include "u__types.h"
#include "u__user.h"
#include "u__domain.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u__usrClock.h"
#include "u__usrReportPlugin.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_config.h"
#include "c_iterator.h"
#include "v_handle.h"

#include "cf_config.h"
#include "cfg_parser.h"
#include "ut_entryPoint.h"

#include "u__cfElement.h"

#include "v_configuration.h"
#include "v_cfNode.h"
#include "v_cfElement.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"

#define IGNORE_THREAD_MESSAGE os_threadMemFree(OS_THREAD_WARNING)
#define PRINT_THREAD_MESSAGE(context) printThreadMessage(context)

#define DATABASE_NAME "defaultDomainDatabase"
#define DATABASE_SIZE (0xA00000)
#define KERNEL_NAME   "defaultDomainKernel"
#define DATABASE_SIZE_MIN (0)
#define DATABASE_FREE_MEM_THRESHOLD (0x100000)
#define DATABASE_FREE_MEM_THRESHOLD_MIN (0)

static void
printThreadMessage(
    const char *context)
{
    char *msg = os_threadMemGet(OS_THREAD_WARNING);
    if (msg) {
        OS_REPORT(OS_ERROR,context,0,msg);
        os_threadMemFree(OS_THREAD_WARNING);
    }
}

C_STRUCT(u_domainConfig) {
    c_char       *name;
    c_size        dbSize;
    c_size        dbFreeMemThreshold;
    c_address     address;
    os_lockPolicy lockPolicy;
    c_bool        heap;
    c_bool        builtinTopicEnabled;
    c_bool        prioInherEnabled;
    os_int32      id;
    c_bool        idReadFromConfig;
    c_bool        maintainObjectCount;
};

C_STRUCT(attributeCopyArg) {
    v_configuration configuration;
    v_cfElement element;
};

/**************************************************************
 * Private functions
 **************************************************************/
static c_char *
u_domainName(
    u_domain _this)
{
    c_char *name;

    if (_this == NULL) {
        name = os_strdup("<NULL>");
    } else {
        if (_this->name == NULL) {
            name = os_strdup("<NULL>");
        } else {
            name = os_strdup(_this->name);
        }
    }
    return name;
}

static void
GetDomainConfig(
    cf_element config,
    C_STRUCT(u_domainConfig) *domainConfig,
    os_sharedAttr    *shm_attr)
{
    cf_element dc = NULL;
    cf_element child;
    cf_element id;
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

    assert(config != NULL);
    assert(domainConfig != NULL);

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));
    if (dc != NULL) {
        name = cf_element(cf_elementChild(dc, CFG_NAME));
        if (name != NULL) {
            elementData = cf_data(cf_elementChild(name, "#text"));
            if (elementData != NULL) {
                value = cf_dataValue(elementData);
                os_free(domainConfig->name);
                domainConfig->name = os_malloc(strlen(value.is.String) + 1);
                os_strcpy(domainConfig->name, value.is.String);

                id = cf_element(cf_elementChild(dc, CFG_ID));

                if (id != NULL) {
                    elementData = cf_data(cf_elementChild(id, "#text"));
                    if (elementData != NULL) {
                        value = cf_dataValue(elementData);
                        sscanf(value.is.String, "%d", &domainConfig->id);
                        domainConfig->idReadFromConfig = TRUE;
                    }
                }

                child = cf_element(cf_elementChild(dc, CFG_DATABASE));
                if (child != NULL) {
                    size = cf_element(cf_elementChild(child, CFG_SIZE));
                    if (size != NULL) {
                        elementData = cf_data(cf_elementChild(size, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            u_cfDataSizeValueFromString(value.is.String,&domainConfig->dbSize);
                            if(domainConfig->dbSize <= DATABASE_SIZE_MIN)
                            {
                                domainConfig->dbSize = DATABASE_SIZE_MIN;
                            }
                        }
                    }
                    threshold = cf_element(cf_elementChild(child, CFG_THRESHOLD));
                    if (threshold != NULL) {
                        elementData = cf_data(cf_elementChild(threshold, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            assert(value.kind == V_STRING);
                            u_cfDataSizeValueFromString(value.is.String,&domainConfig->dbFreeMemThreshold);
                            if(domainConfig->dbFreeMemThreshold <= DATABASE_FREE_MEM_THRESHOLD_MIN)
                            {
                                domainConfig->dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD_MIN;
                            }
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
                                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_USER, 0,
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
                                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_USER, 0,
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

                child = cf_element(cf_elementChild(dc, CFG_REPORT));
                if (child != NULL)
                {
                    attr = cf_elementAttribute(child, "verbosity");
                    if (attr != NULL)
                    {
                        value = cf_attributeValue(attr);
                        if (os_reportSetVerbosity(value.is.String) == os_resultFail)
                        {
                            OS_REPORT_2(OS_WARNING, OSRPT_CNTXT_USER, 0,
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
                            OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_USER, 0,
                                    "Cannot parse report append value \"%s\","
                                    " reporting append mode unchanged",value.is.String);
                        }
                        else
                        {
                            os_reportSetDoAppend(doAppend);
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

            child = (cf_node)c_iterTakeFirst(i);
            while (child != NULL) {
                child = (cf_node)c_iterTakeFirst(i);
            }
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
            OS_REPORT_1(OS_WARNING,"user::u_domain::copyConfiguration",0,
                        "Unsuitable configuration node kind (%d)",
                        cfgNode->kind);
            r = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        *node = NULL;
    }
    return r;
}


static u_result
DisableDomain (
    u_domain _this)
{
    u_participant p;
    u_result result;
    int rc;

    result = U_RESULT_OK;
    rc = 0;
    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        p = u_participant(c_iterTakeFirst(_this->participants));
        while (p != NULL) {
            result = u_participantDisable(p);
            if (result != U_RESULT_OK) {
                rc++;
            }
            p = u_participant(c_iterTakeFirst(_this->participants));
        }
        u_entityUnlock(u_entity(_this));
    } else {
        c_char *name = u_domainName(_this);
        OS_REPORT_3(OS_ERROR,
                    "user::u_domain::DisableDomain",0,
                    "Operation u_entityLock failed for Domain (0x%x) = \"%s\""
                    OS_REPORT_NL "Result = \"%s\"",
                    _this, name, u_resultImage(result));
        os_free(name);
    }

    if (rc != 0) {
        c_char *name = u_domainName(_this);
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT_3(OS_ERROR,
                    "user::u_domain::DisableDomain",0,
                    "Disable of (%d) Participant(s) failed for Domain (0x%x) = \"%s\"",
                    rc, _this, name);
        os_free(name);
    }
    return result;
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
    return result;
}

static os_result
startSplicedWithinProcess(
   const c_char *uri)
{
    os_library libraryHandle;
    os_libraryAttr libraryAttr;
    struct ut_entryPointWrapperArg *mwa;
    os_threadAttr threadAttr;
    os_threadId id;
    os_result result;

    char * spliced = "spliced";
    char * entryPoint = "ospl_spliced";

    mwa = os_malloc(sizeof(struct ut_entryPointWrapperArg));

    /* Initialise the library attributes */
    result = os_libraryAttrInit(&libraryAttr);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR,"user::u_domain::startSplicedInProcess",0,
                    "Problem initialising os_libraryAttr\n");
        return os_resultFail;
    }

    /* Now open the library */
    libraryHandle = os_libraryOpen (spliced, &libraryAttr);
    if (libraryHandle == NULL) {
        OS_REPORT_1(OS_ERROR,"user::u_domain::startSplicedInProcess",0,
                    "Error opening '%s' library\n", spliced);
        return os_resultFail;
    }

    /* Locate the entry point.  Note that the usage of os_libraryGetSymbol
     * may result in a warning of the form :
     *   "ISO C forbids assignment between function pointer and void *"
     * Unfortunately it seems this is a quirk of the standards as there is
     * no valid cast between pointer to function.
     */

    mwa->entryPoint = os_libraryGetSymbol (libraryHandle, entryPoint);
    if (mwa->entryPoint == NULL) {
        OS_REPORT_1(OS_ERROR,"user::u_domain::startSplicedInProcess",0,
                    "Error opening '%s' entry point\n", entryPoint);
        return os_resultFail;
    }

    os_threadAttrInit(&threadAttr);

    mwa->argc = 2;
    mwa->argv = os_malloc((mwa->argc + 1)*sizeof(char *));
    mwa->argv[0] = spliced;
    mwa->argv[1] = (char*)uri;
    mwa->argv[2] = NULL;

    /* Invoke the spliced entry point as a new thread in the current process */
    result = os_threadCreate(&id, spliced, &threadAttr, ut_entryPointWrapper, mwa);
    if (result != os_resultSuccess) {
        result = os_resultFail;
        OS_REPORT_1(OS_ERROR,"user::u_domain::startSplicedInProcess",0,
                    "Error starting thread for '%s'\n", spliced);
    }

    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/

u_result
u_domainNew(
    u_domain *domain,
    const c_char *uri)
{
    os_sharedHandle  shm;
    os_sharedAttr    shm_attr;
    os_result        result;
    c_base           base;
    u_result         r;
    cfgprs_status    s;
    v_kernel         kernel;
    cf_element       processConfig;
    v_configuration  configuration;
    v_cfElement      rootElement;
    C_STRUCT(u_domainConfig) domainCfg;
    C_STRUCT(v_kernelQos) kernelQos;
    char            addrStr[100];

    base = NULL;
    processConfig = NULL;
    shm = NULL;

    r = u_userInitialise();
    if (r != U_RESULT_OK) {
        OS_REPORT_2(OS_ERROR,
                    "u_domainNew",0,
                    "implicit u_userInitialise failed, result = %s, uri = %s",
                    u_resultImage(r), uri ? uri : "NULL");
        return r;
    }
    domainCfg.name = os_malloc(strlen(DOMAIN_NAME) + 1);
    os_strcpy(domainCfg.name, DOMAIN_NAME);
    domainCfg.dbSize = DATABASE_SIZE;
    domainCfg.dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD;
    domainCfg.lockPolicy = OS_LOCK_DEFAULT;
    domainCfg.heap = FALSE;
    domainCfg.builtinTopicEnabled = TRUE;
    domainCfg.prioInherEnabled = FALSE;
    domainCfg.id = DEF_DOMAIN_ID;
    domainCfg.address = NULL;
    domainCfg.idReadFromConfig = FALSE;
    domainCfg.maintainObjectCount = 1;

    if (uri == NULL) {
        base = c_create(DATABASE_NAME,NULL,0, 0);
        shm = NULL;
        if (base == NULL) {
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainNew",0,
                      "u_domainNew:Creation of the database failed.");
        }
    } else {
        os_sharedAttrInit(&shm_attr);
        if (strlen(uri) > 0) {
            s = cfg_parse_ospl(uri, &processConfig);
            if (s == CFGPRS_OK) {
                GetDomainConfig(processConfig, &domainCfg, &shm_attr);

                if (r != U_RESULT_OK)
                {
                    OS_REPORT_2(OS_ERROR, "user::u_domain::u_domainNew",0,
                                "Domain '%s' has not been started - ReportPlugin registration failed - %d \n", domainCfg.name, r);
                    return r;
                }
                u_usrClockInit (processConfig);

            } else {
                OS_REPORT_1(OS_ERROR,
                            "user::u_domain::u_domainNew", 0,
                            "Cannot read configuration from URI: \"%s\".",uri);
            }
        } /*  else Get default values */

        if (domainCfg.heap) {
            /* let the abstraction layer know that this is a single process
             * configuration.  It uses this knowledge to use private rather
             * than shared os constructs such as mutexes, conditionals */
            os_serviceSetSingleProcess ();
        } else {
            /* start for windows the named pipe : only required when multiple
             * processes require communication */
            os_serviceStart(domainCfg.name);
        }

        /* Create the shared memory handle here.  The user layer currently requires
         * the support of os_shared... functions like os_sharedSize, so a shared
         * memory handle is required whether we intend to use shared memory or heap.
         * Note that os_sharedmem also has a "heap" configuration
         */
        shm = os_sharedCreateHandle(domainCfg.name, &shm_attr, domainCfg.id);
        if (shm == NULL) {
            OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_USER,0,
                        "u_domainNew:os_sharedCreateHandle failed for domain %s.",
                        domainCfg.name);
        }
        if (!domainCfg.heap ) {
            result = os_sharedMemoryCreate(shm, domainCfg.dbSize);
            if (result != os_resultSuccess) {
                /* Print any message that was generated */
                PRINT_THREAD_MESSAGE("u_domainNew");
                os_sharedDestroyHandle(shm);
                OS_REPORT_1(OS_ERROR,
                            "user::u_domain::u_domainNew",0,
                            "u_domainNew:os_sharedCreateHandle failed for domain %s.",
                            domainCfg.name);

                /* reset r to 0 as it may have been set to success earlier and if we have an error
                   here then the return code should not be OK */
                r = 0;
            } else {
                /* Ignore any message that was generated */
                IGNORE_THREAD_MESSAGE;
                result = os_sharedMemoryAttach(shm);
                PRINT_THREAD_MESSAGE("u_domainNew");
                if (result != os_resultSuccess) {
                    os_sharedDestroyHandle(shm);
                    OS_REPORT_1(OS_ERROR,
                                "user::u_domain::u_domainNew",0,
                                "u_domainNew:os_sharedMemoryCreate failed."
                                OS_REPORT_NL "The service cannot be started."
                                OS_REPORT_NL "The required SHM size was "PA_SIZEFMT" bytes",
                                domainCfg.dbSize);

                    /* reset r to 0 as it may have been set to success earlier and if we have an error
                       here then the return code should not be OK */
                    r = 0;
                } else {
                    if ((domainCfg.lockPolicy != OS_LOCKED) ||
                        (lockSharedMemory(shm) == 0))
                    {
                        base = c_create(DATABASE_NAME,
                                       os_sharedAddress(shm),
                                       domainCfg.dbSize,
                                       domainCfg.dbFreeMemThreshold);
                    } else {
                       os_sharedMemoryDetach(shm);
                       os_sharedDestroyHandle(shm);
                    }
                }
            }
        } else {
            base = c_create(DATABASE_NAME, NULL, 0, 0);
            if (base == NULL) {
                OS_REPORT(OS_ERROR,
                          "user::u_domain::u_domainNew",0,
                          "u_domainNew:Creation of the database failed.");

                /* reset r to 0 as it may have been set to success earlier and if we have an error
                   here then the return code should not be OK */
                r = 0;
            } else {
                OS_REPORT(OS_INFO,
                          "user::u_domain::u_domainNew", 0,
                          "u_domainNew:Database allocated on heap");
            }
        }
    }
    if (base) {
        c_baseSetMaintainObjectCount (base, domainCfg.maintainObjectCount);
        kernelQos.builtin.enabled = domainCfg.builtinTopicEnabled;
        kernel = v_kernelNew(base, KERNEL_NAME, &kernelQos);
        if (kernel == NULL) {
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainNew",0,
                      "u_domainNew:v_kernelNew failed."
                      OS_REPORT_NL "The service cannot be started"
                      OS_REPORT_NL "The created SHM segment will be destroyed");
            result = os_sharedMemoryDetach(shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR,"u_domainNew", 0,
                          "Destroy of the shared memory failed.");
            }
            result = os_sharedMemoryDestroy(shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR,
                          "u_domainNew:os_sharedMemoryDestroy", 0,
                          "Shared memory destroy failed." OS_REPORT_NL
                          "Node may need manual cleanup of the shared memory segment");
            }
            os_sharedDestroyHandle(shm);

            /* reset r to 0 as it may have been set to success earlier and if we have an error
               here then the return code should not be OK */
            r = 0;
        } else {
            /* Copy configuration to kernel */
            rootElement = NULL;
            configuration = v_configurationNew(kernel);
            r = copyConfiguration(cf_node(processConfig),
                                  configuration,
                                  (v_cfNode *)&rootElement);
            if (r != U_RESULT_OK) {
                v_configurationFree(configuration);
                OS_REPORT(OS_ERROR,
                          "user::u_domain::u_domainNew",0,
                          "initialization of configuration admin failed."
                          OS_REPORT_NL "The service cannot be started"
                          OS_REPORT_NL "The created SHM segment will be destroyed\n");
                result = os_sharedMemoryDetach(shm);
                if (result != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,"u_domainNew", 0,
                              "Destroy of the shared memory failed.");
                }
                result = os_sharedMemoryDestroy(shm);
                if (result != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,
                              "u_domainNew:os_sharedMemoryDestroy", 0,
                              "Shared memory destroy failed." OS_REPORT_NL
                              "Node may need manual cleanup of the "
                              "shared memory segment");
                }
                os_sharedDestroyHandle(shm);
            } else {
                v_configurationSetUri(configuration, uri);
                if (rootElement != NULL) {
                    v_configurationSetRoot(configuration, rootElement);
                    v_setConfiguration(kernel, configuration);
                } else {
                    c_char *name;
                    v_configurationFree(configuration);
                    if (domainCfg.name) {
                        name = domainCfg.name;
                    } else {
                        name = DOMAIN_NAME;
                    }
                    OS_REPORT_1(OS_WARNING,
                                "Create Domain Admin (u_domainNew)",0,
                                "No configuration specified for this domain." OS_REPORT_NL
                                "Therefore the default configuration "
                                "will be used.\nDomain      : \"%s\"",
                                name);
                }
                *domain = u_entityAlloc(NULL,u_domain,kernel,TRUE);
                (*domain)->kernel = kernel;
                (*domain)->shm = shm;
                (*domain)->participants = NULL;
                (*domain)->lockPolicy = domainCfg.lockPolicy;
                (*domain)->protectCount = 0;
                if (uri == NULL) {
                    (*domain)->uri = NULL;
                } else {
                    (*domain)->uri = os_malloc(strlen(uri) + 1);
                    os_strcpy((*domain)->uri, uri);
                }
                (*domain)->name = os_malloc(strlen(domainCfg.name) + 1);
                os_strcpy((*domain)->name, domainCfg.name);
                (*domain)->id = domainCfg.id;
                /* r = u_dispatcherInit(u_dispatcher(domain));*/
                u_userAddDomain(*domain);
                if ((PA_ADDRCAST)os_sharedAddress(shm) == (PA_ADDRCAST)shm_attr.map_address) {
                    os_sprintf(addrStr, "0x" PA_ADDRFMT, (PA_ADDRCAST)os_sharedAddress(shm));
                } else {
                    os_sprintf(addrStr, "0x" PA_ADDRFMT " (Requested: 0x" PA_ADDRFMT ")", (PA_ADDRCAST)os_sharedAddress(shm), (PA_ADDRCAST)shm_attr.map_address);
                }
                OS_REPORT_9(OS_INFO,"The OpenSplice domain service", 0,
                            "---------------------------------------------------------------" OS_REPORT_NL
                            "-- The service is using the following configuration settings --" OS_REPORT_NL
                            "---------------------------------------------------------------" OS_REPORT_NL
                            "Domain (id)          : %s (%u)" OS_REPORT_NL
                            "Storage              : %d Kbytes" OS_REPORT_NL
                            "Storage threshold    : %d Kbytes" OS_REPORT_NL
                            "Storage address      : %s" OS_REPORT_NL
                            "Locking              : %s" OS_REPORT_NL
                            "Memory mode          : %s" OS_REPORT_NL
                            "Builtin topics       : %s" OS_REPORT_NL
                            "Priority inheritance : %s",
                            domainCfg.name,
                            domainCfg.id,
                            domainCfg.dbSize/1024,
                            domainCfg.dbFreeMemThreshold/1024,
                            (os_serviceGetSingleProcess())      ? "Not available"   : addrStr,
                            (domainCfg.lockPolicy == OS_LOCKED) ? "true"            : (domainCfg.lockPolicy == OS_UNLOCKED) ? "false" : "default",
                            domainCfg.heap                      ? "Heap memory"     : "Shared memory",
                            domainCfg.builtinTopicEnabled       ? "true"            : "false",
                            domainCfg.prioInherEnabled          ? "true"            : "false");
            }
        }
    }
    if (processConfig != NULL) {
        cf_elementFree(processConfig);
    }
    os_free(domainCfg.name);

    return r;
}

u_result
u_domainOpen(
    u_domain *domain,
    const c_char *uri,
    c_long timeout)
{
    os_sharedHandle  shm = NULL;
    os_sharedAttr    shm_attr;
    os_result        result;
    c_base           base;
    u_result         r;
    cfgprs_status    s;
    v_kernel         kernel;
    cf_element       processConfig;
    C_STRUCT(u_domainConfig) domainCfg;
    os_time          pollDelay = {1,0};
    const c_char     *name = NULL;
    v_configuration  config;
    os_time          delay_100ms = { 0, 100000000 };
    int              count = 0;
    /** @todo - Fix properly. See: OSPL-1222 */
    static os_boolean singleProcessLoggerRegDoneHack = OS_FALSE;

    r = u_userInitialise();
    if (r != U_RESULT_OK) {
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainOpen",0,
                    "implicit u_userInitialise failed, result = %s, uri = %s",
                    u_resultImage(r), uri ? uri : "NULL");
        return r;
    }
    /* If the domain already exists within the process return that */
    *domain = u_userLookupDomain(uri);
    if (*domain != NULL) {
        return U_RESULT_OK;
    }

    base = NULL;
    processConfig = NULL;

    /* Initialize r to zero as in user return codes this means undefined rather than successful*/
    r = 0;

    domainCfg.name = os_malloc(strlen(DOMAIN_NAME) + 1);
    os_strcpy(domainCfg.name, DOMAIN_NAME);
    domainCfg.dbSize = DATABASE_SIZE;
    domainCfg.dbFreeMemThreshold = DATABASE_FREE_MEM_THRESHOLD;
    domainCfg.lockPolicy = OS_LOCK_DEFAULT;
    domainCfg.heap = FALSE;
    domainCfg.builtinTopicEnabled = TRUE;
    domainCfg.prioInherEnabled = FALSE;
    domainCfg.id = DEF_DOMAIN_ID;
    domainCfg.idReadFromConfig = FALSE;

    os_sharedAttrInit(&shm_attr);
    if ((uri != NULL) && (strlen(uri) > 0)) {
        s = cfg_parse_ospl(uri, &processConfig);
        if (s == CFGPRS_OK) {
            GetDomainConfig(processConfig, &domainCfg, &shm_attr);
#ifdef INCLUDE_PLUGGABLE_REPORTING
            /** @todo - Fix properly. See: OSPL-1222 */
            if (!domainCfg.heap || singleProcessLoggerRegDoneHack == OS_FALSE)
            {
                r = u_usrReportPluginReadAndRegister (processConfig);

                if (r != U_RESULT_OK)
                {
                    OS_REPORT_2(OS_ERROR, "user::u_domain::u_domainOpen",0,
                                "Domain '%s' has not been started - ReportPlugin registration failed - return code %d\n", domainCfg.name, r);
                    return r;
                }
                singleProcessLoggerRegDoneHack = OS_TRUE;
            }
#endif
            if (domainCfg.heap) {
                /* This is expected to be a "SingleProcess" deployment.  If control
                 * has reached here then the domain does not yet exist.  This is
                 * either because this is the spliced process which has yet to start
                 * it (will happen during u_domainNew), or this is an application
                 * which expects to start the spliced within the application process
                 */

                if (!u_splicedInProcess()) {
                    result = startSplicedWithinProcess(uri);
                    if (result != os_resultSuccess) {
                        OS_REPORT_1(OS_ERROR,"user::u_domain::u_domainOpen",0,
                                    "Error starting thread for '%s'\n", "spliced");
                    } else {
                        /* wait for up to 5 seconds for the domain to be available in this process */
                        while (count < 50) {
                            os_nanoSleep(delay_100ms);
                            if (*domain == NULL) {
                                *domain = u_userLookupDomain(uri);
                            }
                            if ((*domain != NULL) && (count > 10)) {
                                return U_RESULT_OK;
                            }
                            count++;
                        }
                        OS_REPORT_1(OS_ERROR,"user::u_domain::u_domainOpen",0,
                                    "Domain '%s' has not been started\n", domainCfg.name);
                    }
                }
                /* If we get here then we have been unable to start the spliced from
                 * within the application process
                 */
                return U_RESULT_INTERNAL_ERROR;
            }

            /* set pipename for windows */
            os_createPipeNameFromDomainName(domainCfg.name);

            u_usrClockInit (processConfig);
            if (domainCfg.prioInherEnabled) {
                os_mutexSetPriorityInheritanceMode(OS_TRUE);
            }
            shm = os_sharedCreateHandle(domainCfg.name, &shm_attr,domainCfg.id);
            name = domainCfg.name;
        } else {
            /* assume that the uri is the domain name */
            shm = os_sharedCreateHandle(uri, &shm_attr,domainCfg.id);
            os_createPipeNameFromDomainName(uri);
        }
        if (processConfig != NULL) {
            cf_elementFree(processConfig);
        }
    } else {
        domainCfg.id = 0;
        shm = os_sharedCreateHandle(domainCfg.name, &shm_attr,domainCfg.id);
        name = domainCfg.name;
        /* set pipename for windows */
        os_createPipeNameFromDomainName(domainCfg.name);


    }
    if (shm == NULL) {
        if (timeout >= 0) {
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainOpen",0,
                      "c_open failed; shared memory open failure!");
        }

        /* reset r to 0 as it may have been set to success earlier and if we have an error
           here then the return code should not be OK */
        r = 0;
    } else {
        result = os_sharedMemoryAttach(shm);
        IGNORE_THREAD_MESSAGE;
        while ((timeout > 0) && (result != os_resultSuccess)) {
            os_nanoSleep(pollDelay);
            result = os_sharedMemoryAttach(shm);
            timeout--;
            PRINT_THREAD_MESSAGE("u_domainOpen");
        }
        if (result != os_resultSuccess) {
            os_sharedDestroyHandle(shm);
            if (timeout >= 0) {
                OS_REPORT(OS_ERROR,
                          "user::u_domain::u_domainOpen",0,
                          "os_sharedMemoryAttach failed");
            }
        } else {
            base = c_open(DATABASE_NAME, os_sharedAddress(shm));
            while ((timeout > 0) && (base == NULL)) {
                os_nanoSleep(pollDelay);
                base = c_open(DATABASE_NAME, os_sharedAddress(shm));
                timeout--;
            }
        }
    }
    if (base == NULL) {
        if (timeout >= 0) {
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainOpen",0,
                      "c_open failed");
        }

        /* reset r to 0 as it may have been set to success earlier and if we have an error
           here then the return code should not be OK */
        r = 0;
    } else {
        kernel = v_kernelAttach(base, KERNEL_NAME);
        while ((timeout > 0) && (kernel == NULL)) {
            os_nanoSleep(pollDelay);
            kernel = v_kernelAttach(base, KERNEL_NAME);
            timeout--;
        }
        if (kernel == NULL) {
            OS_REPORT(OS_ERROR,
                      "user::u_domain::u_domainOpen", 0,
                      "v_kernelAttach failed");

            /* reset r to 0 as it may have been set to success earlier and if we have an error
               here then the return code should not be OK */
            r = 0;
        } else {
            *domain = u_entityAlloc(NULL,u_domain,kernel,TRUE);
            (*domain)->kernel = kernel;
            (*domain)->shm = shm;
            (*domain)->protectCount = 0;
            (*domain)->participants = NULL;
            (*domain)->lockPolicy = OS_LOCK_DEFAULT; /* don't care! */
            if ((uri != NULL) && (name == NULL)) {
                /* Assume uri is the domain name,
                 * try to get the real uri from v_configuration
                 */
                name = uri;
                config = v_getConfiguration(kernel);
                uri = v_configurationGetUri(config);
                if (uri != NULL && (strlen(uri) > 0)) {
                    s = cfg_parse_ospl(uri, &processConfig);
                    if (s == CFGPRS_OK) {
                        GetDomainConfig(processConfig,
                                        &domainCfg,
                                        &shm_attr);
                        u_usrClockInit (processConfig);
                        if (domainCfg.prioInherEnabled) {
                            os_mutexSetPriorityInheritanceMode(OS_TRUE);
                        }
#ifdef INCLUDE_PLUGGABLE_REPORTING
            /** @todo - Fix properly. See: OSPL-1222 */
            if (singleProcessLoggerRegDoneHack == OS_FALSE)
            {
                r = u_usrReportPluginReadAndRegister (processConfig);

                if (r != U_RESULT_OK)
                {
                    OS_REPORT_2(OS_ERROR, "user::u_domain::u_domainOpen",0,
                                "Domain '%s' has not been started - ReportPlugin registration failed - return code %d\n", domainCfg.name, r);
                    return r;
                }
            }
#endif
                    }
                }
            }
            (*domain)->id = domainCfg.id;

            /* Recheck 'uri', maybe it is retrieved from v_configuration */
            if (uri != NULL) {
                (*domain)->uri = os_malloc(strlen(uri) + 1);
                os_strcpy((*domain)->uri, uri);
            } else {
                (*domain)->uri = NULL;
            }
            assert(name);
            (*domain)->name = os_malloc(strlen(name) + 1);
            os_strcpy((*domain)->name, name);
            u_userAddDomain(*domain);

            r = U_RESULT_OK;
        }
    }
    os_free(domainCfg.name);

    return r;
}

u_result
u_domainClose (
    u_domain _this)
{
    u_result  r;
    os_result result;
    os_time   pollDelay = {1,0};
    c_long    protectCount;

    if (_this != NULL) {
        /* All participants of this kernel must be disabled! */
        r = DisableDomain(_this);
        if (r == U_RESULT_OK) {
            protectCount = u_domainProtectCount(_this);
            while (protectCount > 0) {
#ifndef NDEBUG
                printf("u_domainClose: waiting for %d threads to detach\n",
                        protectCount);
#endif
                os_nanoSleep(pollDelay);
                protectCount = u_domainProtectCount(_this);
            }
            v_kernelDetach(_this->kernel);
            result = os_sharedMemoryDetach(_this->shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR,"u_domainClose", 0,
                          "Detach from shared memory failed.");
                r = U_RESULT_INTERNAL_ERROR;
            } else {
                os_sharedDestroyHandle(_this->shm);
                r = U_RESULT_OK;
            }
            c_iterFree(_this->participants);
            os_free(_this->uri);
            os_free(_this->name);
            memset(_this,0,sizeof(C_STRUCT(u_domain)));
            os_free(_this);

#ifdef INCLUDE_PLUGGABLE_REPORTING
            u_usrReportPluginUnregister ();
#endif
        }
    } else {
        OS_REPORT(OS_ERROR,"u_domainClose", 0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }

    return r;
}

u_result
u_domainFree (
    u_domain _this)
{
    u_result r;
    os_result result;
    c_long count;
    c_long protectCount;
    c_char *name;
    os_uint32 sleepCount = 0;

    if (_this == NULL) {
        OS_REPORT(OS_ERROR,
                  "user::u_domain::u_domainFree", 0,
                  "The specified Domain = NULL.");
        return U_RESULT_ILL_PARAM;
    }

    if (u_domainParticipantCount(_this) > 0) {
        return U_RESULT_PRECONDITION_NOT_MET;
    }

    r = u_entityLock(u_entity(_this));
    if (r == U_RESULT_OK) {
        static const os_time zero = {0,0}; /* 0 s */
        static const os_time pollPeriod = {0,100 * 1e6}; /* 0.1 s */
        os_time pollDelay = {1,0}; /* 1s */
        protectCount = u_domainProtectCount(_this);
        while (protectCount > 0 && os_timeCompare(pollDelay, zero) == OS_MORE) {
#ifndef NDEBUG
            printf("u_domainFree: waiting for %d threads to detach\n",
                   protectCount);
#endif
            os_nanoSleep(pollPeriod);
            pollDelay = os_timeSub(pollDelay, pollPeriod);
            protectCount = u_domainProtectCount(_this);
            sleepCount++;
            if(sleepCount > 5 && os_serviceGetSingleProcess()) {
                /* do not wait longer then 5 secs in single process mode */
                break;
            }
        }

        if(u_userRemoveDomain(_this) == 0){
            v_kernelDetach(_this->kernel);
            count = v_kernelUserCount(_this->kernel);
            if (count == 0) {
                /* If no users left for the kernel (including the Spliced)
                 * then the kernel and database can be destroyed and the
                 * memory segment can be detached.
                 */
                c_destroy(c_getBase(_this->kernel));
            } else {
                c_detach(c_getBase(_this->kernel));
            }
            if(!os_serviceGetSingleProcess())
            {
                if (_this->shm != NULL) {
                    if (_this->lockPolicy == OS_LOCKED) {
                        result = unlockSharedMemory(_this->shm);
                        if (result != os_resultSuccess) {
                            name = u_domainName(_this);
                            OS_REPORT_2(OS_ERROR,
                                        "user::u_domain::u_domainFree", 0,
                                        "Could not unlock shared segment from memory."
                                        OS_REPORT_NL "Domain = \"%s\""
                                        OS_REPORT_NL "Result = \"%s\"",
                                        name,
                                        os_resultImage(result));
                            os_free(name);

                        }
                    }
                    result = os_sharedMemoryDetach(_this->shm);
                    if (result != os_resultSuccess) {
                        name = u_domainName(_this);
                        OS_REPORT_2(OS_ERROR,
                                    "user::u_domain::u_domainFree", 0,
                                    "Operation os_sharedMemoryDetach failed."
                                    OS_REPORT_NL "Domain = \"%s\""
                                    OS_REPORT_NL "result = \"%s\"",
                                    name,
                                    os_resultImage(result));
                        os_free(name);
                        r = U_RESULT_INTERNAL_ERROR;
                    } else {
                        if (count == 0) {
                            result = os_sharedMemoryDestroy(_this->shm);
                            if (result != os_resultSuccess) {
                                name = u_domainName(_this);
                                OS_REPORT_2(OS_ERROR,
                                            "user::u_domain::u_domainFree", 0,
                                            "Operation os_sharedMemoryDestroy failed."
                                            OS_REPORT_NL "Domain = \"%s\""
                                            OS_REPORT_NL "result = \"%s\"",
                                            name,
                                            os_resultImage(result));
                                os_free(name);
                                r = U_RESULT_INTERNAL_ERROR;
                            }
                        }
                        os_sharedDestroyHandle(_this->shm);
                    }
                }
            } else {
                result = os_resultSuccess;
            }
            assert(c_iterLength(_this->participants) == 0);
            c_iterFree(_this->participants);
            _this->participants = NULL;
            os_free(_this->uri);
            _this->uri = NULL;
            os_free(_this->name);
            _this->name = NULL;

            /* Explicitly perform u_entityDeinit here by hand, since _this
             * doesn't properly extend from u_entity. */
            {
                u_entity _e = u_entity(_this);
                _e->kind = U_UNDEFINED;
                _e->magic = NULL;
                _e->gid = v_publicGid(NULL);
                _e->participant = NULL;
                _e->userData = NULL;
                /* Since this call needs to be done in locked condition, the mutex needs
                * to be unlocked here before destroying. */
                os_mutexUnlock(&_e->mutex);
                os_mutexDestroy(&_e->mutex);
            }

            os_free(_this);

    #ifdef INCLUDE_PLUGGABLE_REPORTING
            u_usrReportPluginUnregister ();
    #endif
        } else {
            u_entityUnlock(u_entity(_this));
        }
    }
    return r;
}

u_result
u_domainProtect(
    u_domain _this)
{
    u_result r;
    c_ulong count;

    if( _this ) {
        count = pa_increment(&_this->protectCount);
        r = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,
                "u_domainProtect",0,
                "Kernel == NULL.");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

u_result
u_domainUnprotect(
    u_domain _this)
{
    u_result r;
    os_uint32 newCount; /* Only used for checking 0-boundary */

    if (_this) {
        newCount = pa_decrement(&_this->protectCount);
        /* Detect passing of 0 boundary
         * (more likely here than with increment)
         */
        assert(newCount + 1 > newCount);
        r = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,
                  "u_domainUnprotect",0,
                  "Domain == NULL.");
        r = U_RESULT_INTERNAL_ERROR;
    }
    return r;
}

c_long
u_domainProtectCount(
    u_domain _this)
{
    c_long count;

    if ( _this ){
        count = _this->protectCount;
    } else {
        count = 0;
    }
    return count;
}


c_voidp
u_domainGetCopy(
    u_domain _this,
    u_entityCopy copy,
    void *copyArg)
{
    void *result;
    u_result r;
    v_kernel vk;

    result = NULL;

    if ((_this != NULL) && (copy != NULL)) {
        r = u_entityReadClaim(u_entity(_this),(v_entity*)(&vk));
        if (r == U_RESULT_OK) {
            assert(vk);
            result = copy((v_entity)vk, copyArg);
            r = u_entityRelease(u_entity(_this));
        } else {
            c_char *name = u_domainName(_this);
            OS_REPORT_4(OS_ERROR,
                        "user::u_domain::u_domainGetCopy",0,
                        "Operation u_entityReadClaim(domain=0x%x,entity=0x%x) failed."
                        OS_REPORT_NL "Domain name = \"%s\""
                        OS_REPORT_NL "Result = %s",
                        _this, &vk, name, u_resultImage(r));
            os_free(name);
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainGetCopy", 0,
                    "Illegal parameter. Domain=0x%x, copy=0x%x",
                    _this, copy);
        r = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_domainAddParticipant(
    u_domain _this,
    u_participant p)
{
    c_long oldCount,newCount;
    u_result result;

    if ((_this != NULL) && (p != NULL)) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            oldCount = c_iterLength(_this->participants);
            _this->participants = c_iterInsert(_this->participants,p);
            newCount = c_iterLength(_this->participants);
            if (newCount == (oldCount + 1)) {
                result = U_RESULT_OK;
            } else {
                c_char *name = u_entityName(u_entity(p));
                OS_REPORT_4(OS_ERROR,
                            "user::u_domain::u_domainAddParticipant", 0,
                            "The participant count is not increased by one"
                            OS_REPORT_NL "new count = %d and old count = %d"
                            OS_REPORT_NL "Participant name = \"%s\""
                            OS_REPORT_NL "Domain name = \"%s\"",
                            newCount, oldCount, name, _this->name);
                os_free(name);
                result = U_RESULT_INTERNAL_ERROR;
            }
            u_entityUnlock(u_entity(_this));
        }
    } else {
        c_char *dname;
        c_char *name;

        if (_this) {
            dname = _this->name;
        } else {
            dname = "<NULL>";
        }
        name = u_entityName(u_entity(p));
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainAddParticipant", 0,
                    "Operation failed: Illegal parameter."
                    OS_REPORT_NL "Participant name = \"%s\""
                    OS_REPORT_NL "Domain name = \"%s\"",
                    name, dname);
        os_free(name);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_domainRemoveParticipant(
    u_domain _this,
    u_participant p)
{
    u_participant o;
    u_result result;

    if (_this) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            o = c_iterTake(_this->participants,p);
            if (o == NULL) {
                OS_REPORT(OS_ERROR,
                          "user::u_domain::u_domainRemoveParticipant", 0,
                          "Precondition not met: "
                          "Given Participant is not registered for this domain.");
                result = U_RESULT_PRECONDITION_NOT_MET;
            } else {
                result = U_RESULT_OK;
            }
            u_entityUnlock(u_entity(_this));
        }
    } else {
        c_char *name = u_entityName(u_entity(p));
        OS_REPORT_1(OS_ERROR,
                    "user::u_domain::u_domainRemoveParticipant", 0,
                    "Operation failed: Illegal parameter:"
                    OS_REPORT_NL "Participant name = \"%s\""
                    OS_REPORT_NL "Domain = NULL.",
                    name);
        os_free(name);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_domainContainsParticipant(
    u_domain _this,
    u_participant participant)
{
    c_bool found = FALSE;
    u_result result;
    c_char *name;
    c_char *dname;

    if ((_this != NULL) && (participant != NULL)) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterContains(_this->participants,participant);
            u_entityUnlock(u_entity(_this));
        } else {
            name = u_entityName(u_entity(participant));
            dname = u_domainName(_this);
            OS_REPORT_5(OS_ERROR,
                        "user::u_domain::u_domainContainsParticipant",0,
                        "Operation failed to lock Domain."
                        OS_REPORT_NL "Participant (0x%x) name = \"%s\"."
                        OS_REPORT_NL "Domain (0x%x) name = \"%s\""
                        OS_REPORT_NL "Result = %s",
                        participant, name, _this, dname, u_resultImage(result));
            os_free(name);
            os_free(dname);
        }
    } else {
        dname = u_domainName(_this);
        name = u_entityName(u_entity(participant));
        OS_REPORT_4(OS_ERROR,
                    "user::u_domain::u_domainContainsParticipant",0,
                    "Operation failed: Invalid parameter."
                    OS_REPORT_NL "Participant (0x%x) name = \"%s\"."
                    OS_REPORT_NL "Domain (0x%x) name = \"%s\"",
                    participant, name, _this, dname);
        os_free(name);
        os_free(dname);
    }
    return found;
}

c_long
u_domainParticipantCount(
    u_domain _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->participants);
        u_entityUnlock(u_entity(_this));
    } else {
        c_char *name = u_domainName(_this);
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainParticipantCount",0,
                    "Failed to lock Domain."
                    OS_REPORT_NL "Domain = \"%s\""
                    OS_REPORT_NL "Result = %s.",
                    name,
                    u_resultImage(result));
        os_free(name);
    }
    return length;
}

struct collect_participants_arg {
    const c_char *name;
    c_iter participants;
};

static void
collect_participants(
    c_voidp object,
    c_voidp arg)
{
    struct collect_participants_arg *a = (struct collect_participants_arg *)arg;
    u_participant p = (u_participant)object;
    c_char *name;

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
    u_domain _this,
    const c_char *name)
{
    struct collect_participants_arg arg;
    u_result result;

    /* name == NULL is treated as wildcard '*' */
    arg.name = name;
    arg.participants = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->participants, collect_participants, &arg);
        u_entityUnlock(u_entity(_this));
    } else {
        c_char *name = u_domainName(_this);
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainLookupParticipants",0,
                    "Failed to lock Domain."
                    OS_REPORT_NL "Domain = \"%s\""
                    OS_REPORT_NL "Result = %s.",
                    name,
                    u_resultImage(result));
        os_free(name);
    }
    return arg.participants;
}

u_result
u_domainWalkParticipants(
    u_domain _this,
    u_participantAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->participants,
                        (c_iterAction)action,
                        actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        c_char *name = u_domainName(_this);
        OS_REPORT_2(OS_ERROR,
                    "user::u_domain::u_domainWalkParticipants",0,
                    "Failed to lock Domain."
                    OS_REPORT_NL "Domain = \"%s\""
                    OS_REPORT_NL "Result = %s.",
                    name,
                    u_resultImage(result));
        os_free(name);
    }
    return result;
}

u_participant
u_domainCreateParticipant (
    u_domain _this,
    const c_char *name,
    v_qos qos,
    c_bool enable)
{
    /* can be implemented as soon as u_participantNew accepts domain instead of uri */
    return NULL;
}

c_bool
u_domainCompareDomainId(
    u_domain _this,
    const c_char* id)
{
    const c_char *domainId;
    const c_char *uri;
    const c_char *name;
    u_result result;
    os_char* uriQuoted;
    os_size_t uriLength;

    if (_this != NULL) {
        domainId = id;
        uri = _this->uri;
        name = _this->name;
        if ((domainId == NULL) && ((uri == NULL) || name == NULL)) {
            result = TRUE;
        } else if (domainId != NULL)  {
            if (name == NULL) {
                name = "";
            }
            result = (strcmp(domainId, name) == 0);
            if (!result) {
                if (uri == NULL) {
                    uri = "";
                }
                result = (strcmp(domainId, uri) == 0);

                if (!result)
                {
                    /* At this point, the domains (either name or uri) do not match exactly.
                     * The uri [possibly obtained from 'os_getenv ("OSPL_URI")'] may have
                     * leading and trailing quotes because the user may have quoted it to
                     * handle possible spaces in the uri path.  Internally the uri will be
                     * stored without quoutes, so check this is not the difference between
                     * the two URIs:
                     */
                    uriLength = strlen (uri);
                    if (uriLength + 2 == strlen (domainId))
                    {
                        uriQuoted = os_malloc (sizeof (char*) * (uriLength + 2));
                        sprintf (uriQuoted, "\"%s\"", uri);
                        result = (strcmp (uriQuoted, domainId) == 0);
                        os_free (uriQuoted);
                    }
                }
            }
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}


u_result
u_domainDetachParticipants(
    u_domain _this)
{
    u_result result;
    u_participant p;

    if (_this != NULL) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            p = u_participant(c_iterTakeFirst(_this->participants));
            while (p != NULL) {
                (void)u_participantDetach(p);
                p = u_participant(c_iterTakeFirst(_this->participants));
            }
            u_entityUnlock(u_entity(_this));
            result = U_RESULT_OK;
        } else {
            c_char *name = u_domainName(_this);
            OS_REPORT_3(OS_ERROR,
                        "user::u_damain::u_domainDetachParticipants", 0,
                        "Operation u_entityLock(0x%x) failed."
                        OS_REPORT_NL "Domain = \"%s\"."
                        OS_REPORT_NL "result = %s.",
                        _this, name, u_resultImage(result));
            os_free(name);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "user::u_domain::u_domainDetachParticipants", 0,
                  "Illegal parameter: domain=NULL.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_domainCheckHandleServer(
    u_domain _this,
    c_long serverId)
{
    c_bool result = FALSE;

    if (_this != NULL ) {
        result = v_kernelCheckHandleServer(_this->kernel,serverId);
    } else {
        OS_REPORT(OS_ERROR,
                  "user::u_domain::u_domainCheckHandleServer", 0,
                  "Illegal parameter. domain=NULL");
    }
    return result;
}

c_address
u_domainHandleServer(
    u_domain _this)
{
    return (c_address)_this->kernel->handleServer;
}

c_voidp
u_domainAddress(
    u_domain _this)
{
    c_voidp address;

    if (_this) {
        address =  _this->kernel;
    } else {
        address =  NULL;
    }
    return address;
}

c_voidp
u_domainMemoryAddress(
    u_domain _this)
{
    c_voidp address;
    os_sharedHandle handle;

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

c_size
u_domainMemorySize(
    u_domain _this)
{
    os_sharedHandle handle;
    c_size size = 0;

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
    u_domain _this,
    const c_char * partition_expression,
    const c_char * topic_expression,
    const c_char * uri)
{
    v_kernel kernel;
    u_result result;
    v_result kResult;

    if(!_this || !partition_expression || !topic_expression || !uri)
    {
        result = U_RESULT_ILL_PARAM;
    } else
    {
        result = U_RESULT_OK;
    }

    if(result == U_RESULT_OK)
    {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kernel));
        if(result == U_RESULT_OK)
        {
            kResult = v_kernelCreatePersistentSnapshot(
                kernel,
                partition_expression,
                topic_expression,
                uri);
            result = u_resultFromKernel(kResult);
            u_entityRelease(u_entity(_this));
        }
    }
    return result;
}

u_result
u_domainFederationSpecificPartitionName (
    u_domain _this,
    c_char *buf,
    os_size_t bufsize)
{
    u_result result;
    os_uint32 systemId;
    int n;
    if (bufsize < U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE) {
        return U_RESULT_ILL_PARAM;
    }
    if ((result = u_entityLock (u_entity (_this))) != U_RESULT_OK) {
        return result;
    }

    systemId = _this->_parent._parent.gid.systemId;
    n = snprintf (buf, bufsize, "__NODE%08"PA_PRIx32" BUILT-IN PARTITION__", systemId);
    assert (n < bufsize);
    u_entityUnlock (u_entity (_this));
    return U_RESULT_OK;
}

os_sharedHandle
u_domainSharedMemoryHandle (
    u_domain _this)
{
    if (_this) {
        return _this->shm;
    } else {
        return NULL;
    }
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
    u_result result;


    assert(cfgNode);
    assert(participant);

    if (participant) {
        result = u_entityReadClaim(u_entity(participant->domain),(v_entity*)(&kernel));
        if ((result == U_RESULT_OK) && (kernel != NULL)) {
            config = v_configurationNew(kernel);
            u_entityRelease(u_entity(participant->domain));
        } else {
            OS_REPORT(OS_ERROR, "u_domainCopyConfiguration", 0, "Cannot claim u_participant");
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT(OS_ERROR, "u_domainCopyConfiguration", 0, "Participant is invalid");
        result = U_RESULT_ILL_PARAM;
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
