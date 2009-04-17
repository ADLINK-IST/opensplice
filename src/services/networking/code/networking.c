#include <assert.h>
#include "c_base.h"
#include "u_user.h"
#include "nw_runnable.h"
#include "nw_controller.h"
#include "nw_configuration.h"
#include "nw_report.h"
#include "nw__confidence.h"

#ifdef INTEGRITY
#include <include/os_getRSObjects.h>
#endif

static void
nw_splicedaemonListener(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData)
{
    c_bool *terminate = (c_bool *)usrData;

    switch (spliceDaemonState) {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
        *terminate = TRUE;
    break;
    default:
        NW_CONFIDENCE(FALSE);
    break;
    }
}

static void
controller_onfatal(c_voidp usrData)
{
    c_bool *terminate = (c_bool *)usrData;

    NW_REPORT_ERROR("controller_termination", "Terminated due to a Fatal Error");
    *terminate = TRUE;
}


static void
nw_retrieveLeaseSettings(
    v_duration *leaseExpiryTime,
    os_time *sleepTime)
{
    c_float leaseSec;
    c_float sleepSec;

    /* Get lease-period settings from config */
    leaseSec = nw_configurationGetDomainLeaseExpiryTime();
    sleepSec = nw_configurationGetDomainLeaseUpdateTime();

    sleepTime->tv_sec = (os_int32)sleepSec;
    sleepTime->tv_nsec = (os_int32)((sleepSec - (float)sleepTime->tv_sec) * 1000000000.0F);

    leaseExpiryTime->seconds = (os_int32)leaseSec;
    leaseExpiryTime->nanoseconds = (os_int32)((leaseSec - (float)leaseExpiryTime->seconds) * 1000000000.0F);

}
#undef NW_DEFAULT_SLEEPSEC
#undef NW_MIN_SLEEPSEC


static void
nw_serviceMain(
    const char *serviceName,
    const char *URI)
{
    u_service service;
    u_serviceManager serviceManager;
    nw_controller controller;
    os_time sleepTime;
    c_bool terminate = FALSE;
    v_duration leasePeriod;

     /* Create networking service with kernel */
   service = u_serviceNew(URI, NW_ATTACH_TIMEOUT, serviceName, NULL,
                          U_SERVICE_NETWORKING, NULL);
    /* Initialize configuration */
   nw_configurationInitialize(service, serviceName, URI);

    /* Ask service manager for splicedaemon state */
    serviceManager = u_serviceManagerNew(u_participant(service));

    /* Create the controller which starts the updating */
    /* and calls the listener on a fatal error */
    controller = nw_controllerNew(service,controller_onfatal,&terminate);

    if (controller) {
        /* Start the actual engine */
        NW_REPORT_INFO(1, "Networking started");
        NW_TRACE(Mainloop, 1, "Networking started");
        nw_controllerStart(controller);
        /* Change state for spliced */
        u_serviceChangeState(service, STATE_INITIALISING);
        u_serviceChangeState(service, STATE_OPERATIONAL);
        /* Get sleeptime from configuration */
        nw_retrieveLeaseSettings(&leasePeriod, &sleepTime);
        /*sleepTime.tv_sec = 1; */

        /* Loop until termination is requested */
        u_serviceWatchSpliceDaemon(service, nw_splicedaemonListener,
                                   &terminate);
        while (!(int)terminate) {
            /* Assert my liveliness and the Splicedaemon's liveliness*/
            u_participantRenewLease(u_participant(service), leasePeriod);
            /* Check if anybody is still remotely interested */
            nw_controllerUpdateHeartbeats(controller);
            /* Wait before renewing again */
            os_nanoSleep(sleepTime);
/* QAC EXPECT 2467; Control variable, terminate, not modified inside loop. That is correct, it is modified by another thread */
        }
        leasePeriod.seconds = 20;
        leasePeriod.nanoseconds = 0;
        u_participantRenewLease(u_participant(service), leasePeriod);
        u_serviceChangeState(service, STATE_TERMINATING);
        nw_controllerStop(controller);
        nw_controllerFree(controller);
        NW_REPORT_INFO(1, "Networking stopped");
        NW_TRACE(Mainloop, 1, "Networking stopped");
    }
    nw_configurationFinalize();

    /* Clean up */
    u_serviceChangeState(service, STATE_TERMINATED);
    u_serviceManagerFree(serviceManager);
    u_serviceFree(service);
}

#undef NW_SERVICE_NAME
#define NW_ATTACH_TIMEOUT (30)

#ifdef OSPL_ENV_SHMT
int
ospl_main(
    int argc,
    char *argv[])
#else
int
main(
    int argc,
    char *argv[])
#endif
{
    u_result retVal;
    char *name;
    char *config;
#ifdef INTEGRITY
    Error err;
    Semaphore networkSvcStartSem = SemaphoreObjectNumber(13);
    name = "networking";
    config = "file:///ospl.xml";

    err = WaitForSemaphore(networkSvcStartSem);
    assert ( err == Success );

    argc = 3;
#else
    os_time delay = {1, 0};

#ifdef NW_DEBUGGING
    /* Stop and wait for debugger */
    if (argc > 3) {
        if (strcmp(argv[3], "wait") == 0) {
            int p = 0;
            while (p==0) {
                os_time delay = {1, 0};
                printf("Networking sleeping, waiting for debugger\n");
                os_nanoSleep(delay);
            }
        }
        argc--;
    }

    if (argc > 3) {
        int i;
        printf("Ignoring superfluous parameters: ");
        for (i=3; i<argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }
#endif

#endif
    /* First check command line arguments */
    if (argc == 3)
    {
#ifndef INTEGRITY
        name = argv[1];
        config = argv[2];
#endif
        /* Initialize user API */
        retVal = u_userInitialise();
        if (retVal == U_RESULT_OK) {
                /* The actual service actions */
            nw_serviceMain(name, config);

            u_userDetach();
        } else {
           NW_REPORT_ERROR("networking main loop",
                           "Error attaching to kernel, bailing out");
        }
    } else {
       NW_REPORT_ERROR("networking main loop",
                       "Usage: networking name <configuration-URI>");
    }

    return 0;

}

