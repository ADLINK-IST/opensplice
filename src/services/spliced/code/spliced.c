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
#include <assert.h>
#include <string.h>

#include "vortex_os.h"
#include "os_report.h"
#include "os_sharedmem.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include "c_stringSupport.h"
#include "v_leaseManager.h"

#include "spliced.h"
#include "v_spliced.h"
#include "s_misc.h"
#include "s_configuration.h"
#include "s_threadsMonitor.h"
#include "report.h"
#include "serviceMonitor.h"
#include "s_kernelManager.h"
#include "s_shmMonitor.h"
#include "s_gc.h"
#include "sr_componentInfo.h"
#include "ut_entryPoint.h"
#include "u_domain.h"
#include "u_serviceManager.h"
#include "u_observable.h"
#include "os_signalHandler.h"
#include "sr_componentInfo.h"
#include "ut_thread.h"

#ifdef OSPL_ENV_SHMT
#include <dlfcn.h>
#include <link.h>

#define PURE_MAIN_SYMBOL "ospl_main"
#endif


C_STRUCT(spliced)
{
    s_configuration         config;
    u_spliced               service;        /* splicedaemon service/participant */
    u_serviceManager        serviceManager;
    serviceMonitor          serviceMon;
    c_bool                  isSingleProcess;
    os_uint32               nrKnownServices;
    os_uint32               nrTerminatedServices;
    ut_thread               heartbeatManager;
    ut_thread               durabilityClient;
    sr_componentInfo        *knownServices;
    s_kernelManager         km;
    s_garbageCollector      gc;
    s_shmMonitor            shmMonitor;
    os_uint32               nrApplications;
    sr_componentInfo        *applications;
    struct terminate {
        int                     flag;
        int                     shmClean;
        os_cond                 cond;
        os_mutex                mtx;
    } terminate;
    u_service_cmdopts           cfg_handle;
    ut_threads                  threads;
    s_threadsMonitor            threadsMonitor;
};

/**************************************************************
 * Private functions
 **************************************************************/

static int
argumentsCheck(
    spliced _this,
    int argc,
    char *argv[])
{
    int retCode = SPLICED_EXIT_CODE_CONTINUE;
    const char *name = argv[0];

    assert(_this);
    assert(name);

    if ( argc > 0 )
    {
      if ((_this->cfg_handle.exeName = os_strdup(name)) == NULL) {
           OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                   "Memory claim (%"PA_PRIdSIZE" bytes) denied; could not duplicate string '%s'.",
                   strlen(name) + 1,
                   name);
           retCode = SPLICED_EXIT_CODE_RECOVERABLE_ERROR;
           goto err_name;
       }
    }

    if ( argc < 2 ){
#ifdef CONF_PARSER_NOFILESYS
        _this->cfg_handle.uri = os_strdup("osplcfg://ospl.xml");
#else
        _this->cfg_handle.uri = NULL;
#endif
    } else if (argc == 2) {
         _this->cfg_handle.uri = os_strdup(argv[1]);
    } else {
        printf("usage: %s [<URI>]\n", name);
        printf("\n");
        printf("<URI> file://<abs path to configuration> \n");
        retCode = SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR;
    }
    u_serviceCheckEnvURI(&_this->cfg_handle);
 err_name:
    return retCode;
}

static void *
leaseRenewThread(
    void *arg)
{
    spliced _this;
    int flag;
    u_result result;
    os_timeM renewed, prevRenewed;
    os_duration lag;
    ut_thread self;

    assert(arg);

    _this = (spliced)arg;
    self = ut_threadLookupSelf(_this->threads);

    /* Set the previous renew-time to maximum value that can be passed to
     * os_timeMDiff(...), so no lag is reported for the first lease renewal. */
    prevRenewed = OS_TIMEM_INFINITE;

    do {
        result = u_serviceRenewLease(u_service(_this->service), _this->config->leasePeriod);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to update service lease");
        }
        renewed = os_timeMGet();
        lag = os_timeMDiff(renewed, prevRenewed);
        prevRenewed = renewed;
        if (os_durationCompare(lag, _this->config->leasePeriod) == OS_MORE) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                        "Splice-daemon failed to renew its lease within "
                        "the configured lease expiry-time (%"PA_PRId64".%09d). "
                        "The lease renewal took %"PA_PRId64".%09d s.",
                        OS_DURATION_GET_SECONDS(_this->config->leasePeriod),
                        OS_DURATION_GET_NANOSECONDS(_this->config->leasePeriod),
                        OS_DURATION_GET_SECONDS(lag), OS_DURATION_GET_NANOSECONDS(lag));
        }
        /* Terminate flag is also accessed outside the mutex lock. This is no problem,
         * protection guarantees no wait entry while setting the flag. */
        os_mutexLock(&_this->terminate.mtx);
        if ((flag = _this->terminate.flag) == SPLICED_EXIT_CODE_CONTINUE) {
            ut_condTimedWait(self, &_this->terminate.cond, &_this->terminate.mtx, _this->config->leaseRenewalPeriod);
        }
        os_mutexUnlock(&_this->terminate.mtx);
    } while ((flag == SPLICED_EXIT_CODE_CONTINUE) && (result == U_RESULT_OK));

    {
        result = u_serviceRenewLease(u_service(_this->service), 300*OS_DURATION_SECOND);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to update service lease");
        }
    }

    return NULL;
}

static void heartbeatManagerAction(v_public p, void *arg)
{
    v_leaseManager lm;

    OS_UNUSED_ARG(arg);

    lm = v_splicedGetHeartbeatManager(v_spliced(p), FALSE);
    assert(lm);
    OS_REPORT(OS_INFO,
              "spliced::heartbeatManagerThread", 1,
              "Heartbeat Manager for spliced started");

    v_leaseManagerMain(lm);
    OS_REPORT(OS_INFO,
              "spliced::heartbeatManagerThread", 1,
              "Heartbeat Manager for spliced stopped");
    c_free(lm);
}

