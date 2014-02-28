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
#include <assert.h>
#include <string.h>

#include "os.h"
#include "os_report.h"
#include "os_sharedmem.h"

#include "c_typebase.h"
#include "c_stringSupport.h"
#include "v_leaseManager.h"

#include "spliced.h"
#include "s_misc.h"
#include "s_configuration.h"
#include "report.h"
#include "serviceMonitor.h"
#include "s_kernelManager.h"
#include "s_gc.h"
#include "sr_componentInfo.h"
#include "u_scheduler.h"
#include "dds_builtInTypes_register.h"
#include "ut_entryPoint.h"
#include "u_domain.h"
#include "os_signalHandler.h"
#include "sr_componentInfo.h"

#ifdef OSPL_ENV_SHMT
#include <dlfcn.h>
#include <link.h>

#define PURE_MAIN_SYMBOL "ospl_main"
#endif

#ifdef CONF_PARSER_NOFILESYS
#define URI_FILESCHEMA "file://"

/* Defined in output of osplconf2c */
extern const char ospl_xml_data[];
extern const unsigned int ospl_xml_data_size;
/* Defined in config parser */
extern char *ospl_xml_data_ptr;
extern unsigned int ospl_xml_data_ptr_size;
#endif

static void
splicedExit(
    const char *msg,
    int result);

struct HeartbeatManager {
    os_threadId threadId;
    c_bool      running;
};

C_STRUCT(spliced)
{
    int                     terminate;
    int                     systemHaltCode;/* 0 == ok, -1 == recoverable */
    c_ulong                 options;
    s_configuration         config;
    u_spliced               service;        /* splicedaemon service/participant */
    u_serviceManager        serviceManager;
    c_char                  *uri;
    serviceMonitor          sMonitor;
    c_bool                  isSingleProcess;
    os_int                  nrKnownServices;
    os_int                  nrTerminatedServices;
    struct HeartbeatManager heartbeatManager;
    sr_componentInfo        *knownServices;
    s_kernelManager         km;
    s_garbageCollector      gc;
    c_char*                 name;
    int                     nrApplications;
    sr_componentInfo        *applications;
};

/** This global variable is needed, since all resources of the
    splicedaemon are freed when the process exits. No arguments
    are given to the exit handler (in our case the function
    splicedFree())
*/

static spliced spl_daemon = NULL;

static void splicedFree(void);
/**************************************************************
 * Private functions
 **************************************************************/
