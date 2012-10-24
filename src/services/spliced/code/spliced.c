/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

#include "c_typebase.h"
#include "c_stringSupport.h"

#include "spliced.h"
#include "s_misc.h"
#include "s_configuration.h"
#include "report.h"
#include "serviceMonitor.h"
#include "s_kernelManager.h"
#include "s_gc.h"
#include "sr_serviceInfo.h"
#include "u_scheduler.h"
#include "dds_builtInTypes_register.h"

#ifdef OSPL_ENV_SHMT
#include <dlfcn.h>
#include <link.h>

#define PURE_MAIN_SYMBOL "ospl_main"
#endif

static void
splicedExit(
    const char *msg,
    int result);

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
    int                     nrKnownServices;
    sr_serviceInfo          *knownServices;
    s_kernelManager         km;
    s_garbageCollector      gc;
    c_char*                 name;
#ifdef OSPL_ENV_SHMT
    int                     nrTestServices;
    sr_serviceInfo          *testServices;
#endif
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
    os_time delay;
    delay.tv_sec = this->config->leaseRenewalPeriod.seconds;
    delay.tv_nsec = this->config->leaseRenewalPeriod.nanoseconds;
    while (!this->terminate && (this->systemHaltCode == SPLICED_EXIT_CODE_OK)) {
        u_serviceRenewLease(this->service, this->config->leasePeriod);
        os_nanoSleep(delay);
    }
    return NULL;
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
    splicedFree();
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