static void *
heartbeatManagerThread(
    void *arg)
{
    spliced _this = (spliced)arg;
    ut_thread self = ut_threadLookupSelf(_this->threads);
    u_result result;

    /* We can not detect progress here. So, simulate a thread sleep. */
    ut_threadAsleep(self, UT_SLEEP_INDEFINITELY);
    result = u_observableAction(u_observable(_this->service), heartbeatManagerAction, NULL);
    if (result != U_RESULT_OK) {
        OS_REPORT(OS_WARNING, ut_threadGetName(self), 0,
                  "Failed to retrieve heartbeat manager");
    }

    return NULL;
}

static c_bool
startHeartbeatManager(
    spliced _this)
{
    c_bool started = FALSE;
    v_leaseManager lm;
    u_result ures;

    _this->heartbeatManager = NULL;

    if (_this->config->heartbeatAttribute) {
        lm = u_splicedGetHeartbeatManager(_this->service, TRUE);
        if (lm) {
            ut_threadCreate(_this->threads, &(_this->heartbeatManager), S_THREAD_HEARTBEAT_THREAD,
                                   _this->config->heartbeatAttribute, heartbeatManagerThread, _this);
            if (_this->heartbeatManager != NULL) {
                ures = u_splicedStartHeartbeat(_this->service, _this->config->heartbeatExpiryTime,
                                               _this->config->heartbeatUpdateInterval);
                started = ures == U_RESULT_OK;
            } else {
                OS_REPORT(OS_WARNING, "spliced::startHeartbeatManager", 0,
                          "Failed to start heartbeat manager thread");
            }
            c_free(lm);
        } else {
            OS_REPORT(OS_WARNING, "spliced::startHeartbeatManager", 0,
                      "Failed to initialize heartbeat manager");
        }
    } else {
        ures = u_splicedStartHeartbeat(_this->service, _this->config->heartbeatExpiryTime,
                                       _this->config->heartbeatUpdateInterval);
        started = ures == U_RESULT_OK;
    }

    return started;
}

static void
stopHeartbeatManager(
     spliced _this)
{
    v_leaseManager lm;

    u_splicedStopHeartbeat(_this->service);

    lm = u_splicedGetHeartbeatManager(_this->service, FALSE);
    if (lm != NULL) {
        v_leaseManagerNotify(lm, NULL, V_EVENT_TERMINATE);
        if (_this->heartbeatManager != NULL) {
           ut_threadWaitExit(_this->heartbeatManager, NULL);
        }
        c_free(lm);
    }
    ut_threadAwake(ut_threadLookupSelf(_this->threads));
}

/* The function below is intended to retrieve the federation
 * specific partition name that is used by DDSI. Currently,
 * DDSI prevents alignment of almost all topics published on
 * this partition. To use this partition for client durability
 * an exception has to be made for the d_historicalData topic.
 * Until that is has not been implemented the use of this
 * partition is commented out.
 */
static char *
getFederationSpecificAlignmentPartitionName(
    spliced _this)
{
    u_result ures;
    char name[U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE];
    char *buf;

    /* For the federation specific partition we use the same partition name
     * that is used by ddsi. The name of this partition is a 35 byte name
     * __NODE%08"PA_PRIx32" BUILT-IN PARTITION__  where the systemId is
     * encoded as 8-byte hex representation.
     */
    ures = u_participantFederationSpecificPartitionName(u_participant(_this->service), name, sizeof(name));
    assert(ures == U_RESULT_OK);
    OS_UNUSED_ARG(ures);
    buf = os_strdup(name);
    return buf;
}


static char *
getPartitionName(
    spliced _this)
{
    char *buf;

    buf = os_strdup(_this->config->partition);
    return buf;
}


static c_bool
durabilityClientStart(spliced _this)
{
    os_threadAttr threadAttr;
    u_result ures;
    char *privatePartition;
    char *requestPartition;

    _this->durabilityClient = NULL;
    /* Get a federation specific alignment partition */
    privatePartition = getFederationSpecificAlignmentPartitionName(_this);
     /* Get the requestPartition */
    requestPartition = getPartitionName(_this);

    ures = u_splicedDurabilityClientSetup(
                            _this->service,
                            _this->config->durablePolicies,
                            requestPartition,
                            "clientDurabilityPartition",        /* LH: Global partition, currently not used */
                            privatePartition);

    os_free(requestPartition);
    os_free(privatePartition);

    if (ures == U_RESULT_OK) {
        os_threadAttrInit(&threadAttr);
        ut_threadCreate(_this->threads,
                        &(_this->durabilityClient),
                        S_THREAD_DURABILITYCLIENT,
                        &threadAttr,
                        u_splicedDurabilityClientMain,
                        _this->service);
        if (_this->durabilityClient != NULL) {
            /* We can not detect progress here. So, simulate a thread sleep. */
            ut_threadAsleep(_this->durabilityClient, UT_SLEEP_INDEFINITELY);
        } else {
            OS_REPORT(OS_WARNING, "spliced::durabilityClientStart", 0,
                      "Failed to start durability client thread");
        }
    } else {
        OS_REPORT(OS_WARNING, "spliced::durabilityClientStart", 0,
                  "Failed to setup durability client");
    }

    return (_this->durabilityClient != NULL);
}

static void
durabilityClientStop(spliced _this)
{
    u_result u_res;

    if (_this->durabilityClient != NULL) {
        u_res = u_splicedDurabilityClientTerminate(_this->service);
        if (u_res == U_RESULT_OK) {
            (void)ut_threadWaitExit(_this->durabilityClient, NULL);
        } else {
            OS_REPORT(OS_WARNING, "spliced::durabilityClientStop", 0,
                      "Failed to terminate durability client");
        }
    }
    ut_threadAwake(ut_threadLookupSelf(_this->threads));
}

const os_char*
splicedGetDomainName(
    spliced _this)
{
    assert(_this);
    assert(_this->config);
    assert(_this->config->domainName);

    return _this->config->domainName;
}

static os_result
exitRequestHandler(
    os_callbackArg ignore,
    void * arg)
{
    struct terminate *terminate;

    OS_UNUSED_ARG(ignore);

    terminate = (struct terminate*) arg;

    /* The os_callbackArg can be ignored, because setting the below flag will
     * cause proper exit of the spliced. */
    os_mutexLock(&terminate->mtx);
    terminate->flag = SPLICED_EXIT_CODE_OK;
    os_condBroadcast(&terminate->cond);
    os_mutexUnlock(&terminate->mtx);

    return os_resultSuccess; /* the main thread will take care of termination */
}