static void
argumentsCheck(
    spliced this,
    int argc,
    char *argv[])
{
    this->name = os_strdup(argv[0]);
    if (this->name == NULL) {
        splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    if (argc == 1) {
        this->uri = os_strdup("");
        if (this->uri == NULL) {
            splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
        }
    } else if (argc == 2) {
        this->uri = os_strdup(argv[1]);
        if (this->uri == NULL) {
            splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
        }
    } else {
        printf("usage: %s [<URI>]\n",argv[0]);
        printf("\n");
        printf("<URI> file://<abs path to configuration> \n");
        exit(SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR);
    }
}
static void *
leaseRenewThread(
    void *arg)
{
    spliced this = (spliced)arg;
    os_time delay, before, after, accepted, lag;
    delay.tv_sec = this->config->leaseRenewalPeriod.seconds;
    delay.tv_nsec = this->config->leaseRenewalPeriod.nanoseconds;

    accepted.tv_sec = this->config->leasePeriod.seconds;
    accepted.tv_nsec = this->config->leasePeriod.nanoseconds;

    while (this && !this->terminate && (this->systemHaltCode == SPLICED_EXIT_CODE_OK)) {
        before = os_timeGet();
        u_serviceRenewLease(u_service(this->service), this->config->leasePeriod);
        os_nanoSleep(delay);
        after = os_timeGet();

        lag = os_timeSub(after, before);

        if(os_timeCompare(lag, accepted) == OS_MORE){
            OS_REPORT_4(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Splice-daemon failed to renew its lease within "
                "the configured lease expiry-time (%d.%09d). "
                "The lease renewal took %d.%09d s.",
                accepted.tv_sec, accepted.tv_nsec,
                lag.tv_sec, lag.tv_nsec);
        }
    }
    return NULL;
}

static void *
heartbeatManagerThread(
    void *arg)
{
    spliced _this = (spliced)arg;
    v_leaseManager lm;

    lm = u_splicedGetHeartbeatManager(_this->service, FALSE);
    if (lm) {
        OS_REPORT(OS_INFO,
                  "spliced::heartbeatManagerThread", 1,
                  "Heartbeat Manager for spliced started");
        v_leaseManagerMain(lm);
        OS_REPORT(OS_INFO,
                  "spliced::heartbeatManagerThread", 1,
                  "Heartbeat Manager for spliced stopped");
        c_free(lm);
    } else {
        OS_REPORT(OS_WARNING, "spliced::heartbeatManagerThread", 0,
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
    os_result ores;

    _this->heartbeatManager.running = FALSE;

    if (_this->config->heartbeatScheduling) {
        lm = u_splicedGetHeartbeatManager(_this->service, TRUE);
        if (lm) {
            ores = os_threadCreate(&_this->heartbeatManager.threadId, S_THREAD_HEARTBEAT_THREAD,
                                   _this->config->heartbeatScheduling, heartbeatManagerThread, _this);
            if (ores == os_resultSuccess) {
                _this->heartbeatManager.running = TRUE;
                ures = u_splicedStartHeartbeat(_this->service, _this->config->heartbeatExpiryTime,
                                               _this->config->heartbeatUpdateInterval, _this->config->heartbeatTransportPriority);
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
                                       _this->config->heartbeatUpdateInterval, _this->config->heartbeatTransportPriority);
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
        c_free(lm);
    }

    if (_this->heartbeatManager.running) {
       os_threadWaitExit(_this->heartbeatManager.threadId, NULL);
    }
}



os_char*
splicedGetDomainName(
    void)
{
    os_char* domainName = NULL;
    spliced _this = spl_daemon;

    if(_this)
    {
        domainName = _this->config->domainName;
    }
    return domainName;
}

void
splicedExit(
    const char *msg,
    int result)
{
    if (msg == NULL) {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED,
                    0, "Exiting reason: unspecified, Exit value: %d",result);
    } else {
        OS_REPORT_2(OS_INFO, OSRPT_CNTXT_SPLICED,
                    0, "Exiting reason: %s, Exit value: %d",msg,result);
    }

    exit(result);
}

static os_int32
termHandler(os_terminationType reason)
{
    if (spl_daemon != NULL) {
        spl_daemon->terminate = 1;
    } else {
        OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                    0, "Could not handle termination %d", reason);
    }
    return 0; /* the main thread will take care of termination */
}

static os_result
exitRequestHandler(
    os_callbackArg ignore)
{
    OS_UNUSED_ARG(ignore);
    /* The os_callbackArg can be ignored, because setting the below flag will
     * cause proper exit of the spliced. */
    if (spl_daemon != NULL) {
        spl_daemon->terminate = 1;
    } else {
        OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                    0, "Could not handle termination request");
    }
    return os_resultSuccess; /* the main thread will take care of termination */
}

static void
waitForServices(
    spliced this)
{
    os_int j;
    int cmp;
    os_int terminateCount;
    c_iter names;
    c_char *name;
    os_int32 status;
    os_result result;

    /* dds2164: decrease the poll delay to 100ms to allow for faster detection */
    os_time pollDelay = {0, 100000000};
    os_time curTime;
    os_time stopTime;

    curTime = os_timeGet();
    stopTime = os_timeAdd(curTime, this->config->serviceTerminatePeriod);
    do {
        terminateCount = 0;
        /* Wait for services to reach final state TERMINATED.
         */
        names = u_serviceManagerGetServices(this->serviceManager, STATE_TERMINATED);
        name = c_iterTakeFirst(names);
        while (name != NULL) {
            for (j = 0; j < this->nrKnownServices; j++) {
                if (this->knownServices[j] != NULL ) {
                    cmp = strcmp(name, this->knownServices[j]->name);
                    if (cmp == 0) {
                        terminateCount++;
                    }
                }
            }
            os_free(name);
            name = c_iterTakeFirst(names);
        }
        c_iterFree(names);

        /* Wait for services in final state DIED to be reaped.
         */
        names = u_serviceManagerGetServices(this->serviceManager, STATE_DIED);
        name = c_iterTakeFirst(names);
        while (name != NULL) {
            for (j = 0; j < this->nrKnownServices; j++) {
                cmp = strcmp(name, this->knownServices[j]->name);
                if (cmp == 0) {
                    if (this->knownServices[j]->procId != OS_INVALID_PID) {
                        result = os_procCheckStatus(this->knownServices[j]->procId, &status);
                        if (result == os_resultSuccess) {
                            this->knownServices[j]->procId = OS_INVALID_PID;
                            terminateCount++;
                        }
                    }
                }
            }
            os_free(name);
            name = c_iterTakeFirst(names);
        }
        c_iterFree(names);

        curTime = os_timeGet();

        if( (os_timeCompare(curTime, stopTime) == OS_LESS) &&
            (terminateCount < this->nrKnownServices)) {
            os_nanoSleep(pollDelay);
        }
        curTime = os_timeGet();
    } while ((os_timeCompare(curTime, stopTime) == OS_LESS) && (terminateCount < (this->nrKnownServices - this->nrTerminatedServices)));
}

static void
killServices(
    spliced this)
{
    os_int j;
    os_int32 status;
    os_result result;

    for (j = 0; j < this->nrKnownServices; j++) {
        /* service may have been removed already (see splicedRemoveServiceInfo),
         * so check for NULL here.
         */
        if(this->knownServices[j]){
            if (this->knownServices[j]->procId != OS_INVALID_PID) {
                result = os_procCheckStatus(this->knownServices[j]->procId, &status);
                if (result == os_resultSuccess) {
                    this->knownServices[j]->procId = OS_INVALID_PID;
                } else if (result == os_resultBusy){ /* Only kill if pid is (still) a child */
                    OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                                0, "Service '%s' did not terminate, sending kill",
                                this->knownServices[j]->name);
                    os_procDestroy(this->knownServices[j]->procId, OS_SIGKILL);
                }
            }
        }
    }
}

