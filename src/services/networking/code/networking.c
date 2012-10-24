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
#include "c_base.h"
#include "u_user.h"
#include "nw_runnable.h"
#include "nw_controller.h"
#include "nw_configuration.h"
#include "nw_report.h"
#include "nw__confidence.h"

#ifdef INTEGRITY
#include "include/os_getRSObjects.h"
#endif

#include "os.h"


static nw_controller controller = NULL;
static u_service service = NULL;
c_bool f_exit = FALSE;

typedef struct {
  os_mutex mtx;
  os_cond cv;
  c_bool terminate;
} nw_termination;

static void
nw_splicedaemonListener(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData)
{
    nw_termination *terminate = (nw_termination *)usrData;

    switch (spliceDaemonState) {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
        os_mutexLock( &terminate->mtx );
        terminate->terminate = TRUE;
	os_condBroadcast( &terminate->cv );
	os_mutexUnlock( &terminate->mtx );

    break;
    default:
        NW_CONFIDENCE(FALSE);
    break;
    }
}

static void
controller_onfatal(c_voidp usrData)
{
    c_bool *fatal = (c_bool *)usrData;

    NW_REPORT_ERROR("controller_termination", "Terminated due to a Fatal Error");
    *fatal = TRUE;
}


static void
on_exit_handler(void)
{
    v_duration leasePeriod;
	if (controller) {
		NW_REPORT_ERROR("controller_termination", "Terminated due to a signal");

		NW_TRACE(Mainloop, 1, "Networking exit handler");
		nw_controllerStop(controller);
		leasePeriod.seconds = -1;
		leasePeriod.nanoseconds = 0;
		u_serviceRenewLease(service, leasePeriod);
		f_exit = TRUE;
	}

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
    u_serviceManager serviceManager;
    os_time sleepTime;
    os_result waitResult;
    nw_termination terminate;
    os_mutexAttr termMtxAttr;
    os_condAttr termCvAttr;
	c_bool fatal = FALSE;
    v_duration leasePeriod;
    terminate.terminate = FALSE;

    os_mutexAttrInit( & termMtxAttr );
    os_condAttrInit( & termCvAttr );
    termMtxAttr.scopeAttr = OS_SCOPE_PRIVATE;
    termCvAttr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit( &terminate.mtx, &termMtxAttr );
    os_condInit( &terminate.cv, &terminate.mtx, &termCvAttr );

     /* Create networking service with kernel */
   service = u_serviceNew(URI, NW_ATTACH_TIMEOUT, serviceName, NULL,
                          U_SERVICE_NETWORKING, NULL);
    /* Initialize configuration */
   nw_configurationInitialize(service, serviceName, URI);

    /* Ask service manager for splicedaemon state */
    serviceManager = u_serviceManagerNew(u_participant(service));

    /* Create the controller which starts the updating */
    /* and calls the listener on a fatal error */
    controller = nw_controllerNew(service,controller_onfatal,&fatal);

    if (controller) {
		os_procAtExit(on_exit_handler);
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
	os_mutexLock( &terminate.mtx );
        while ((!(int)terminate.terminate) && (!(int)fatal) && (!(int)f_exit)) {
            /* Assert my liveliness and the Splicedaemon's liveliness*/
            u_serviceRenewLease(service, leasePeriod);
            /* Check if anybody is still remotely interested */
            nw_controllerUpdateHeartbeats(controller);
            /* Wait before renewing again */
	        waitResult = os_condTimedWait( &terminate.cv, &terminate.mtx, &sleepTime );
	        if (waitResult == os_resultFail)
            {
                OS_REPORT(OS_CRITICAL, "nw_serviceMain", 0,
                          "os_condTimedWait failed - thread will terminate");
                fatal = TRUE;
            }
/* QAC EXPECT 2467; Control variable, terminate, not modified inside loop. That is correct, it is modified by another thread */
        }
	os_mutexUnlock( &terminate.mtx );
		/* keep process here waiting for the exit processing */
		while ((int)f_exit){os_nanoSleep(sleepTime);}

		if (!(int)fatal ) {
			leasePeriod.seconds = 20;
			leasePeriod.nanoseconds = 0;
			u_serviceRenewLease(service, leasePeriod);
	        u_serviceChangeState(service, STATE_TERMINATING);

	        nw_controllerStop(controller);
	        nw_controllerFree(controller);
			controller = NULL;
	        NW_REPORT_INFO(1, "Networking stopped");
	        NW_TRACE(Mainloop, 1, "Networking stopped");
		}
    }
	if (!(int)fatal ) {
	    nw_configurationFinalize();

	    /* Clean up */
	    u_serviceChangeState(service, STATE_TERMINATED);
		u_serviceManagerFree(serviceManager);

		u_serviceFree(service);
	}
}

#undef NW_SERVICE_NAME
#define NW_ATTACH_TIMEOUT (30)

int
ospl_main(
    int argc,
    char *argv[])
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
/*
struct updateStatistics {
	nw_networkingStatisticsCallback callback;
    c_voidp userData;
};

static void
nw_networkingUpdateStatisticsCallback(
    v_entity entity,
    c_voidp args)
{
    struct updateStatistics* update;


    if(entity->statistics){
        update = (struct updateStatistics*)args;
        update->callback(v_networkingStatistics(entity->statistics), update->userData);
    }
    return;
}

void
nw_networkingUpdateStatistics(
	nw_networking networking,
	nw_networkingStatisticsCallback callback,
    c_voidp args)
{
    struct updateStatistics update;

    update.callback = callback;
    update.userData = args;

    u_entityAction(u_entity(networking->service), v_networkingUpdateStatisticsCallback, &update);
}*/


#if ( !(defined(OSPL_ENV_SHMT)  || defined(NW_SECURITY)))
OPENSPLICE_MAIN (ospl_networking)
{
    return ospl_main(argc, argv);
}
#endif