static void
splicedKnownServicesFree(
    spliced _this)
{
    os_uint32 i;

    assert(_this != NULL);

    for (i = 0; i < _this->nrKnownServices; i++) {
        sr_componentInfoFree(_this->knownServices[i]);
        _this->knownServices[i] = NULL;
    }
    os_free(_this->knownServices);
    _this->knownServices = NULL;
    _this->nrKnownServices = 0;
    _this->nrTerminatedServices = 0;
}

static void
splicedApplicationsFree(
    spliced _this)
{
    os_uint32 i;

    assert(_this != NULL);

    for (i = 0; i < _this->nrApplications; i++) {
        sr_componentInfoFree(_this->applications[i]);
        _this->applications[i] = NULL;
    }
    os_free(_this->applications);
    _this->applications = NULL;
    _this->nrApplications = 0;
}

static c_bool
serviceCommandIsValid(
    char **command)
{
    c_bool result = FALSE;
    char *suffixedCommand;
    char *fullCommand;

    assert(command != NULL);
    assert(*command != NULL);

    fullCommand = os_locate(*command, OS_ROK|OS_XOK);
    if (fullCommand) {
        os_free(*command);
        *command = fullCommand;
        result = TRUE;
    }
    if (!result && (sizeof(OS_EXESUFFIX) > 1)) {
        /* Try the same thing with the exe suffix attached */
        if (strstr(*command, OS_EXESUFFIX) == NULL) {
            suffixedCommand = os_malloc(strlen(*command) + sizeof(OS_EXESUFFIX));
            os_strcpy(suffixedCommand, *command);
            os_strcat(suffixedCommand, OS_EXESUFFIX);
            fullCommand = os_locate(suffixedCommand, OS_ROK|OS_XOK);
            if (fullCommand) {
                os_free(*command);
                *command = fullCommand;
                result = TRUE;
            }
        }
    }

    return result;
}

static char* splitOnFirstToken(char *original, char token)
{
   int i;
   for (i = 0; original[i] != '\0'; i++)
   {
      if (original[i] == token)
      {
         original[i] = '\0';
         return &original[++i];
      }
   }
   return NULL;
}

static os_result
deployLibrary(
   sr_componentInfo info
)
{
    os_library libraryHandle;
    os_libraryAttr libraryAttr;
    os_result result;
    struct ut_entryPointWrapperArg *mwa;
    os_threadAttr threadAttr;
    os_threadId id;
    int argc;
    char ** argv;
    char * tempString = NULL;
    char * libraryName = NULL;
    char * entryPoint = NULL;
    char * saveptr;

    /* First, determine the argc/argv parameters to the entry point.
     * The manner of these varies as to whether we are deploying
     * an internal service or a user application:
     *
     * The internal services require argv[1] to be the service "name"
     * specified in the xml, so when started, it knows which xml section
     * to parse in order to obtain its configuration.  Applications
     * do not have this requirement.
     */

    argc = 0;
    argv = os_malloc(256*sizeof(char *));
    argv[argc++] = info->command;

    if (info->isService)
    {
        argv[argc++] = info->name;
        if (info->configuration)
        {
            argv[argc++] = info->configuration;
        }
    }

    tempString = os_strtok_r(info->args, " ", &saveptr);
    while (tempString)
    {
       argv[argc++] = os_strdup (tempString);
       tempString = os_strtok_r(NULL, " ", &saveptr);
    }
    argv[argc] = NULL;

    mwa = os_malloc(sizeof(struct ut_entryPointWrapperArg));
    mwa->argc = argc;
    mwa->argv = argv;

    /* Initialise the library attributes */
    os_libraryAttrInit(&libraryAttr);

    /* Applications can have their library overriden if the user
     * specifies the library attribute */

    if (!info->isService && info->library && strlen(info->library) > 0)
    {
        libraryName = info->library;
    }
    else
    {
        libraryName = info->command;
    }

    /* Now open the library */
    libraryHandle = os_libraryOpen (libraryName, &libraryAttr);
    if (libraryHandle != NULL) {

        /* Locate the entry point */
        if (info->isService)
        {
            /* Derive entry point name from Command.  When deploying a service
             * we use the "ospl_" prefix as we have full control over the
             * names of these libraries */
            entryPoint = os_malloc (sizeof (char*) * (strlen(info->command) + 6));
            sprintf (entryPoint, "ospl_%s", info->command);
        }
        else
        {
            /* Derive entry point name from Command */
            entryPoint = os_malloc (sizeof (char*) * (strlen(info->command) + 6));
            sprintf (entryPoint, "%s", info->command);
        }

        /* Writing: mwa->entryPoint = (ut_entryPointFunc)os_libraryGetSymbol (libraryHandle, entryPoint);
         * would seem more natural, but results in: warning: ISO C forbids conversion of object pointer
         * to function pointer type [-pedantic]
         * dlopen man page suggests using the used assignment based on the POSIX.1-2003 (Technical
         * Corrigendum 1) workaround.
         */
        *(void **)&mwa->entryPoint = os_libraryGetSymbol (libraryHandle, entryPoint);

        if (mwa->entryPoint != NULL) {
            os_threadAttrInit(&threadAttr);

            threadAttr.stackSize = ((size_t)1024*1024);
#ifndef INTEGRITY
            threadAttr.schedClass = info->procAttr.schedClass;
            threadAttr.schedPriority = info->procAttr.schedPriority;
#endif

            /* Invoke the entry point as a new thread */
            result = os_threadCreate(&id, info->name, &threadAttr, ut_entryPointWrapper, mwa);
            if (result != os_resultSuccess)
            {
                result = os_resultFail;
                OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                            "Error starting thread for '%s'\n", info->name);
            }
            else
            {
                info->threadId = id;
                /* Don't believe that a wait/sleep is required here.
                 * historically there was one */
            }
        } else {
            result = os_resultFail;
            OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                        "Command '%s' not found\n", entryPoint);
        }
    } else {
        result = os_resultFail;
        OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                    "Problem opening '%s'\n",libraryName);
    }

    if (entryPoint)
    {
        os_free (entryPoint);
    }

    return result;
}