static void
splicedKnownServicesFree(
    spliced this)
{
    os_int i;

    assert(this != NULL);

    for (i = 0; i < this->nrKnownServices; i++) {
        sr_componentInfoFree(this->knownServices[i]);
        this->knownServices[i] = NULL;
    }
    if (this->knownServices != NULL) {
        os_free(this->knownServices);
    }
    this->knownServices = NULL;
    this->nrKnownServices = 0;
    this->nrTerminatedServices = 0;
}

static void
splicedApplicationsFree(
    spliced this)
{
    int i;

    assert(this != NULL);

    for (i = 0; i < this->nrApplications; i++) {
        sr_componentInfoFree(this->applications[i]);
        this->applications[i] = NULL;
    }
    if (this->applications != NULL) {
        os_free(this->applications);
    }
    this->applications = NULL;
    this->nrApplications = 0;
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

    tempString = strtok(info->args, " ");
    while (tempString)
    {
       argv[argc++] = os_strdup (tempString);
       tempString = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    mwa = os_malloc(sizeof(struct ut_entryPointWrapperArg));
    mwa->argc = argc;
    mwa->argv = argv;

    /* Initialise the library attributes */
    result = os_libraryAttrInit(&libraryAttr);
    if (result != os_resultSuccess)
    {
        OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                    "Problem initialising os_libraryAttr\n");
        return os_resultFail;
    }

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

        /* Note that the usage of os_libraryGetSymbol may result in a warning
         * of the form :
         *   "ISO C forbids assignment between function pointer and void *"
         * Unfortunately it seems this is a quirk of the standards as there is
         * no valid cast between pointer to function.
         */
        mwa->entryPoint = (ut_entryPointFunc)os_libraryGetSymbol (libraryHandle, entryPoint);

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
                OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
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
            OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                        "Command '%s' not found\n", entryPoint);
        }
    } else {
        result = os_resultFail;
        OS_REPORT_1(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
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
    spliced this)
{
    int retCode = SPLICED_EXIT_CODE_OK;
#ifdef INTEGRITY
    static Semaphore networkSvcStartSem;
    static Semaphore durabilitySvcStartSem;
    static Semaphore soapSvcStartSem;

    Error err;

    networkSvcStartSem = SemaphoreObjectNumber(12);
    durabilitySvcStartSem = SemaphoreObjectNumber(13);
    soapSvcStartSem = SemaphoreObjectNumber(14);

    err = ReleaseSemaphore(networkSvcStartSem);
    assert (err == Success);

    err = ReleaseSemaphore(durabilitySvcStartSem);
    assert (err == Success);

    err = ReleaseSemaphore(soapSvcStartSem);
    assert (err == Success);
#else
    os_int i;
    os_result createResult;
    sr_componentInfo info;
    c_char* args;
    int argc;
    char *vg_cmd = NULL;
    char *command = NULL;
    char *vg_args = NULL;

    assert(this != NULL);

    /*
    In case we haven't logged anything yet we call these methods now
    before we start any other domain services
    to (potentially) remove the previous versions of the files
    */
    os_free(os_reportGetInfoFileName());
    os_free(os_reportGetErrorFileName());

    for (i = 0; i < this->nrKnownServices; i++) {

        info = this->knownServices[i];

        if (this->isSingleProcess) {

            /* This is a 'SingleProcess' configuration so the service must be
             * deployed as a library.
             */

            createResult = deployLibrary (info);

            if (createResult == os_resultInvalid) {
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                            0, "Starting %s with <SingleProcess> not supported on this platform",
                            info->name);
                retCode = SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR;
                break;
            }
            else if (createResult != os_resultSuccess) {
                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0, "Could not start service %s as thread",
                            info->name);
            }
        } else {

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
                if (args)
                {
                    if (strlen(info->args) == 0)
                    {
                        if (!vg_cmd)
                        {
                            snprintf(args, argc, "\"%s\" \"%s\"",
                                     info->name, info->configuration);
                        }
                        else
                        {
                            snprintf(args, argc, "%s \"%s\" \"%s\" \"%s\"",
                                     vg_args, info->command, info->name, info->configuration);
                        }
                    }
                    else
                    {
                        if (!vg_cmd)
                        {
                            snprintf(args, argc, "\"%s\" \"%s\" \"%s\"",
                                     info->name, info->configuration, info->args);
                        }
                        else
                        {
                            snprintf(args, argc, "%s \"%s\" \"%s\" \"%s\" \"%s\"",
                                     vg_args, info->command, info->name, info->configuration, info->args);
                        }
                    }
                }

                createResult = os_procCreate(command,
                                             info->name, args,
                                             &info->procAttr, &info->procId);

                if (createResult == os_resultSuccess)
                {
                    os_sharedMemoryRegisterUserProcess(splicedGetDomainName(), info->procId);
                    OS_REPORT_2(OS_INFO, OSRPT_CNTXT_SPLICED,
                                0, "Started service %s with args %s",
                                info->name, args);
                }
                else
                {
                    OS_REPORT_2(OS_WARNING, OSRPT_CNTXT_SPLICED,
                                0, "Could not start service %s with args %s",
                                info->name, args);
                }

                if (args)
                {
                    os_free(args);
                }
                if (command)
                {
                    os_free(command);
                }
            }
            else
            {
                retCode = SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR;
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                            "Could not find file '%s' with read and execute permissions",
                            info->command);
                break;
            }
        }
    }