static void
waitForServices(
    spliced this)
{
    int j;
    int cmp;
    int terminateCount;
    c_iter names;
    c_char *name;

    /* dds2164: decrease the poll delay to 100ms to allow for faster detection */
    os_time pollDelay = {0, 100000000};
    os_time curTime;
    os_time stopTime;

    curTime = os_timeGet();
    stopTime = os_timeAdd(curTime, this->config->serviceTerminatePeriod);

    do {
        terminateCount = 0;
        names = u_serviceManagerGetServices(this->serviceManager, STATE_TERMINATED);

        name = c_iterTakeFirst(names);

        while (name != NULL) {
            for (j = 0; j < this->nrKnownServices; j++) {
                cmp = strcmp(name, this->knownServices[j]->name);
                if (cmp == 0) {
                    terminateCount++;
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
    } while ((os_timeCompare(curTime, stopTime) == OS_LESS) && (terminateCount < this->nrKnownServices));
}

static void
splicedKnownServicesFree(
    spliced this)
{
    int i;

    assert(this != NULL);

    for (i = 0; i < this->nrKnownServices; i++) {
        sr_serviceInfoFree(this->knownServices[i]);
        this->knownServices[i] = NULL;
    }
    if (this->knownServices != NULL) {
        os_free(this->knownServices);
    }
    this->knownServices = NULL;
    this->nrKnownServices = 0;
}

#ifdef OSPL_ENV_SHMT
static void
splicedTestServicesFree(
    spliced this)
{
    int i;

    assert(this != NULL);

    for (i = 0; i < this->nrTestServices; i++) {
        sr_serviceInfoFree(this->testServices[i]);
        this->testServices[i] = NULL;
    }
    if (this->testServices != NULL) {
        os_free(this->testServices);
    }
    this->testServices = NULL;
    this->nrTestServices = 0;
}
#else
#define splicedTestServicesFree(this)
#endif

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
    int i;
    os_result procCreateResult;
    sr_serviceInfo info;
    c_char* args;
    int argc;
    char *vg_cmd = NULL;
    char *command = NULL;
    char *vg_args = NULL;

    assert(this != NULL);

    for (i = 0; i < this->nrKnownServices; i++) {
        /* Now compose stuff */
        info = this->knownServices[i];
#ifndef VXWORKS_RTP
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

           procCreateResult = os_procCreate(command,
                                            info->name, args,
                                            &info->procAttr, &info->procId);
            if (procCreateResult == os_resultSuccess)
            {
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
        }
    }
#endif
    return retCode;
}

#ifdef OSPL_ENV_SHMT

#define ARGV_LEN 4

typedef int (*mainfunc)(int,char **);

struct mainWrapperArg {
    mainfunc mainSymbol;
    int argc;
    char **argv;
};

static void *
mainWrapper(void *arg)
{
    struct mainWrapperArg *mwa = (struct mainWrapperArg *)arg;
    int result;

    result = mwa->mainSymbol(mwa->argc,mwa->argv);

    os_free(mwa->argv);
    os_free(mwa);

    return (void *)result;
}
static void
startTestServices(
    spliced this)
{
    char **argv;
    int argc = 1;
    int i;

    void *execHandle;
    struct mainWrapperArg *mwa;
    os_result rv = os_resultSuccess;
    os_threadAttr attr;
    os_threadId id;
    os_time delay = {5, 0};

    assert(this != NULL);

    for (i = 0; i < this->nrTestServices; i++) {

        argv = os_malloc(ARGV_LEN*sizeof(char *));
        argv[0] = this->testServices[i]->name;
        argv[1] = this->testServices[i]->name;
        argv[2] = this->testServices[i]->configuration;
        argv[3] = NULL;
        argc = 3;

        mwa = os_malloc(sizeof(struct mainWrapperArg));
        mwa->argc = argc;
        mwa->argv = argv;
        execHandle = dlopen(this->testServices[i]->command, RTLD_LAZY);
        if (execHandle == NULL) {
            rv = os_resultFail;
            OS_REPORT_2(OS_WARNING,OSRPT_CNTXT_SPLICED,0,
                "Problem starting TestService '%s':\n              %s", argv[0], dlerror());
        } else {
            /* now find the main symbol! */
            mwa->mainSymbol = (int (*)(int,char**))dlsym(execHandle, PURE_MAIN_SYMBOL);
            if (mwa->mainSymbol == NULL) {
                rv = os_resultFail;
                OS_REPORT_2(OS_WARNING,OSRPT_CNTXT_SPLICED,0,
                    "Entry-point not found for TestService '%s':\n              %s", argv[0], dlerror());
            } else {
                /* now start the thread iso the executable */
                os_threadAttrInit(&attr);
                attr.stackSize = 10*1024*1024;
                rv = os_threadCreate(&id, this->testServices[i]->name, &attr, mainWrapper, mwa);
                this->testServices[i]->procId = id;
                OS_REPORT_1(OS_INFO,OSRPT_CNTXT_SPLICED,0, "Starting TestService: %s", argv[0]);
            }
        }
        os_nanoSleep(delay);
    }
}
#else
#define startTestServices(this)
#endif

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

    u_entityAction(u_entity(e), retrieveBase, &base);

    return base;
}

/**************************************************************
 * configuration
 **************************************************************/
static void
getKnownServices(
    spliced this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    int i;

    assert(this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "Service");
        this->nrKnownServices = c_iterLength(services);
        if (this->nrKnownServices > 0) {
            this->knownServices = (sr_serviceInfo *)os_malloc((os_uint32)(this->nrKnownServices *
                                                            (int)sizeof(sr_serviceInfo)));
            if (this->knownServices != NULL) {
                i = 0;
                s = c_iterTakeFirst(services);
                while (s != NULL) {
                    this->knownServices[i] = sr_serviceInfoNew(s, this->uri);
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

#ifdef OSPL_ENV_SHMT
static void
getTestServices(
    spliced this,
    u_cfElement spliceCfg)
{
    c_iter services;
    u_cfElement s;
    int i;

    assert(this != NULL);

    i = 0;
    if (spliceCfg != NULL) {
        services = u_cfElementXPath(spliceCfg, "TestService");
        this->nrTestServices = c_iterLength(services);
        if (this->nrTestServices > 0) {
            this->testServices = (sr_serviceInfo *)os_malloc((os_uint)(this->nrTestServices *
                                                            (int)sizeof(sr_serviceInfo)));
            if (this->testServices != NULL) {
                i = 0;
                s = c_iterTakeFirst(services);
                while (s != NULL) {
                    this->testServices[i] = sr_serviceInfoNew(s, this->uri);
                    u_cfElementFree(s);
                    s = c_iterTakeFirst(services);
                    if (this->testServices[i] != NULL) {
                        i++;
                    }
                }
            }
        }
        c_iterFree(services);
        this->nrTestServices = i;
    }
}
#else
#define getTestServices(this,spliceCfg)
#endif

static void
readConfiguration(
    spliced _this)
{
    u_cfElement cfg;
    u_cfElement dc;
    c_iter      domains;

    if (_this != NULL) {
        s_configurationRead(_this->config, _this);
        if (_this->service != NULL) {
            cfg = u_participantGetConfiguration(u_participant(_this->service));
            if (cfg != NULL) {
                domains = u_cfElementXPath(cfg, "Domain");
                dc = c_iterTakeFirst(domains);
                if (dc != NULL) {
                    getKnownServices(_this, dc);
                    getTestServices(_this, dc);
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
        } else {
            OS_REPORT(OS_ERROR,"spliced::readConfiguration",0,
                      "Spliced user proxy not initialised.");
            assert(0);
        }
    } else {
        OS_REPORT(OS_ERROR,"spliced::readConfiguration",0,
                  "Spliced not specified.");
        assert(0);
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
        this->nrKnownServices = 0;
        this->knownServices = NULL;
        this->km = NULL;
        this->gc = NULL;
#ifdef OSPL_ENV_SHMT
        this->nrTestServices = 0;
        this->testServices = NULL;
#endif
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
            u_serviceRenewLease(this->service, lease);

            if (!u_serviceChangeState(u_service(this->service),STATE_TERMINATING)) {
                OS_REPORT(OS_ERROR,OSRPT_CNTXT_SPLICED,0,
                                   "Failed to go to TERMINATING state.\n");
            }
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

        splicedKnownServicesFree(this);
        splicedTestServicesFree(this);
        u_userDetach();
        os_serviceStop();
        OS_REPORT(OS_INFO,OSRPT_CNTXT_SPLICED,0,
                  "==============================================\n"
                  "              == The service has successfully terminated. ==\n"
                  "              ==============================================");

        spl_daemon = NULL;
        s_configurationFree(this->config);
        this->config = NULL;
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
    spliced spliceDaemon,
    int code)
{
    assert(spliceDaemon != NULL);

    spliceDaemon->systemHaltCode = code;
}

sr_serviceInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name)
{
    int i;
    sr_serviceInfo si;

    assert(spliceDaemon != NULL);

    si = NULL;
    i = 0;
    while ((si == NULL) && (i < spliceDaemon->nrKnownServices)) {
        if (strcmp(spliceDaemon->knownServices[i]->name, name) == 0) {
            si = spliceDaemon->knownServices[i];
        } else {
            i++;
        }
    }

    return si;
}

/**************************************************************
 * Main
 **************************************************************/
OPENSPLICE_MAIN (ospl_spliced)
{
    spliced this;
    u_result r;
    os_time delay;
    os_result osr;
    int retCode = SPLICED_EXIT_CODE_OK;
    os_threadId lrt;

    u_userInitialise();

    this = splicedNew();
    if (this == NULL) {
        splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    spl_daemon = this;

#ifdef INTEGRITY
    this->name = os_strdup("spliced");
    if (this->name == NULL) {
        splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
    this->uri = os_strdup ("file:///ospl.xml");
    if (this->uri == NULL) {
        splicedExit("Failed to allocate memory.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }
#else
    argumentsCheck(this, argc, argv);
    os_procSetTerminationHandler(termHandler);
#endif


    this->service = u_splicedNew(this->uri);
    if (this->service == NULL) {
        splicedExit("Failed to attach to kernel.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    readConfiguration(this);

    dds_builtInTypes__register_types (kernelGetBase(u_entity(this->service)));

    u_serviceChangeState(u_service(this->service), STATE_INITIALISING);
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
    r = u_splicedStartHeartbeat(this->service, this->config->leasePeriod,
            this->config->leaseRenewalPeriod);
    if (r != U_RESULT_OK) {
        splicedExit("Failed to start heartbeats.", SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
    }

    u_serviceChangeState(u_service(this->service), STATE_OPERATIONAL);

    /* Start services */
    retCode = startServices(this);
    if(retCode == SPLICED_EXIT_CODE_OK)
    {
        startTestServices(this);

        delay.tv_sec = this->config->leaseRenewalPeriod.seconds;
        delay.tv_nsec = this->config->leaseRenewalPeriod.nanoseconds;

        osr = os_threadCreate(&lrt, S_THREAD_LEASE_RENEW_THREAD, &this->config->leaseRenewScheduling, leaseRenewThread, this);
        if (osr != os_resultSuccess) {
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
    u_splicedStopHeartbeat(this->service);
    splicedFree();
    return retCode;
}