static int
startServices(
    spliced _this)
{
    int retCode = SPLICED_EXIT_CODE_CONTINUE;
    os_uint32 i;
    os_result createResult;
    sr_componentInfo info;
    c_char* args;
    os_size_t argc;
    ut_thread self;
    char *vg_cmd = NULL;
    char *command = NULL;
    char *vg_args = NULL;

    assert(_this != NULL);

    self = ut_threadLookupSelf(_this->threads);

#ifdef INTEGRITY
    if ( !os_getIsSingleProcess() )
    {
       Semaphore serviceStartSem;
       Error err;
       serviceStartSem = SemaphoreObjectNumber(12);
       err = ReleaseSemaphore(serviceStartSem);
       assert (err == Success);
       (void) err;
    }
#endif

    /*
    In case we haven't logged anything yet we call these methods now
    before we start any other domain services
    to (potentially) remove the previous versions of the files
    */
    os_free(os_reportGetInfoFileName());
    os_free(os_reportGetErrorFileName());

    for (i = 0; i < _this->nrKnownServices; i++) {
        ut_threadAwake(self);

        info = _this->knownServices[i];

        if (_this->isSingleProcess) {

            /* This is a 'SingleProcess' configuration so the service must be
             * deployed as a library.
             */

            createResult = deployLibrary (info);

            if (createResult == os_resultInvalid) {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED,
                            0, "Starting %s with <SingleProcess> not supported on this platform",
                            info->name);
                retCode = SPLICED_EXIT_CODE_INAPPLICABLE_CONFIGURATION;
                break;
            }
            else if (createResult != os_resultSuccess) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0, "Could not start service %s as thread",
                            info->name);
            }
        }
#ifndef INTEGRITY
        else
        {

#if !defined ( VXWORKS_RTP )&& !defined ( _WRS_KERNEL )
            if (serviceCommandIsValid(&info->command))
#else
            if (TRUE)
#endif
            {
                if (!strcmp(info->name,"networking"))
                {
                    vg_cmd = os_getenv("VG_NETWORKING");
                }
                else if (!strcmp(info->name,"durability"))
                {
                    vg_cmd = os_getenv("VG_DURABILITY");
                }
                else if (!strcmp(info->name,"snetworking"))
                {
                    vg_cmd = os_getenv("VG_SNETWORKING");
                }
                else if (!strcmp(info->name,"cmsoap"))
                {
                    vg_cmd = os_getenv("VG_CMSOAP");
                }

                if (!vg_cmd)
                {
                    command = os_strdup(info->command);
                    /* allocate with room for 2 quotes, a space, and an end-of-string */
                    argc = 1+strlen(info->name)+
                           3+strlen(info->args)+
                           3+strlen(info->configuration)+
                           2;
                }
                else
                {
                    /* get the valgrind command */
                    vg_args = splitOnFirstToken(vg_cmd, ' ');
                    command = os_locate(vg_cmd, OS_ROK|OS_XOK);
                    argc = 1+strlen(info->command)+
                           1+strlen(info->name)+
                           3+strlen(info->args)+
                           1+strlen(vg_args)+
                           3+strlen(info->configuration)+
                           2;
                }
                args = os_malloc(argc);
                if (strlen(info->args) == 0) {
                    if (!vg_cmd) {
                        snprintf(args, argc, "\"%s\" \"%s\"", info->name, info->configuration);
                    } else {
                        snprintf(args, argc, "%s \"%s\" \"%s\" \"%s\"", vg_args, info->command, info->name, info->configuration);
                    }
                } else {
                    if (!vg_cmd) {
                        snprintf(args, argc, "\"%s\" \"%s\" \"%s\"", info->name, info->configuration, info->args);
                    } else {
                        snprintf(args, argc, "%s \"%s\" \"%s\" \"%s\" \"%s\"", vg_args, info->command, info->name, info->configuration, info->args);
                    }
                }

                createResult = os_procCreate(command,
                                             info->name, args,
                                             &info->procAttr, &info->procId);

                if (createResult == os_resultSuccess)
                {
                    os_sharedMemoryRegisterUserProcess(splicedGetDomainName(_this), info->procId);
                    OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED,
                                0, "Started service %s with args %s",
                                info->name, args);
                }
                else
                {
                    OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                                0, "Could not start service %s with args %s",
                                info->name, args);
                }

                os_free(args);
                os_free(command);
            }
            else
            {
                retCode = SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR;
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                            "Could not find file '%s' with read and execute permissions",
                            info->command);
                break;
            }
        }
#endif /* !INTEGRITY */
    }
    return retCode;
}

static void
startApplications(
    spliced _this)
{
    os_uint32 i;
    os_result deployResult;
    ut_thread self;

    assert(_this != NULL);

    self = ut_threadLookupSelf(_this->threads);

    for (i = 0; i < _this->nrApplications; i++) {
        ut_threadAwake(self);

        deployResult = deployLibrary (_this->applications[i]);

        if (deployResult == os_resultInvalid) {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED,
                            0, "Starting Application %s with <SingleProcess> not supported on this platform",
                            _this->applications[i]->name);
        }
        else if (deployResult != os_resultSuccess) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0, "Could not start application %s as thread",
                            _this->applications[i]->name);
        }
    }
}

/**************************************************************
 * configuration
 **************************************************************/
static void
getSingleProcessValue(
    spliced _this,
    u_cfElement spliceCfg)
{
    assert(_this != NULL);

    if (spliceCfg != NULL) {

        c_char * value;
        c_bool result;
        u_cfData node;
        c_iter iter;

        iter = u_cfElementXPath(spliceCfg, "SingleProcess/#text");
        if (iter) {
            node = u_cfData(c_iterTakeFirst(iter));
            if (node != NULL) {
                result = u_cfDataStringValue(node, &value);
                if (result) {
                    if (os_strcasecmp (value, "True" ) == 0) {
                        _this->isSingleProcess = 1;
                    }
                    os_free(value);
                }
                u_cfDataFree(node);
            }
            c_iterFree(iter);
        }
    }
}