#endif
    return retCode;
}

static void
startApplications(
    spliced this)
{
    int i;
    os_result deployResult;

    assert(this != NULL);

    for (i = 0; i < this->nrApplications; i++) {

        deployResult = deployLibrary (this->applications[i]);

        if (deployResult == os_resultInvalid) {
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                            0, "Starting Application %s with <SingleProcess> not supported on this platform",
                            this->applications[i]->name);
        }
        else if (deployResult != os_resultSuccess) {
                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0, "Could not start application %s as thread",
                            this->applications[i]->name);
        }
    }
}

static void
retrieveBase(v_entity e, c_voidp arg)
{
    c_base *pbase = (c_base *) arg;

    *pbase = c_getBase(e);
}

static c_base
kernelGetBase(u_entity e)
{
    c_base base = NULL;
    u_result result = U_RESULT_UNDEFINED;

    result = u_entityAction(u_entity(e), retrieveBase, &base);
    if (result != U_RESULT_OK) {
        OS_REPORT_1(OS_ERROR, "kernelGetBase", result,
            "Entity action to retrieve database from kernel failed (%s)",
            u_resultImage(result));
        base = NULL;
    }

    return base;
}





/**************************************************************
 * configuration
 **************************************************************/
static void
getSingleProcessValue(
    spliced this,
    u_cfElement spliceCfg)
{
    assert(this != NULL);

    if (spliceCfg != NULL) {

        c_char * value;
        c_bool result;
        u_cfData node;
        c_iter iter;

        iter = u_cfElementXPath(spliceCfg, "SingleProcess/#text");
        if(iter)
        {
           node = u_cfData(c_iterTakeFirst(iter));
           if (node != NULL)
           {
              result = u_cfDataStringValue(node, &value);
              if (result) {
                 if (os_strcasecmp (value, "True" ) == 0)
                 {
                    this->isSingleProcess = 1;
                 }
              }
           }
           c_iterFree(iter);
        }
    }
}

