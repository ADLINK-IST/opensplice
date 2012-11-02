#include "kernelModule.h"
#include "u_types.h"
#include "u_user.h"
#include "in_result.h"
#include "in_report.h"
#include "in__config.h"
#include "in_controller.h"
#include "in_connectivityAdmin.h"
#include "os.h"

static void
in_splicedaemonListener(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData);

static void
in_serviceMain(
    const os_char* serviceName,
    const os_char* uri);

static void
in_retrieveLeaseSettings(
    v_duration *leaseExpiryTime,
    os_time *sleepTime);

OPENSPLICE_MAIN (ospl_ddsi)
{
    u_result retVal;
    /* First check command line arguments */
    if (argc == 3)
    {
        /* Initialize user API */
        retVal = u_userInitialise();
        if (retVal == U_RESULT_OK)
        {
            /* The actual service actions */
            in_serviceMain(argv[1], argv[2]);
            /* detach from the user API */
            u_userDetach();
        } else
        {

           IN_REPORT_ERROR(
               "DDSI networking main loop",
               "Error attaching to kernel, bailing out");
        }
    } else
    {
       IN_REPORT_ERROR(
           "DDSI networking main loop",
           "Usage: ddsi name <configuration-URI>");
    }
    assert(in_objectValidate(0));
    printf("%s is gone...\n", argv[1]);
    return 0;

}

void
in_serviceMain(
    const os_char* serviceName,
    const os_char* uri)
{
    u_service service;
    in_config config;
    in_result result;
    u_serviceManager serviceManager;
    in_controller controller;
    v_duration leasePeriod;
    os_time sleepTime;
    os_boolean terminate = OS_FALSE;
    in_connectivityAdmin admin;

    assert(serviceName);
    assert(uri);

    /* Create networking service with kernel */
    service = u_serviceNew(
        uri,
        IN_ATTACH_TIMEOUT,
        serviceName,
        NULL,
        U_SERVICE_NETWORKING,
        NULL);
    assert(service);
    /* Initialize configuration */
    config = in_configGetInstance();
    result = in_configConvertDomTree(config, uri, service);
    if(result == IN_RESULT_OK)
    {
        /* Ask service manager for splicedaemon state */
        serviceManager = u_serviceManagerNew(u_participant(service));

        admin = in_connectivityAdminGetInstance();
        /* Create the controller which starts the updating */
        controller = in_controllerNew(service);
        if (controller)
        {
            /* Start the actual engine */
            IN_REPORT_INFO(1, "DDSI networking started");
            IN_TRACE(Mainloop, 1, "DDSI networking started");
            in_controllerStart(controller);
            /* Change state for spliced */
            u_serviceChangeState(service, STATE_INITIALISING);
            u_serviceChangeState(service, STATE_OPERATIONAL);
            /* Get sleeptime from configuration */
            in_retrieveLeaseSettings(&leasePeriod, &sleepTime);

            u_serviceRenewLease(service, leasePeriod);
            /* Loop until termination is requested */
            u_serviceWatchSpliceDaemon(
                service,
                in_splicedaemonListener,
                &terminate);
            /* terminate flag is modified by the splice deamon listener thread*/
            while (!terminate)
            {
                /* Assert my liveliness and the Splicedaemon's liveliness */
                u_serviceRenewLease(service, leasePeriod);
                /* Wait before renewing again */
                os_nanoSleep(sleepTime);
            }
            leasePeriod.seconds = 20;
            u_serviceRenewLease(service, leasePeriod);
            u_serviceChangeState(service, STATE_TERMINATING);
            in_controllerStop(controller);
            in_controllerFree(controller);
            IN_REPORT_INFO(1, "DDSI networking stopped");
            IN_TRACE(Mainloop, 1, "DDSI networking stopped");
        }
        u_serviceChangeState(service, STATE_TERMINATED);
		u_serviceManagerFree(serviceManager);
		in_objectFree(in_object(admin));
    }
    /* Clean up */
    in_configFree(config);
    u_serviceFree(service);
}

void
in_retrieveLeaseSettings(
    v_duration* leaseExpiryTime,
    os_time* sleepTime)
{
    os_float leaseSec;
    os_float sleepSec;

    /* Get lease-period settings from config */
    leaseSec = in_configurationGetDomainLeaseExpiryTime();
    sleepSec = in_configurationGetDomainLeaseUpdateTime();

    sleepTime->tv_sec = (os_timeSec)sleepSec;
    sleepTime->tv_nsec = (os_int32)((sleepSec - (os_float)sleepTime->tv_sec) * 1000000000.0F);

    leaseExpiryTime->seconds = (os_int32)leaseSec;
    leaseExpiryTime->nanoseconds = (os_uint32)((leaseSec - (os_float)leaseExpiryTime->seconds) * 1000000000.0F);
}

void
in_splicedaemonListener(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData)
{
    os_boolean* terminate = (os_boolean*) usrData;

    switch (spliceDaemonState)
    {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
        *terminate = OS_TRUE;
    break;
    default:
        assert(OS_FALSE);
    break;
    }
}