static void
getKnownServices(
    spliced _this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    os_uint32 i;

    assert(_this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "Service");
        _this->nrKnownServices = c_iterLength(services);
        if (_this->nrKnownServices > 0) {
            _this->knownServices = os_malloc(_this->nrKnownServices * sizeof(sr_componentInfo));
            i = 0;
            s = c_iterTakeFirst(services);
            while (s != NULL) {
                _this->knownServices[i] = sr_componentInfoServiceNew(s, _this->cfg_handle.uri);
                u_cfElementFree(s);
                s = c_iterTakeFirst(services);
                if (_this->knownServices[i] != NULL) {
                    i++;
                }
            }
        }
        c_iterFree(services);
        _this->nrKnownServices = i;
    }
}

static void
getApplications(
    spliced _this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    os_uint32 i;

    assert(_this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "Application");
        _this->nrApplications = c_iterLength(services);
        if (_this->nrApplications > 0) {
            _this->applications = (sr_componentInfo *)os_malloc((os_uint)(_this->nrApplications *
                                                            (int)sizeof(sr_componentInfo)));
            if (_this->applications != NULL) {
                i = 0;
                s = c_iterTakeFirst(services);
                while (s != NULL) {
                    _this->applications[i] = sr_componentInfoApplicationNew(s, _this->cfg_handle.uri);
                    u_cfElementFree(s);
                    s = c_iterTakeFirst(services);
                    if (_this->applications[i] != NULL) {
                        i++;
                    }
                }
            }
        }
        c_iterFree(services);
        _this->nrApplications = i;
    }
}

static void
readConfiguration(
    spliced _this)
{
    u_cfElement cfg;
    u_cfElement dc;
    c_iter      domains;

    assert(_this);
    assert(_this->service);
    assert(_this->config);

    s_configurationRead(_this->config, _this);
    cfg = u_participantGetConfiguration(u_participant(_this->service));
    if (cfg != NULL) {
        domains = u_cfElementXPath(cfg, "Domain");
        dc = c_iterTakeFirst(domains);
        if (dc != NULL) {
            getSingleProcessValue (_this, dc);
            getKnownServices(_this, dc);
            getApplications(_this, dc);
            u_cfElementFree(dc);
            dc = c_iterTakeFirst(domains);
            while(dc){
                u_cfElementFree(dc);
                dc = c_iterTakeFirst(domains);
            }
        }
        c_iterFree(domains);
        u_cfElementFree(cfg);
    }
}

/**************************************************************
 * Public functions
 **************************************************************/
s_configuration
splicedGetConfiguration(
    spliced spliceDaemon)
{
    assert(spliceDaemon != NULL);
    return spliceDaemon->config;
}

u_spliced
splicedGetService(
    spliced spliceDaemon)
{
    assert(spliceDaemon != NULL);
    return spliceDaemon->service;
}


ut_threads
splicedGetThreads(
    spliced spliceDaemon)
{
    assert(spliceDaemon != NULL);
    return spliceDaemon->threads;
}

u_serviceManager
splicedGetServiceManager(
    spliced spliceDaemon)
{
    u_serviceManager m;

    assert(spliceDaemon != NULL);

    m = spliceDaemon->serviceManager;

    return m;
}

s_shmMonitor
splicedGetShmMonitor(
    spliced _this)
{
    assert(_this != NULL);
    return _this->shmMonitor;
}

void
splicedSignalTerminate(
    spliced _this,
    int code,
    int shmClean)
{
    assert(_this);

    os_mutexLock(&_this->terminate.mtx);
    if (_this->terminate.flag == SPLICED_EXIT_CODE_CONTINUE) {
        _this->terminate.flag = code;
        _this->terminate.shmClean = shmClean;
        os_condBroadcast(&_this->terminate.cond);
    }
    os_mutexUnlock(&_this->terminate.mtx);
}

static int
splicedGetTerminateFlag(
        spliced _this)
{
    int flag;

    assert(_this);

    os_mutexLock(&_this->terminate.mtx);
    flag = _this->terminate.flag;
    os_mutexUnlock(&_this->terminate.mtx);

    return flag;
}

static int
splicedIsShmOk(
    spliced _this)
{
    int result;

    assert(_this);

    os_mutexLock(&_this->terminate.mtx);
    result = _this->terminate.shmClean;
    os_mutexUnlock(&_this->terminate.mtx);

    return result;
}

os_boolean
splicedIsDoingSystemHalt(
    spliced _this)
{
    os_boolean result;

    assert(_this);

    os_mutexLock(&_this->terminate.mtx);
    result = _this->terminate.flag > SPLICED_EXIT_CODE_OK;
    os_mutexUnlock(&_this->terminate.mtx);

    return result;
}

os_result
splicedRemoveServiceInfo(
    spliced spliceDaemon,
    const c_char *name)
{
    os_uint32 i;
    os_int32 status;
    os_result result = os_resultFail;

    assert(spliceDaemon != NULL);

    i = 0;
    while (result == os_resultFail && (i < spliceDaemon->nrKnownServices)) {
        if ((spliceDaemon->knownServices[i] != NULL) && (strcmp(spliceDaemon->knownServices[i]->name, name) == 0)) {
            if (os_threadIdToInteger(spliceDaemon->knownServices[i]->threadId) != os_threadIdToInteger(OS_THREAD_ID_NONE)) {
                assert (spliceDaemon->knownServices[i]->procId == OS_INVALID_PID);
                /* Don't try to wait for the thread: service could have died by means of a deadlock. */
                spliceDaemon->knownServices[i]->threadId = OS_THREAD_ID_NONE;
            } else if (spliceDaemon->knownServices[i]->procId != OS_INVALID_PID) {
                assert (os_threadIdToInteger(spliceDaemon->knownServices[i]->threadId) == os_threadIdToInteger(OS_THREAD_ID_NONE));
                result = os_procCheckStatus(spliceDaemon->knownServices[i]->procId, &status);
                if (result == os_resultSuccess) {
                    /* Only remove service from list when process is no longer known by OS */
                    sr_componentInfoFree(spliceDaemon->knownServices[i]);
                    spliceDaemon->knownServices[i] = NULL;
                }
            }
            result = os_resultSuccess;
        } else {
            i++;
        }
    }
    return result;
}

