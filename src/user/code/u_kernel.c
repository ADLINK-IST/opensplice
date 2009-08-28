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
#include "u__user.h"
#include "u__kernel.h"
#include "u__participant.h"
#include "u__usrClock.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "os.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "c_iterator.h"
#include "v_handle.h"

#include "cf_config.h"
#include "cfg_parser.h"

#include "v_configuration.h"
#include "v_cfNode.h"
#include "v_cfElement.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"

#define DOMAIN_NAME   "The default Domain"
#define DATABASE_NAME "defaultDomainDatabase"
#define KERNEL_NAME   "defaultDomainKernel"
#define DATABASE_SIZE (0xA00000)

#define IGNORE_THREAD_MESSAGE os_threadMemFree(OS_WARNING)
#define PRINT_THREAD_MESSAGE(context) printThreadMessage(context)

static void
printThreadMessage(
    const char *context)
{
    char *msg = os_threadMemGet(OS_WARNING);
    if (msg) {
        OS_REPORT(OS_ERROR,context,0,msg);
        os_threadMemFree(OS_WARNING);
    }
}


C_STRUCT(u_kernel) {
    os_mutex        mutex;
    v_kernel        kernel;
    os_sharedHandle shm;
    c_iter          participants;
    c_char          *uri;
    os_lockPolicy   lockPolicy;
};

C_STRUCT(domain) {
    c_char       *name;
    c_ulong       dbSize;
    c_address     address;
    os_lockPolicy lockPolicy;
    c_bool        heap;
    c_bool        builtinTopicEnabled;
    c_bool        prioInherEnabled;
};

C_STRUCT(attributeCopyArg) {
    v_configuration configuration;
    v_cfElement element;
};

/**************************************************************
 * Private functions
 **************************************************************/