static void
getKnownServices(
    spliced this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    os_int i;

    assert(this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "Service");
        this->nrKnownServices = c_iterLength(services);
        if (this->nrKnownServices > 0) {
            this->knownServices = (sr_componentInfo *)os_malloc((os_uint32)(this->nrKnownServices *
                                                            (int)sizeof(sr_componentInfo)));
            if (this->knownServices != NULL) {
                i = 0;
                s = c_iterTakeFirst(services);
                while (s != NULL) {
                    this->knownServices[i] = sr_componentInfoServiceNew(s, this->uri);
                    u_cfElementFree(s);
                    s = c_iterTakeFirst(services);
                    if (this->knownServices[i] != NULL) {
                        i++;
                    }
                }
            }
        }
        c_iterFree(services);
        this->nrKnownServices = i;
    }
}

static void
getApplications(
    spliced this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    int i;

    assert(this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "Application");
        this->nrApplications = c_iterLength(services);
        if (this->nrApplications > 0) {
            this->applications = (sr_componentInfo *)os_malloc((os_uint)(this->nrApplications *
                                                            (int)sizeof(sr_componentInfo)));
            if (this->applications != NULL) {
                i = 0;
                s = c_iterTakeFirst(services);
                while (s != NULL) {
                    this->applications[i] = sr_componentInfoApplicationNew(s, this->uri);
                    u_cfElementFree(s);
                    s = c_iterTakeFirst(services);
                    if (this->applications[i] != NULL) {
                        i++;
                    }
                }
            }
        }
        c_iterFree(services);
        this->nrApplications = i;
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
 * constructor/destructor
 **************************************************************/
static spliced
splicedNew()
{
    spliced this;

    this = spliced(os_malloc((os_uint32)C_SIZEOF(spliced)));

    if (this != NULL) {
        this->terminate = 0;
        this->systemHaltCode = SPLICED_EXIT_CODE_OK;
        this->config = s_configurationNew();
        this->service = NULL;
        this->serviceManager = NULL;
        this->options = 0;
        this->uri = NULL;
        this->sMonitor = NULL;
        this->isSingleProcess = 0;
        this->nrKnownServices = 0;
        this->nrTerminatedServices = 0;
        this->knownServices = NULL;
        this->km = NULL;
        this->gc = NULL;
        this->nrApplications = 0;
        this->applications = NULL;
        this->heartbeatManager.running = FALSE;
    }

    return this;
}

static void
splicedFree(void)
{
    spliced this = spl_daemon;
    os_time sleep = {1,0}; /* 1s */
    v_duration lease = {300, 0}; /* 5 minutes */

    if (this != NULL) {
        if (this->service != NULL) {
            u_serviceRenewLease(u_service(this->service), lease);

            if (!u_serviceChangeState(u_service(this->service),STATE_TERMINATING)) {
                OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                                   "Failed to go to TERMINATING state.\n");
            }
            stopHeartbeatManager(this);
        }
        serviceMonitorStop(this->sMonitor);
        serviceMonitorFree(this->sMonitor);
        this->sMonitor = NULL;
        waitForServices(this);
        /* All services have stopped, or timeout has occurred */

        /* signal internal threads to stop.
         */
        u_splicedPrepareTermination(this->service);
        /* Only perform the delay if the service terminate period has not been
         * configured as '0'.
         */
        if(this->config->serviceTerminatePeriod.tv_sec != 0)
        {
            /* Give internal threads some time to stop.
             */
            os_nanoSleep(sleep);
        }
        /* At this point no rock solid guarantee all threads are stopped.
         * Following destruction will cause threads too crash.
         */
        s_kernelManagerFree(this->km);
        this->km = NULL;
        s_garbageCollectorFree(this->gc);
        this->gc = NULL;
        if (this->service != NULL) {
            if (!u_serviceChangeState(u_service(this->service),STATE_TERMINATED)) {
                OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                                   "Failed to go to TERMINATING state.\n");
            }
        }
        if (this->serviceManager != NULL) {
            u_serviceManagerFree(this->serviceManager);
            this->serviceManager = NULL;
        }
        if (this->service != NULL) {
            u_splicedFree(this->service);
            this->service = NULL;
        }
        os_free(this->name);
        os_free(this->uri);

        /* Kill all not stopped services. */
        killServices(this);
        splicedKnownServicesFree(this);
        splicedApplicationsFree(this);
        u_userDetach();
        os_serviceStop();
        spl_daemon = NULL;
        if(this->config){
            s_configurationFree(this->config);
            this->config = NULL;
        }
        os_free(this);
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

u_serviceManager
splicedGetServiceManager(
    spliced spliceDaemon)
{
    u_serviceManager m;

    assert(spliceDaemon != NULL);

    m = spliceDaemon->serviceManager;

    return m;
}

void
splicedDoSystemHalt(
    int code)
{
    spliced _this;

    _this = spl_daemon;
    if(_this != NULL)
    {
        _this->systemHaltCode = code;
    }
}

os_result
splicedRemoveServiceInfo(
    spliced spliceDaemon,
    const c_char *name)
{
    os_int i;
    os_result result = os_resultFail;

    assert(spliceDaemon != NULL);

    i = 0;
    while (result == os_resultFail && (i < spliceDaemon->nrKnownServices)) {
        if (spliceDaemon->knownServices[i] != NULL && strcmp(spliceDaemon->knownServices[i]->name, name) == 0) {
            sr_componentInfoFree(spliceDaemon->knownServices[i]);
            spliceDaemon->knownServices[i] = NULL;
            result = os_resultSuccess;
        } else {
            i++;
        }
    }
    return result;
}

void
splicedRemoveKnownService(
    const c_char *name)
{
    spliced _this;
    os_result result = os_resultFail;

    _this = spl_daemon;
    if(_this != NULL)
    {
        result = splicedRemoveServiceInfo(_this,name);
        if(result != os_resultSuccess)
        {
            OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 0,
                   "Unable to remove service %s from the knownservices list",
                   name);
        } else {
            _this->nrTerminatedServices++;
        }
    }
}