void
splicedRemoveKnownService(
    spliced _this,
    const c_char *name)
{
    os_result result;

    assert(_this);

    result = splicedRemoveServiceInfo(_this,name);
    if(result != os_resultSuccess)
    {
        OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED, 0,
               "Unable to remove service %s from the knownservices list",
               name);
    } else {
        _this->nrTerminatedServices++;
    }
}

sr_componentInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name)
{
    os_uint32 i;
    sr_componentInfo ci;

    assert(spliceDaemon != NULL);

    ci = NULL;
    i = 0;
    while ((ci == NULL) && (i < spliceDaemon->nrKnownServices)) {
        if ((spliceDaemon->knownServices[i] != NULL) && (strcmp(spliceDaemon->knownServices[i]->name, name) == 0)) {
            ci = spliceDaemon->knownServices[i];
        } else {
            i++;
        }
    }

    return ci;
}

#if 0
static c_bool
deleteContainedEntitiesForApplParticipants(
    u_participant participant,
    c_voidp arg)
{
    u_result result;

    assert(participant);

    OS_UNUSED_ARG(arg);

    if(u_objectKind(u_object(participant)) != U_SERVICE) {
        result = u_participantDeleteContainedEntities(participant);
        if(result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                    "An error occurred during termination. Unable to delete "
                    "contained entities of participant '0x%x'; "
                    "u_participantDeleteContainedEntities(...) returned %s.",
                    participant, os_resultImage(result));
        } /* when this fails, continue with the next participant */
    }
    return TRUE;
}
#endif


static void
waitForServices(
    spliced _this)
{
    os_uint32 j;
    int cmp;
    os_uint32 terminateCount;
    c_iter names;
    c_char *name;
    os_int32 status;
    ut_thread self = ut_threadLookupSelf(_this->threads);

    /* serviceManager might be NULL, when spliced has detected other spliced */
    if (_this->serviceManager != NULL) {
        os_duration pollDelay = 100*OS_DURATION_MILLISECOND;
        os_timeM curTime;
        os_timeM stopTime;

        curTime = os_timeMGet();
        stopTime = os_timeMAdd(curTime, _this->config->serviceTerminatePeriod);
        terminateCount = 0;

        ut_threadAsleep(self, (os_uint32)OS_DURATION_GET_SECONDS(_this->config->serviceTerminatePeriod));
        do {
            /* Wait for services to reach final state TERMINATED.
             */
            names = u_serviceManagerGetServices(_this->serviceManager, STATE_TERMINATED);
            name = c_iterTakeFirst(names);
            while (name != NULL) {
                for (j = 0; j < _this->nrKnownServices; j++) {
                    if (_this->knownServices[j] != NULL ) {
                        cmp = strcmp(name, _this->knownServices[j]->name);
                        if (cmp == 0) {
                            if (os_threadIdToInteger(_this->knownServices[j]->threadId) != os_threadIdToInteger(OS_THREAD_ID_NONE)) {
                                assert (_this->knownServices[j]->procId == OS_INVALID_PID);
                                if (os_threadWaitExit(_this->knownServices[j]->threadId, NULL) == os_resultSuccess) {
                                    _this->knownServices[j]->threadId = OS_THREAD_ID_NONE;
                                    terminateCount++;
                                }
                            } else if (_this->knownServices[j]->procId != OS_INVALID_PID) {
                                assert (os_threadIdToInteger(_this->knownServices[j]->threadId) == os_threadIdToInteger(OS_THREAD_ID_NONE));
                                if (os_procCheckStatus(_this->knownServices[j]->procId, &status) == os_resultSuccess) {
                                    _this->knownServices[j]->procId = OS_INVALID_PID;
                                    terminateCount++;
                                }
                            }
                        }
                    }
                }
                os_free(name);
                name = c_iterTakeFirst(names);
            }
            c_iterFree(names);

            curTime = os_timeMGet();
            if( (os_timeMCompare(curTime, stopTime) == OS_LESS) &&
                (terminateCount < _this->nrKnownServices)) {
                os_sleep(pollDelay);
            }
            curTime = os_timeMGet();
        } while ((os_timeMCompare(curTime, stopTime) == OS_LESS) && (terminateCount < (_this->nrKnownServices - _this->nrTerminatedServices)));
        ut_threadAwake(self);
    }
}

static os_uint32
killServices(
    spliced _this,
    os_boolean waited)
{
    os_uint32 count = 0;
    os_uint32 j;
    os_int32 status;
    os_result result;

    if (_this->isSingleProcess) {
        for (j = 0; j < _this->nrKnownServices; j++) {
            if(_this->knownServices[j] && os_threadIdToInteger(_this->knownServices[j]->threadId) != os_threadIdToInteger(OS_THREAD_ID_NONE)) {
                OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                          0, "Service '%s' did not terminate within serviceTerminatePeriod, thread will be forcefully destroyed.",
                          _this->knownServices[j]->name);
            }
        }
    } else {
        for (j = 0; j < _this->nrKnownServices; j++) {
            /* service may have been removed already (see splicedRemoveServiceInfo),
             * so check for NULL here.
             */
            if(_this->knownServices[j]){
                if (_this->knownServices[j]->procId != OS_INVALID_PID) {
                    result = os_procCheckStatus(_this->knownServices[j]->procId, &status);
                    if (result == os_resultSuccess) {
                        _this->knownServices[j]->procId = OS_INVALID_PID;
                    } else if (result == os_resultBusy){ /* Only kill if pid is (still) a child */
                        if(waited){
                            OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                                      0, "Service '%s' <%u> did not terminate within serviceTerminatePeriod, sending KILL signal",
                                      _this->knownServices[j]->name,
                                      _this->knownServices[j]->procId);
                        } else {
                            OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                                      0, "Forcing termination of service '%s' <%u>",
                                      _this->knownServices[j]->name,
                                      _this->knownServices[j]->procId);
                        }
                        os_procDestroy(_this->knownServices[j]->procId, OS_SIGKILL);
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

static os_uint32
stopServices(
    spliced _this,
    os_boolean wait)
{
    os_uint32 count;
    ut_thread self = ut_threadLookupSelf(_this->threads);

    assert(splicedGetTerminateFlag(_this) != SPLICED_EXIT_CODE_CONTINUE);
    if(wait){
        waitForServices(_this);
        ut_threadAwake(self);
    }

    /* Kill all not stopped services. */
    count = killServices(_this, wait);
    ut_threadAwake(self);

    return count;
}

static os_sharedHandle
sharedMemoryHandle(
    spliced _this)
{
    u_domain domain;
    domain = u_participantDomain(u_participant(_this->service));
    return u_domainSharedMemoryHandle(domain);
}

static void
forcedShmDestroy(
        os_sharedHandle shm)
{
    char *dn = NULL;
    char *fname;

    if (os_sharedMemoryGetNameFromId(shm, &dn) == os_resultSuccess) {
        assert(dn);
        fname = os_findKeyFile(dn);
        if (fname != NULL) {
            (void) os_sharedMemorySegmentFree(fname);
            (void) os_destroyKeyFile(fname);
            os_free(fname);
        }
    }
    os_free(dn);
}

static c_bool
setState(
    spliced _this,
    v_serviceStateKind state)
{
    c_bool ok = TRUE;
    os_sharedHandle shmHdl;

    /* We have shmHdl == NULL when running in single process. */
    shmHdl = sharedMemoryHandle(_this);
    if(shmHdl != NULL){
        os_state oState;
        switch (state) {
            case STATE_INITIALISING:
                oState = OS_STATE_INITIALIZING;
                break;
            case STATE_OPERATIONAL:
                oState = OS_STATE_OPERATIONAL;
                break;
            case STATE_TERMINATING:
                oState = OS_STATE_TERMINATING;
                break;
            case STATE_TERMINATED:
                oState = OS_STATE_TERMINATED;
                break;
            default:
                oState = OS_STATE_NONE;
                break;
        }

        if (os_sharedMemorySetState(shmHdl, oState) != os_resultSuccess) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                    "Failed to update shared memory state to %d.", (int)oState);
            ok = FALSE;
        }
    }

    if (!u_serviceChangeState(u_service(_this->service), state)) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to update service state to %d.", (int)state);
        ok = FALSE;
    }

    return ok;
}