static void
u_kernelGetDomainConfig(
    cf_element config,
    C_STRUCT(domain) *domainConfig,
    os_sharedAttr    *shm_attr)
{
    cf_element dc = NULL;
    cf_element child;
    cf_element name;
    cf_data elementData;
    cf_element size;
    cf_element address;
    cf_element heap;
    cf_element locked;
    c_value value;
    cf_attribute attr;

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
                strcpy(domainConfig->name, value.is.String);
                child = cf_element(cf_elementChild(dc, CFG_DATABASE));
                if (child != NULL) {
                    size = cf_element(cf_elementChild(child, CFG_SIZE));
                    if (size != NULL) {
                        elementData = cf_data(cf_elementChild(size, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            sscanf(value.is.String, "%uld", &domainConfig->dbSize);
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
                    heap = cf_element(cf_elementChild(child, CFG_HEAP));
                    if (heap != NULL) {
                        elementData = cf_data(cf_elementChild(heap, "#text"));
                        if (elementData != NULL) {
                            value = cf_dataValue(elementData);
                            if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
                                domainConfig->heap = TRUE;
                            } else if (os_strncasecmp(value.is.String, "FALSE", 5) == 0) {
                                domainConfig->heap = FALSE;
                            }
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
            OS_REPORT_1(OS_WARNING,"copyConfiguration",0,
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
kernelDisable (
    u_kernel k)
{
    u_participant p;
    u_result r;
    int rc;

    r = U_RESULT_OK;
    rc = 0;
    os_mutexLock(&k->mutex);
    p = u_participant(c_iterTakeFirst(k->participants));
    while (p != NULL) {
        r = u_participantDisable(p);
        if (r != U_RESULT_OK) {
            rc++;
        }
        p = u_participant(c_iterTakeFirst(k->participants));
    }
    os_mutexUnlock(&k->mutex);

    if (rc != 0) {
        r = U_RESULT_INTERNAL_ERROR;
    }
    return r;
}

#ifndef INTEGRITY
static int
lockSharedMemory(
    os_sharedHandle shm,
    os_lockPolicy lockPolicy)
{
    int result;
    void *address;
    os_result r;
    os_uint size;

    if (lockPolicy == OS_LOCKED) {
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
	     we are not attached to shared memory at all */
            assert(0);
            result = 1; /* fail */
        }
    } else {
        result = 0; /* success */
    }
    return result;
}

static int
unlockSharedMemory(
    u_kernel k)
{
    int result;
    os_result r;
    os_uint size;
    void *address;

    if (k->lockPolicy == OS_LOCKED) {
        address = os_sharedAddress(k->shm);
        if (address) {
            r = os_sharedSize(k->shm, &size);
            if (r == os_resultSuccess) {
                r = os_procMUnlock(address, size);
            }
            if (r == os_resultSuccess) {
                result = 0; /* success */
            } else {
                result = 1; /*fail*/
            }
        } else {
           /* this should not happen, as this would mean
              we are not attach to shared memory at all */
            assert(0);
            result = 1; /* fail */
        }
    } else {
        result = 0; /* success */
    }
    return result;
}
#endif

/**************************************************************
 * Public functions
 **************************************************************/
u_result
u_kernelClaim (
    u_kernel _this,
    v_kernel *kernel)
{
    u_result result;

    if ((_this != NULL) && (kernel != NULL)) {
        result = u_userProtect(NULL);
        if (result == U_RESULT_OK) {
            *kernel = v_kernel(_this->kernel);
        } else {
            OS_REPORT(OS_ERROR,"u_kernelClaim",0,
                      "Protect process failed");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_kernelClaim",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_kernelRelease (
    u_kernel _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_userUnprotect(NULL);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR,"u_kernelRelease",0,
                      "Protect process failed");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_kernelRelease",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_kernel
u_kernelNew(
    const c_char *uri)
{
    os_sharedHandle  shm;
    os_sharedAttr    shm_attr;
    os_result        result;
    c_base           base;
    u_kernel         uKernel;
    u_result         r;
    cfgprs_status    s;
    v_kernel         kernel;
    cf_element       processConfig;
    v_configuration  configuration;
    v_cfElement      rootElement;
    C_STRUCT(domain) domainCfg;
    os_mutexAttr     mutexAttr;
    C_STRUCT(v_kernelQos) kernelQos;

    uKernel = NULL;
    base = NULL;
    processConfig = NULL;
    shm = NULL;

    r = u_userProtect(NULL);

    domainCfg.name = os_malloc(strlen(DOMAIN_NAME) + 1);
    strcpy(domainCfg.name, DOMAIN_NAME);
    domainCfg.dbSize = DATABASE_SIZE;
    domainCfg.lockPolicy = OS_LOCK_DEFAULT;
    domainCfg.heap = FALSE;
    domainCfg.builtinTopicEnabled = TRUE;
    domainCfg.prioInherEnabled = FALSE;

    if (uri == NULL) {
        base = c_create(DATABASE_NAME,NULL,0);
        shm = NULL;
        if (base == NULL) {
            OS_REPORT(OS_WARNING,OSRPT_CNTXT_USER,0,
                      "u_kernelNew:Creation of the database failed.");
        }
    } else {
        os_sharedAttrInit(&shm_attr);
        if ((uri != NULL) && (strlen(uri) > 0)) {
            s = cfg_parse_ospl(uri, &processConfig);
            if (s == CFGPRS_OK) {
                u_kernelGetDomainConfig(processConfig, &domainCfg, &shm_attr);
            } else {
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_USER, 0,
                            "Cannot read configuration from URI: \"%s\".",uri);
            }
        } /*  else Get default values */
        if (!domainCfg.heap) {
            shm = os_sharedCreateHandle(domainCfg.name, &shm_attr);
            if (shm == NULL) {
                OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_USER,0,
                            "u_kernelNew:os_sharedCreateHandle failed for domain %s.",
                            domainCfg.name);
            } else {
                result = os_sharedMemoryCreate(shm, domainCfg.dbSize);
                if (result != os_resultSuccess) {
                    /* Print any message that was generated */
                    PRINT_THREAD_MESSAGE("u_kernelNew");
                    os_sharedDestroyHandle(shm);
                    OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_USER,0,
                                "u_kernelNew:os_sharedMemoryCreate failed.\n"
                                "              The service cannot be started.\n"
                                "              The required SHM size was %d bytes",
                                domainCfg.dbSize);
                } else {
                    /* Ignore any message that was generated */
                    IGNORE_THREAD_MESSAGE;
                    result = os_sharedMemoryAttach(shm);
                    PRINT_THREAD_MESSAGE("u_kernelNew");
                    if (result != os_resultSuccess) {
                        os_sharedDestroyHandle(shm);
                        OS_REPORT(OS_ERROR,OSRPT_CNTXT_USER,0,
                                  "u_kernelNew:os_sharedMemoryAttach failed."
                                  "              The service cannot be started\n"
                                  "              The created SHM segment will be destroyed");
                    } else {
#ifndef INTEGRITY
                        if (lockSharedMemory(shm, domainCfg.lockPolicy) == 0) {
#endif
                            base = c_create(DATABASE_NAME,
                                        os_sharedAddress(shm),
                                        domainCfg.dbSize);
#ifndef INTEGRITY

                        } else {
                            os_sharedMemoryDetach(shm);
                            os_sharedDestroyHandle(shm);
                        }
#endif
                    }
                }
            }
        } else {
            base = c_create(DATABASE_NAME, NULL, 0);
            if (base == NULL) {
                OS_REPORT(OS_WARNING,OSRPT_CNTXT_USER,0,
                          "u_kernelNew:Creation of the database failed.");
            } else {
                OS_REPORT(OS_INFO, OSRPT_CNTXT_USER, 0,
                          "u_kernelNew:Database allocated on heap");
            }
        }
    }
    if (base) {
        kernelQos.builtin.enabled = domainCfg.builtinTopicEnabled;
        kernel = v_kernelNew(base, KERNEL_NAME, &kernelQos);
        if (kernel == NULL) {
            OS_REPORT(OS_WARNING,OSRPT_CNTXT_USER,0,
                      "u_kernelNew:v_kernelNew failed.\n"
                      "              The service cannot be started\n"
                      "              The created SHM segment will be destroyed\n");
            result = os_sharedMemoryDetach(shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_WARNING,"u_kernelNew", 0,
                          "Destroy of the shared memory failed.");
            }
            result = os_sharedMemoryDestroy(shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_WARNING,
                          "u_kernelNew:os_sharedMemoryDestroy", 0,
                          "Shared memory destroy failed.\n"
                          "Node may need manual cleanup of the shared memory segment");
            }
            os_sharedDestroyHandle(shm);
        } else {
            /* Copy configuration to kernel */
            rootElement = NULL;
            configuration = v_configurationNew(kernel);
            r = copyConfiguration(cf_node(processConfig),
                                  configuration, (v_cfNode *)&rootElement);
            if (r != U_RESULT_OK) {
                v_configurationFree(configuration);
                OS_REPORT(OS_WARNING,OSRPT_CNTXT_USER,0,
                          "u_kernelNew:initialization of configuration admin failed.\n"
                          "              The service cannot be started\n"
                          "              The created SHM segment will be destroyed\n");
                result = os_sharedMemoryDetach(shm);
                if (result != os_resultSuccess) {
                    OS_REPORT(OS_WARNING,"u_kernelNew", 0,
                              "Destroy of the shared memory failed.");
                }
                result = os_sharedMemoryDestroy(shm);
                if (result != os_resultSuccess) {
                    OS_REPORT(OS_WARNING,
                              "u_kernelNew:os_sharedMemoryDestroy", 0,
                              "Shared memory destroy failed.\n"
                              "              Node may need manual cleanup of the "
                              "shared memory segment");
                }
                os_sharedDestroyHandle(shm);
            } else {
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
                    OS_REPORT_1(OS_WARNING,"Create Domain Admin (u_kernelNew)",0,
                                "No configuration specified for this domain.\n"
                                "              Therefore the default configuration "
                                "will be used.\nDomain      : \"%s\"",
                                name);
                }
                uKernel = os_malloc(sizeof(C_STRUCT(u_kernel)));
                os_mutexAttrInit(&mutexAttr);
                mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
                os_mutexInit(&uKernel->mutex,&mutexAttr);
                uKernel->kernel = kernel;
                uKernel->shm = shm;
                uKernel->participants = NULL;
                if (uri == NULL) {
                    uKernel->uri = NULL;
                } else {
                    uKernel->uri = os_malloc(strlen(uri) + 1);
                    strcpy(uKernel->uri, uri);
                }
                OS_REPORT_3(OS_INFO,"The OpenSplice domain service", 0,
                            "+++++++++++++++++++++++++++++++++++++++++++\n"
                            "              ++ The service has successfully started. ++\n"
                            "              +++++++++++++++++++++++++++++++++++++++++++\n"
                            "Storage     : %d Kbytes.\n"
                            "Locking     : %s\n"
                            "Domain      : \"%s\"",
                             domainCfg.dbSize/1024,
                             (domainCfg.lockPolicy == OS_LOCKED)?"true":
                             (domainCfg.lockPolicy == OS_UNLOCKED)? "false":"default",
                             domainCfg.name);

            }
        }
    }
    if (processConfig != NULL) {
        cf_elementFree(processConfig);
    }
    r = u_userUnprotect(NULL);

    os_free(domainCfg.name);

    return uKernel;
}

u_kernel
u_kernelOpen(
    const c_char *uri,
    c_long timeout)
{
    os_sharedHandle  shm = NULL;
    os_sharedAttr    shm_attr;
    os_result        result;
    c_base           base;
    u_kernel         o;
    cfgprs_status    s;
    v_kernel         kernel;
    cf_element       processConfig;
    C_STRUCT(domain) domainCfg;
    os_mutexAttr     mutexAttr;
    os_time          pollDelay = {1,0};

    o = NULL;
    base = NULL;
    processConfig = NULL;

    domainCfg.name = os_malloc(strlen(DOMAIN_NAME) + 1);
    strcpy(domainCfg.name, DOMAIN_NAME);
    domainCfg.dbSize = DATABASE_SIZE;
    domainCfg.lockPolicy = OS_LOCK_DEFAULT;
    domainCfg.heap = FALSE;
    domainCfg.builtinTopicEnabled = TRUE;
    domainCfg.prioInherEnabled = FALSE;

    os_sharedAttrInit(&shm_attr);
    if ((uri != NULL) && (strlen(uri) > 0)) {
        s = cfg_parse_ospl(uri, &processConfig);
        if (s == CFGPRS_OK) {
            u_kernelGetDomainConfig(processConfig, &domainCfg, &shm_attr);
    	    u_usrClockInit (processConfig);
            if (domainCfg.prioInherEnabled) {
                os_mutexSetPriorityInheritanceMode(OS_TRUE);
            }
        } else {
            if (timeout >= 0) {
                OS_REPORT_1(OS_ERROR, "u_kernelOpen", 0,
                            "Cannot read configuration from URI: \"%s\".",uri);
            }
        }
        if (processConfig != NULL) {
            cf_elementFree(processConfig);
        }
    }
    shm = os_sharedCreateHandle(domainCfg.name, &shm_attr);
    if (shm == NULL) {
        if (timeout >= 0) {
            OS_REPORT(OS_ERROR,"u_kernelOpen",0,
                      "c_open failed; shared memory open failure!");
        }
    } else {
        result = os_sharedMemoryAttach(shm);
        IGNORE_THREAD_MESSAGE;
        while ((timeout > 0) && (result != os_resultSuccess)) {
            os_nanoSleep(pollDelay);
            result = os_sharedMemoryAttach(shm);
            timeout--;
            PRINT_THREAD_MESSAGE("u_kernelOpen");
        }
        if (result != os_resultSuccess) {
            os_sharedDestroyHandle(shm);
            if (timeout >= 0) {
                OS_REPORT(OS_ERROR,"u_kernelOpen",0,
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
            OS_REPORT(OS_ERROR,"u_kernelOpen",0,
                      "c_open failed");
        }
    } else {
        kernel = v_kernelAttach(base, KERNEL_NAME);
        while ((timeout > 0) && (kernel == NULL)) {
            os_nanoSleep(pollDelay);
            kernel = v_kernelAttach(base, KERNEL_NAME);
            timeout--;
        }
        if (kernel == NULL) {
            OS_REPORT(OS_ERROR,"u_kernelOpen", 0,
                      "v_kernelAttach failed");
        } else {
            o = os_malloc(sizeof(C_STRUCT(u_kernel)));
            os_mutexAttrInit(&mutexAttr);
            mutexAttr.scopeAttr=OS_SCOPE_PRIVATE;
            os_mutexInit(&o->mutex,&mutexAttr);
            o->kernel = kernel;
            o->shm = shm;
            if (uri == NULL) {
                o->uri = NULL;
            } else {
                o->uri = os_malloc(strlen(uri) + 1);
                strcpy(o->uri, uri);
            }
            o->participants = NULL;
            o->lockPolicy = OS_LOCK_DEFAULT; /* don't care! */
        }
    }
    os_free(domainCfg.name);
    return o;
}

u_result
u_kernelClose (
    u_kernel k)
{
    u_result  r;
    os_result result;
    os_time   pollDelay = {1,0};
    c_long    protectCount;

    if (k != NULL) {
        /* All participants of this kernel must be disabled! */
        r = kernelDisable(k);
        if (r == U_RESULT_OK) {
            protectCount = u_userProtectCount();
            while (protectCount > 0) {
                os_nanoSleep(pollDelay);
                protectCount = u_userProtectCount();
            }
            v_kernelDetach(k->kernel);
            result = os_sharedMemoryDetach(k->shm);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR,"u_kernelClose", 0,
                          "Destroy shared memory failed.");
                r = U_RESULT_INTERNAL_ERROR;
            } else {
                os_sharedDestroyHandle(k->shm);
                r = U_RESULT_OK;
            }
            c_iterFree(k->participants);
            os_free(k->uri);
            memset(k,0,sizeof(C_STRUCT(u_kernel)));
            os_free(k);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_kernelClose", 0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_kernelFree (
    u_kernel k)
{
    u_result r;
    os_result result;
    c_long count,laps;
    os_time pollDelay = {1,0};

    if (k != NULL) {
        r = u_userProtect(NULL);
        if (r == U_RESULT_OK) {
            laps = 4;
            count = v_kernelUserCount(k->kernel);

            while ((laps > 0) && (count > 1)) {
#ifndef NDEBUG
                printf("u_kernelFree: waiting for %d users to detach\n",count);
#endif
                os_nanoSleep(pollDelay);
                laps--;
                count = v_kernelUserCount(k->kernel);
            }
            if (count > 1) {
                OS_REPORT(OS_WARNING,
                          "u_kernelFree", 0,
                          "Some blocking applications are not reponding, "
                          "DDS will terminate anyway.");
            }
            /* All participants of this kernel must be disabled! */
            r = kernelDisable(k);
            if (result != os_resultSuccess) {
                r = U_RESULT_INTERNAL_ERROR;
            }
            if (k->shm != NULL) {
#ifndef INTEGRITY
                if (unlockSharedMemory(k) != 0) {
                    OS_REPORT(OS_WARNING,"u_kernelFree", 0,
                              "Could not unlock shared segment from memory.");
                }
#endif
                result = os_sharedMemoryDetach(k->shm);
                if (result != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,
                              "u_kernelFree", 0,
                              "Detach from shared memory failed.");
                } else {
                    result = os_sharedMemoryDestroy(k->shm);
                    if (result != os_resultSuccess) {
                        OS_REPORT(OS_WARNING,
                                  "u_kernelFree", 0,
                                  "Destroy shared memory failed.");
                    } else {
                        os_sharedDestroyHandle(k->shm);
                    }
                }
            } else {
                result = os_resultSuccess;
            }
            c_iterFree(k->participants);
            os_free(k->uri);
            k->participants = NULL;
            k->uri = NULL;
            os_free(k);
            r = u_userUnprotect(NULL);
        }

    } else {
        OS_REPORT(OS_WARNING,"u_kernelFree", 0,
                  "The specified Kernel = NIL.");
        r = U_RESULT_OK;
    }
    return r;
}

c_voidp
u_kernelGetCopy(
    u_kernel _this,
    u_entityCopy copy,
    void *copyArg)
{
    void *result;
    u_result r;
    v_kernel vk;

    result = NULL;

    if ((_this != NULL) && (copy != NULL)) {
        r = u_kernelClaim(_this,&vk);
        if ((r == U_RESULT_OK) && (vk != NULL)) {
            result = copy((v_entity)vk, copyArg);
            r = u_kernelRelease(_this);
        } else {
            OS_REPORT(OS_ERROR,"u_kernelGetCopy",0,
                      "Claim Kernel failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_kernelGetCopy", 0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_kernelAdd(
    u_kernel _this,
    u_participant p)
{
    c_long oldCount,newCount;
    u_result r;

    if ((_this != NULL) && (p != NULL)) {
        os_mutexLock(&_this->mutex);
        oldCount = c_iterLength(_this->participants);
        _this->participants = c_iterInsert(_this->participants,p);
        newCount = c_iterLength(_this->participants);
        if (newCount == (oldCount + 1)) {
            r = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR,"u_kernelAdd", 0,
                      "Internal error.");
            r = U_RESULT_INTERNAL_ERROR;
        }
        os_mutexUnlock(&_this->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_kernelAdd", 0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_kernelRemove(
    u_kernel kernel,
    u_participant p)
{
    u_participant o;
    u_result r;

    os_mutexLock(&kernel->mutex);
    o = c_iterTake(kernel->participants,p);
    if (o == NULL) {
        OS_REPORT(OS_ERROR,"u_kernelRemove", 0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    } else {
        r = U_RESULT_OK;
    }
    os_mutexUnlock(&kernel->mutex);
    return r;
}

const c_char *
u_kernelUri(
    u_kernel kernel)
{
    if (kernel != NULL) {
        return kernel->uri;
    } else {
        return "";
    }
}

u_result
u_kernelDetachParticipants(
    u_kernel kernel)
{
    u_result result;
    u_participant p;

    if (kernel != NULL) {
        p = u_participant(c_iterTakeFirst(kernel->participants));
        while (p != NULL) {
            (void)u_participantDetach(p);
            p = u_participant(c_iterTakeFirst(kernel->participants));
        }
        result = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,"u_kernelDetachParticipants", 0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_kernelCheckHandleServer(
    u_kernel kernel,
    c_long serverId)
{
    c_bool result = FALSE;

    if (kernel != NULL ) {
        result = v_kernelCheckHandleServer(kernel->kernel,serverId);
    } else {
        OS_REPORT(OS_ERROR,"u_kernelCheckHandleServer", 0,
                  "Illegal parameter.");
    }
    return result;
}

c_long
u_kernelHandleServer(
    u_kernel kernel)
{
    return (c_long)kernel->kernel->handleServer;
}

c_voidp
u_kernelAddress(
    u_kernel kernel)
{
    c_voidp address;

    if (kernel) {
        address =  kernel->kernel;
    } else {
        address =  NULL;
    }
    return address;
}