sr_componentInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name)
{
    os_int i;
    sr_componentInfo ci;

    assert(spliceDaemon != NULL);

    ci = NULL;
    i = 0;
    while ((ci == NULL) && (i < spliceDaemon->nrKnownServices)) {
        if (spliceDaemon->knownServices[i] != NULL && strcmp(spliceDaemon->knownServices[i]->name, name) == 0) {
            ci = spliceDaemon->knownServices[i];
        } else {
            i++;
        }
    }

    return ci;
}

static void
childProcessDied(
    void)
{
    spliced _this = spl_daemon;
    os_int i;
    sr_componentInfo info;
    os_int32 result;
    os_result osResult;

    if(_this)
    {
        for(i = 0; i <_this->nrKnownServices; i++)
        {
            info = _this->knownServices[i];
            if (info != NULL) {
                osResult = os_procCheckStatus(info->procId, &result);
                if(osResult == os_resultSuccess)
                {
                    /* if result is 0, then we assume the regular service monitor takes over */
                    if(result != 0 && !_this->terminate)
                    {
                        serviceMonitorProcessDiedservice(splicedGetServiceManager(_this), info);
                    }
                }
            }
        }
    }
}

/**************************************************************
 * Main
 **************************************************************/
OPENSPLICE_ENTRYPOINT (ospl_spliced)
{
    spliced this;
    os_time delay;
    int retCode = SPLICED_EXIT_CODE_OK;
    os_threadId lrt;
    os_result result;
#ifdef CONF_PARSER_NOFILESYS
    char *env_uri;
#endif

    /* set the flag in the user layer that spliced is running in this process */
    u_splicedSetInProcess();

    u_userInitialise();
    result = os_procAtExit(ospl_splicedAtExit);
    if(result != os_resultSuccess)
    {
        OS_REPORT(
            OS_ERROR,
            "spliced_main",
            0,
            "Failed to register the spliced exit handler.");
        return -1;
    }
    this = splicedNew();
    if (this == NULL || this->config == NULL) {
        splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    spl_daemon = this;

#ifdef EVAL_V
    OS_REPORT(OS_INFO,"The OpenSplice domain service", 0,
	      "++++++++++++++++++++++++++++++++" OS_REPORT_NL
	      "++ spliced EVALUATION VERSION ++" OS_REPORT_NL
	      "++++++++++++++++++++++++++++++++\n");
#endif

#ifdef CONF_PARSER_NOFILESYS
    env_uri = os_getenv("OSPL_URI");
    argumentsCheck(this, argc, argv);
    if (ospl_xml_data_size != 0) {
        this->name = os_strdup("spliced");
        if (this->name == NULL) {
            splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
        }
        if (*this->uri == '\0') {
            os_free (this->uri);
            this->uri = os_strdup ("file:///ospl.xml");
            if (this->uri == NULL) {
                splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
            }
        }
    }

    if ((argc > 1) || env_uri) {
       FILE *cfgFile;
       size_t count = 0;
       char *buffer;

       if (argc == 1) {
          os_free (this->uri);
          this->uri = os_strdup (env_uri);
          if (this->uri == NULL) {
             splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
          }
       }

       if ( this->uri
            && (strncmp(this->uri, URI_FILESCHEMA, strlen(URI_FILESCHEMA)) == 0)) {
          cfgFile = fopen(&this->uri[7], "r");
          if ( cfgFile != NULL ) {
             fseek(cfgFile, 0, SEEK_END);
             count = ftell(cfgFile);
             fseek(cfgFile, 0, SEEK_SET);
             buffer = os_malloc(count+1);
             if ( buffer != NULL ) {
                fread( buffer, 1, count, cfgFile);
                buffer[count]='\0';
                ospl_xml_data_ptr = buffer;
                ospl_xml_data_ptr_size = count;
             } else {
                splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
             }
          } else {
             splicedExit("Failed to open config file.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
          }
       } else {
          if (ospl_xml_data_ptr == NULL) {
             splicedExit("Invalid URI for config file.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
          }
       }
    }
#else
    argumentsCheck(this, argc, argv);
#endif

#ifndef INTEGRITY
    os_procSetTerminationHandler(termHandler);
#endif

#ifndef PIKEOS_POSIX
    /*os_signalHandlerSetHandler(OS_SIGCHLD, childProcessDied); see OSPL-1509 why this is disabled */
#endif

    this->service = u_splicedNew(this->uri);
    if (this->service == NULL) {
        splicedExit("Failed to attach to kernel.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    readConfiguration(this);
#ifndef INTEGRITY
    if(!(this->isSingleProcess))
    {
        os_procSetTerminationHandler(termHandler);

        /* Below 2 calls will unset the handlers set by the user-layer */
        os_signalHandlerSetExitRequestCallback(exitRequestHandler);
        os_signalHandlerSetExceptionCallback((os_signalHandlerExceptionCallback)0);
    }
#endif
    dds_builtInTypes__register_types (kernelGetBase(u_entity(this->service)));

    if (!u_serviceChangeState(u_service(this->service), STATE_INITIALISING)) {
        splicedExit("Failed to set service state to STATE_INITIALISING.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    this->serviceManager = u_serviceManagerNew(u_participant(this->service));

    this->km = s_kernelManagerNew(this);
    if (this->km == NULL) {
        splicedExit("Failed to create kernel manager.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    if (!s_kernelManagerWaitForActive(this->km)) {
        splicedExit("Failed to start kernel manager.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    this->gc = s_garbageCollectorNew(this);
    if (this->gc == NULL) {
        splicedExit("Failed to create garbage collector.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    if (!s_garbageCollectorWaitForActive(this->gc)) {
        splicedExit("Failed to start garbage collector.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    this->sMonitor = serviceMonitorNew(this);
    if (this->sMonitor == NULL) {
        splicedExit("Failed to create service monitor.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    if (!startHeartbeatManager(this)) {
        splicedExit("Failed to start heartbeats.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    if (!u_serviceChangeState(u_service(this->service), STATE_OPERATIONAL)) {
        splicedExit("Failed to set service state to STATE_OPERATIONAL.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    /* Start services */
    retCode = startServices(this);
    if(retCode == SPLICED_EXIT_CODE_OK)
    {
        /* Start applications specified in XML on a best effort basis. Note
         * this will only succeed if this is a SingleProcess configuration,
         * and leave warning messages otherwise */
        startApplications(this);

        delay.tv_sec = this->config->leaseRenewalPeriod.seconds;
        delay.tv_nsec = this->config->leaseRenewalPeriod.nanoseconds;

        /* start the spliced watchDog thread*/
        result = os_threadCreate(&lrt, S_THREAD_LEASE_RENEW_THREAD, &this->config->leaseRenewScheduling, leaseRenewThread, this);
        if (result != os_resultSuccess) {
            splicedExit("Failed to start lease renew thread.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
        }

        while (!this->terminate && (this->systemHaltCode == SPLICED_EXIT_CODE_OK)) {
            os_nanoSleep(delay);
        }
        os_threadWaitExit(lrt, NULL);
    }

    if(this->systemHaltCode != SPLICED_EXIT_CODE_OK && retCode == SPLICED_EXIT_CODE_OK)
    {
        retCode = SPLICED_EXIT_CODE_RECOVERABLE_ERROR;
    }
    return retCode;
}

c_bool
deleteContainedEntitiesForApplParticipants(
    u_participant participant,
    c_voidp arg)
{
    u_result result;
    u_kind entityKind;

    OS_UNUSED_ARG(arg);

    entityKind = u_entityKind(u_entity(participant));
    if(entityKind != U_SERVICE)
    {
        result = u_participantDeleteContainedEntities(participant);
        if(result != U_RESULT_OK)
        {
            OS_REPORT_2(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                    "An error occuring during exit handling. Unable to "
                    "delete contained entities of participant '0x%x'. Result "
                    "code was '%d'.",
                    participant,
                    result);
        } /* when this fails, continue with the next participant */
    }
    return TRUE;
}

void
ospl_splicedAtExit(
    void)
{
    spliced _this;

    _this = spl_daemon;
    if(_this)
    {
        u_result result;
        u_domain domain;

        _this->terminate = 1;
        if(!_this->isSingleProcess )
        {
            if(_this->service)
            {
                domain = u_participantDomain(u_participant(_this->service));
                if(domain)
                {
                    result = u_domainWalkParticipants(domain, deleteContainedEntitiesForApplParticipants, NULL);
                    if(result != U_RESULT_OK)
                    {
                        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                                    "An error occuring during exit handling. Unable to "
                                    "complete a walk over all known participants. "
                                    "Result code was '%d'.",
                                    result);
                    }
                }
                else
                {
                    OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                              "An error occuring during exit handling. Unable to "
                              "delete contained entities of application participants. "
                              "No domain was found.");
                }
            }
            else
            {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                          "An error occuring during exit handling. Unable to determine "
                          "the presence of application participants. "
                          "The splice daemon service object was NULL.");
            }
            /* Terminate all OpenSplice DDS activities. One of the steps here is to
             * modify the v_kernel 'splicedRunning' boolean so that application
             * threads will be denied access.
             */
            splicedFree();
            /* finally call u_userExit */
            u_userExit();
        }
#ifdef CONF_PARSER_NOFILESYS
        if ( ospl_xml_data != ospl_xml_data_ptr ) {
           os_free( ospl_xml_data_ptr );
        }
#endif
    }
}