/**************************************************************
 * Main
 **************************************************************/
OPENSPLICE_ENTRYPOINT (ospl_spliced)
{
    os_sharedHandle shm = NULL;
    spliced _this;
    int retCode;
    ut_thread lrt;
    os_result osresult;
    u_result uresult;
    ut_thread self;
    os_signalHandlerExitRequestHandle erh = os_signalHandlerExitRequestHandleNil;

    /* Set the flag in the user layer that spliced is running in this process.
     * For a hybrid multi-domain SHM and SP deployment this is not OK. */
    u_splicedSetInProcess(TRUE);

    if((uresult = u_userInitialise()) != U_RESULT_OK){
        /* Error reported by u_userInitialize(). */
        retCode = SPLICED_EXIT_CODE_RECOVERABLE_ERROR;
        goto err_userInitialize;
    }

    _this = os_malloc(sizeof *_this);

    if((osresult = os_mutexInit(&_this->terminate.mtx, NULL)) != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
              "Mutex initialization failed; os_mutexInit returned %s.", os_resultImage(osresult));
        retCode = SPLICED_EXIT_CODE_RECOVERABLE_ERROR;
        goto err_mutexInit;
    }

    if((osresult = os_condInit(&_this->terminate.cond, &_this->terminate.mtx, NULL)) != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
              "Condition variable initialization failed; os_condInit returned %s.", os_resultImage(osresult));
        retCode = SPLICED_EXIT_CODE_RECOVERABLE_ERROR;
        goto err_condInit;
    }

    _this->terminate.flag = SPLICED_EXIT_CODE_CONTINUE;
    _this->terminate.shmClean = SPLICED_SHM_OK;

    if((_this->config = s_configurationNew()) == NULL){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                      "Failed to allocate and initialize configuration storage.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_configurationNew;
    }

    _this->threads = ut_threadsNew(s_main_tread_name, _this->config->serviceTerminatePeriod, S_THREAD_CNT_MAX, (void*)_this);
    if (_this->threads == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                      "Failed to allocate and initialize threads management.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_threadsManager;
    }
    self = ut_threadLookupSelf(_this->threads);

    _this->threadsMonitor = s_threadsMonitorNew(_this);
    if (_this->threadsMonitor == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                      "Failed to allocate and initialize threads monitor.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_threadsMonitor;
    }

#ifdef EVAL_V
    OS_REPORT(OS_INFO,"The OpenSplice domain service", 0,
          "++++++++++++++++++++++++++++++++" OS_REPORT_NL
          "++ spliced EVALUATION VERSION ++" OS_REPORT_NL
          "++++++++++++++++++++++++++++++++\n");
#endif

    if((retCode = argumentsCheck(_this, argc, argv)) != SPLICED_EXIT_CODE_CONTINUE){
        /* Error reported by argumentsCheck(...) */
        splicedSignalTerminate(_this, retCode, SPLICED_SHM_OK);
        goto err_nameOrUri;
    }

    uresult = u_splicedNew(&(_this->service), _this->cfg_handle.uri);
    if (uresult != U_RESULT_OK) {
        /* Errors reported by u_splicedNew(...). */
        if (uresult == U_RESULT_PRECONDITION_NOT_MET) {
            splicedSignalTerminate(_this, SPLICED_EXIT_CODE_ALREADY_OPERATIONAL, SPLICED_SHM_OK);
        } else if (uresult == U_RESULT_ILL_PARAM) {
            splicedSignalTerminate(_this, SPLICED_EXIT_CODE_INAPPLICABLE_CONFIGURATION, SPLICED_SHM_OK);
        } else {
            splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        }
        goto err_usplicedNew;
    }

    if (!setState(_this, STATE_INITIALISING)) {
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_changeStateInitializing;
    }

    _this->isSingleProcess = 0;
    _this->nrKnownServices = 0;
    _this->nrTerminatedServices = 0;
    _this->knownServices = NULL;
    _this->nrApplications = 0;
    _this->applications = NULL;
    readConfiguration(_this);

    if(!_this->isSingleProcess){
#ifndef INTEGRITY
        erh = os_signalHandlerRegisterExitRequestCallback(exitRequestHandler, &_this->terminate);
#endif
        if ((_this->shmMonitor = s_shmMonitorNew(_this)) == NULL) {
            /* Error reported by s_shmMonitorNew(...). */
            splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
            goto err_shmMonitor;
        }
    } else {
        _this->shmMonitor = NULL;
    }

    if((_this->serviceManager = u_serviceManagerNew(u_participant(_this->service))) == NULL){
        /* Error reported by u_serviceManagerNew(...). */
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_userviceManagerNew;
    }

    if ((_this->km = s_kernelManagerNew(_this)) == NULL) {
        /* Error reported by s_kernelManagerNew(...). */
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_kernelManagerNew;
    }

    s_kernelManagerWaitForActive(_this->km);

    if ((_this->gc = s_garbageCollectorNew(_this)) == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to create the garbage collector.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_garbageCollectorNew;
    }

    s_garbageCollectorWaitForActive(_this->gc);

    if ((_this->serviceMon = serviceMonitorNew(_this)) == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to create the service monitor.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_serviceMonitorNew;
    }

    if (!startHeartbeatManager(_this)) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to start the heartbeat manager.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_startHeartbeatManager;
    }

    /* Start the client durability thread only if durable policies are configured */
    _this->durabilityClient = NULL;
    if (c_iterLength(_this->config->durablePolicies) != 0) {
        if (!durabilityClientStart(_this)) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                    "Failed to start the durability client.");
            splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
            goto err_startDurabilityClient;
        }
    }

    /* Start the spliced lease renewal thread */
    ut_threadCreate(_this->threads, &(lrt), S_THREAD_LEASE_RENEW_THREAD, &_this->config->leaseRenewAttribute, leaseRenewThread, _this);
    if (lrt == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to start lease renew thread.");
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_startLeaseRenewThread;
    }

    if (!setState(_this, STATE_OPERATIONAL)) {
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        goto err_changeStateOperational;
    }

    /* Start services */
    if((retCode = startServices(_this)) != SPLICED_EXIT_CODE_CONTINUE){
        /* Error reported by startServices(...). */
        splicedSignalTerminate(_this, retCode, SPLICED_SHM_OK);
        goto err_startServices;
    }

    /* Start applications specified in XML on a best effort basis. Note
     * this will only succeed if this is a SingleProcess configuration,
     * and leave warning messages otherwise. */
    startApplications(_this);

    /**************************************************************************/
    /*                            RUNNING PHASE                               */
    /**************************************************************************/
    os_mutexLock(&_this->terminate.mtx);
    while (_this->terminate.flag == SPLICED_EXIT_CODE_CONTINUE) {
        (void)ut_condWait(self, &_this->terminate.cond, &_this->terminate.mtx);
    }
    os_mutexUnlock(&_this->terminate.mtx);

    setState(_this, STATE_TERMINATING);

    /**************************************************************************/
    /*                          TERMINATION PHASE                             */
    /**************************************************************************/
    if(!splicedIsDoingSystemHalt(_this)){
        splicedSignalTerminate(_this, SPLICED_EXIT_CODE_OK, SPLICED_SHM_OK);
    }

    if(!splicedIsShmOk(_this)){
        goto fatal_shmNok;
    }
    if (_this->cfg_handle.exeName != NULL) {
        os_free(_this->cfg_handle.exeName);
    }

err_startServices:
    assert(splicedGetTerminateFlag(_this) != SPLICED_EXIT_CODE_CONTINUE);
    setState(_this, STATE_TERMINATING);
    if(stopServices(_this, OS_TRUE) > 0) {
        shm = sharedMemoryHandle(_this);
    }
err_changeStateOperational:
    assert(splicedGetTerminateFlag(_this) != SPLICED_EXIT_CODE_CONTINUE);
    (void) ut_threadWaitExit(lrt, NULL);
    setState(_this, STATE_TERMINATING);
err_startLeaseRenewThread:
    durabilityClientStop(_this);
err_startDurabilityClient:
    stopHeartbeatManager(_this);
err_startHeartbeatManager:
    serviceMonitorStop(_this->serviceMon);
    serviceMonitorFree(_this->serviceMon);
err_serviceMonitorNew:
    u_splicedPrepareTermination(_this->service);
    s_garbageCollectorFree(_this->gc);
err_garbageCollectorNew:
    u_splicedPrepareTermination(_this->service);
    s_kernelManagerFree(_this->km);
err_kernelManagerNew:
    u_objectFree(_this->serviceManager);
err_userviceManagerNew:
    s_shmMonitorFree(_this->shmMonitor);
err_shmMonitor:
    splicedKnownServicesFree(_this);
    splicedApplicationsFree(_this);
err_changeStateInitializing:
    setState(_this, STATE_TERMINATED);
    u_splicedSetInProcess(FALSE);
    os_signalHandlerUnregisterExitRequestCallback(erh);
    u_objectFree(_this->service);
    if(shm){
        /* Normally SHM is expected to be gone if spliced terminates normally.
         * If however one of the services was killed during shutdown, we try a
         * little harder to wipe the traces. */
        forcedShmDestroy(shm);
    }
err_usplicedNew:
    os_free(_this->cfg_handle.uri);
err_nameOrUri:
    s_threadsMonitorFree(_this->threadsMonitor);
err_threadsMonitor:
    ut_threadsFree(_this->threads);
err_threadsManager:
    s_configurationFree(_this->config);
err_configurationNew:
    assert(splicedGetTerminateFlag(_this) != SPLICED_EXIT_CODE_CONTINUE);
    retCode = splicedGetTerminateFlag(_this);
    os_condDestroy(&_this->terminate.cond);
err_condInit:
    os_mutexDestroy(&_this->terminate.mtx);
err_mutexInit:
    os_free(_this);
    /* No explicit undo for u_userInitialize(). */
err_userInitialize:
    return retCode;


    /**************************************************************************/
    /*                  ERROR HANDLING IN CASE OF BROKEN SHM                  */
    /**************************************************************************/
fatal_shmNok:
    /* Resources are leaked in this case */
    if(stopServices(_this, OS_FALSE) > 0 ){
        shm =  sharedMemoryHandle(_this);
        forcedShmDestroy(shm);
    }
    retCode = splicedGetTerminateFlag(_this);

    return retCode;
}

