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

/* Interface */
#include "nw_configuration.h"
#include "nw_report.h"
#include "nw_profiling.h"

/* Implementation */
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "os_abstract.h"
#include "os_stdlib.h"
#include "os_time.h"
#include "os_heap.h"
#include "c_typebase.h"
#include "c_laptime.h"
#include "u_user.h"
#include "nw_misc.h"
#include "nw__confidence.h"

/* --------------------------------- Private -------------------------------- */


static c_char* ReportLevelMap[] = { "None",
                                    "Basic",
                                    "Low_frequent",
                                    "Medium_frequent",
                                    "High_frequent",
                                    "Full"
                                  };

static os_uint32
LookupReportLevel(
    const c_char * report_level);

#ifdef NW_TRACING

c_ulong highestTraceLevel = 0;

struct nw_traceConfig {
   FILE *outFile;
   c_bool timestamps;
   c_bool relTimestamps;
   os_time startTime;
   c_ulong levels[TC(Count)];
};

#endif /* NW_TRACING */

struct nw_lossyConfig {
    c_bool beLossy;
    c_ulong count;
    c_ulong threshold;
};

#ifdef NW_PROFILING
struct nw_lapAdmin {
    c_bool    doProfiling;
    c_laptime lapTimer;
    c_ulong   lapCount;
    c_ulong   reportOnceEvery;
    c_bool    running;
};

struct nw_profilingConfig {
    c_bool doProfiling;
    FILE *outFile;
    struct nw_lapAdmin lapAdmins[PR(Count)];
};
#endif

struct nw_reportingConfig {
    c_ulong verbosity;
    c_bool events;
    c_bool periodic;
    c_bool oneShot;
};


typedef struct nw_configuration_s {
    u_cfElement domainElement;
    u_cfElement networkingElement;
    /* Store commonly used parameters */
    struct nw_reportingConfig reporting;
#ifdef NW_LOOPBACK
    c_bool useLoopback;
    c_bool useComplementPartitions;
#endif /* NW_LOOPBACK */
#ifdef NW_TRACING
    struct nw_traceConfig traceConfig;
#endif /* NW_TRACING */
    struct nw_lossyConfig sendingLossiness;
    struct nw_lossyConfig receivingLossiness;
#ifdef NW_DEBUGGING
    c_bool noPacking;
#endif
#ifdef NW_PROFILING
    struct nw_profilingConfig profilingConfig;
#endif
    v_qos qos;
    /**
    * Boolean that indicates whether this networking service is configured for IPv6 or not.
    */
    c_bool isIPv6;
} *nw_configuration;


/* theConfiguration is hidden for users */
static nw_configuration
nw_configurationGetConfiguration(
    void)
{
    static struct nw_configuration_s theConfiguration;
    return &theConfiguration;
}

/* -------------------------- trace/verbosity/profiling stuff --------------- */

#ifdef NW_TRACING

static void
nw_configurationWaitIfRequested(
    nw_configuration configuration)
{
    const char * root = NWCF_ROOT(Debugging);
    c_ulong waitTime;
    os_time delay = {1, 0};

    if (configuration) {
        waitTime = NWCF_SIMPLE_PARAM(ULong, root, WaitForDebugger);
        while (waitTime > 0) {
            printf("Waiting for debugger to attach, %d seconds to go.\n", waitTime);
            os_nanoSleep(delay);
            waitTime--;
        }
    }
}

static void
nw_configurationInitializeTracing(
    nw_configuration configuration)
{
#define NW_STDOUT "stdout"
    struct nw_traceConfig *traceConfig;
    c_ulong defLvl;
    c_char *outFileName = NULL;
    const c_char *root;
    c_bool tracingEnabled;
    os_uint32 index;

    if (configuration) {
        traceConfig = &configuration->traceConfig;

        /* Node: Tracing */
        root = NWCF_ROOT(Tracing);
        tracingEnabled  = NWCF_SIMPLE_ATTRIB( Bool, root, enabled);

        if (tracingEnabled) {

            outFileName = NWCF_SIMPLE_PARAM(String, root, OutputFile);

            if (strncmp(outFileName, NW_STDOUT, (os_uint)sizeof(NW_STDOUT)) == 0) {
                traceConfig->outFile = stdout;
            } else {
                char * filename = os_fileNormalize(outFileName);
                traceConfig->outFile = fopen(filename, "w");
                if (!traceConfig->outFile) {
                     NW_REPORT_WARNING_2("Configuration",
                         "Can not open trace outputfile %s, "
                         "errno = %d. Switching to stdout.",
                         outFileName, errno);
                     traceConfig->outFile = stdout;
                }
                os_free(filename);
            }

            traceConfig->timestamps = NWCF_SIMPLE_PARAM(Bool, root, Timestamps);
            traceConfig->startTime = os_timeGet();
            /* In the old configuration timestamps had a relative property.
               In the new configuration timestamps have an absolute property.
               To minimize implementation changes, the value is simply negated.
            */
            traceConfig->relTimestamps = ! NWCF_SIMPLE_ATTRIB( Bool, NWCF_ROOT_Tracing NWCF_SEP NWCF_NAME_Timestamps, absolute);


            /* Change root to categories tag */
            root = NWCF_ROOT(Categories);

            /* Get the default tracing level as a string so it can be used as
             * default value for other parameters */
            defLvl = NWCF_SIMPLE_PARAM(ULong, root, Default);

            traceConfig->levels[TC(Configuration)] = NWCF_DEFAULTED_PARAM(ULong, root, Configuration, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Configuration)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Configuration)] :
                                    highestTraceLevel);

            /* Write the settings to trace-log because all trace messages have
             * been lost until now */
            NW_TRACE_3(Configuration, 1,
                       "Processed parameter %s"NWCF_SEP"%s, value is %s",
                       NWCF_ROOT(Tracing), NWCF_NAME(OutputFile), outFileName);
            os_free(outFileName);
            NW_TRACE_3(Configuration, 1,
                       "Processed parameter %s"NWCF_SEP"%s, value is %s",
                       NWCF_ROOT(Tracing), NWCF_NAME(Timestamps),
                       (traceConfig->timestamps ? "TRUE" : "FALSE"));
            NW_TRACE_3(Configuration, 1,
                       "Processed parameter %s"NWCF_SEP"%s, value is %s",
                       NWCF_ROOT(Tracing), NWCF_ATTRIB(absolute),
                       ((!(traceConfig->relTimestamps)) ? "TRUE" : "FALSE"));
            NW_TRACE_3(Configuration, 1,
                       "Processed parameter %s"NWCF_SEP"%s, value is %d",
                       root, NWCF_NAME(Default), defLvl);
            NW_TRACE_3(Configuration, 1,
                       "Processed parameter %s"NWCF_SEP"%s, value is %d",
                       root, NWCF_NAME(Configuration),
                       traceConfig->levels[TC(Configuration)]);
            /* After this, stop printing because the configuration parameters have
             * been set properly; printing will now happen automatically */

            traceConfig->levels[TC(Construction)]     = NWCF_DEFAULTED_PARAM(ULong, root, Construction, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Construction)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Construction)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Destruction)]      = NWCF_DEFAULTED_PARAM(ULong, root, Destruction, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Destruction)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Destruction)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Mainloop)]         = NWCF_DEFAULTED_PARAM(ULong, root, Mainloop, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Mainloop)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Mainloop)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Groups)]           = NWCF_DEFAULTED_PARAM(ULong, root, Groups, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Groups)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Groups)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Send)]             = NWCF_DEFAULTED_PARAM(ULong, root, Send, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Send)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Send)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Receive)]          = NWCF_DEFAULTED_PARAM(ULong, root, Receive, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Receive)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Receive)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Discovery)       ] = NWCF_DEFAULTED_PARAM(ULong, root, DiscoveryTracing, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Discovery)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Discovery)] :
                                    highestTraceLevel);
            traceConfig->levels[TC(Test)]             = NWCF_DEFAULTED_PARAM(ULong, root, Test, defLvl);
            highestTraceLevel = (traceConfig->levels[TC(Test)] > highestTraceLevel ?
                                    traceConfig->levels[TC(Test)] :
                                    highestTraceLevel);

        } else {
            for (index=0; index<TC(Count); index++) {
                 traceConfig->levels[index] = 0;
            }
        }
    }
#undef NW_STDOUT
}

static void
nw_configurationFinalizeTracing(
    nw_configuration configuration)
{
    if (configuration) {
        if (configuration->traceConfig.outFile) {
           if(configuration->traceConfig.outFile != stdout) {
               fclose(configuration->traceConfig.outFile);
           }
        }
    }
}

#endif /* NW_TRACING */

#ifdef NW_PROFILING

static const char *
nw_profClassName(
    nw_profilingClass profClass)
{
    const char *result = NULL;

#define ___CASE___(class) \
    case class: result = #class; break
#define __CASE__(class) \
   ___CASE___(PR(class))

    switch (profClass) {
        __CASE__(BridgeWrite); __CASE__(BridgeRead_1); __CASE__(BridgeRead_2);
        __CASE__(PlugWrite);   __CASE__(PlugRead_1);   __CASE__(PlugRead_2);
        __CASE__(Frag);        __CASE__(Defrag);       __CASE__(DefragCleanUp);
        __CASE__(SendTo);      __CASE__(RecvFrom);
        __CASE__(Serialization); __CASE__(Deserialization);
        default: NW_CONFIDENCE(FALSE); break;
    }
    return result;
}

static void
nw_initLapAdmin(
    struct nw_profilingConfig *profConfig,
    nw_profilingClass profClass)
{
    struct nw_lapAdmin *lapAdmin;

    lapAdmin = &(profConfig->lapAdmins[profClass]);
    if (lapAdmin->doProfiling) {
        lapAdmin->lapTimer = c_laptimeCreate(nw_profClassName(profClass));
        lapAdmin->lapCount = 0;
        lapAdmin->running = FALSE;
        profConfig->doProfiling = TRUE;
    }
}

static void
nw_configurationInitializeProfiling(
    nw_configuration configuration)
{
#define NW_STDOUT "stdout"
    struct nw_profilingConfig *profConfig;
    c_char *outFileName = NULL;
    c_bool profDefaultDo;
    c_ulong profDefaultEvery;
    struct nw_lapAdmin lapAdmin;
    const c_char *root;

    if (configuration) {
        profConfig = &configuration->profilingConfig;
        root = NWCF_ROOT(ProfDefault);

        profDefaultDo = NWCF_SIMPLE_PARAM(Bool, root, DoProfiling);
        profDefaultEvery = NWCF_SIMPLE_PARAM(ULong, root, ReportLaps);

        memset(&lapAdmin, 0, (os_uint)sizeof(lapAdmin));

        root = NWCF_ROOT(Bridge);
        lapAdmin.doProfiling = NWCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo );
        lapAdmin.reportOnceEvery = NWCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(BridgeWrite)] = lapAdmin;
        profConfig->lapAdmins[PR(BridgeRead_1)] = lapAdmin;
        profConfig->lapAdmins[PR(BridgeRead_2)] = lapAdmin;

        root = NWCF_ROOT(Plug);
        lapAdmin.doProfiling = NWCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = NWCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(PlugWrite)] = lapAdmin;
        profConfig->lapAdmins[PR(PlugRead_1)] = lapAdmin;
        profConfig->lapAdmins[PR(PlugRead_2)] = lapAdmin;

        root = NWCF_ROOT(Fragmentation);
        lapAdmin.doProfiling = NWCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = NWCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(Frag)] = lapAdmin;
        profConfig->lapAdmins[PR(Defrag)] = lapAdmin;
        profConfig->lapAdmins[PR(DefragCleanUp)] = lapAdmin;

        root = NWCF_ROOT(NetworkStack);
        lapAdmin.doProfiling = NWCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = NWCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(SendTo)] = lapAdmin;
        profConfig->lapAdmins[PR(RecvFrom)] = lapAdmin;

        root = NWCF_ROOT(Serialization);
        lapAdmin.doProfiling = NWCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = NWCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(Serialization)] = lapAdmin;
        profConfig->lapAdmins[PR(Deserialization)] = lapAdmin;

        /* Looping over enum variable not allowed by QAC, so call function
         * separately for each value */
        profConfig->doProfiling = FALSE;
        nw_initLapAdmin(profConfig, PR(BridgeWrite));
        nw_initLapAdmin(profConfig, PR(BridgeRead_1));
        nw_initLapAdmin(profConfig, PR(BridgeRead_2));
        nw_initLapAdmin(profConfig, PR(PlugWrite));
        nw_initLapAdmin(profConfig, PR(PlugRead_1));
        nw_initLapAdmin(profConfig, PR(PlugRead_2));
        nw_initLapAdmin(profConfig, PR(Frag));
        nw_initLapAdmin(profConfig, PR(Defrag));
        nw_initLapAdmin(profConfig, PR(DefragCleanUp));
        nw_initLapAdmin(profConfig, PR(SendTo));
        nw_initLapAdmin(profConfig, PR(RecvFrom));
        nw_initLapAdmin(profConfig, PR(Serialization));
        nw_initLapAdmin(profConfig, PR(Deserialization));

        if (profConfig->doProfiling) {
            root = NWCF_ROOT(Profiling);
            outFileName = NWCF_SIMPLE_PARAM(String, root, ProfOutFile);
            if (strncmp(outFileName, NW_STDOUT, (os_uint)sizeof(NW_STDOUT)) == 0) {
                profConfig->outFile = stdout;
            } else {
                char * filename = os_fileNormalize(outFileName);
                profConfig->outFile = fopen(filename, "w");
                if (!profConfig->outFile) {
                     NW_REPORT_WARNING_2("Configuration",
                         "Can not open profiling outputfile %s, "
                         "errno = %d. Switching to stdout.",
                         outFileName, errno);
                     profConfig->outFile = stdout;
                }
                os_free(filename);
            }
            os_free(outFileName);
        }
    }
#undef NW_STDOUT
}


static void
nw_finalizeLapAdmin(
    struct nw_profilingConfig *profConfig,
    nw_profilingClass profClass)
{
    struct nw_lapAdmin lapAdmin;

    lapAdmin = profConfig->lapAdmins[profClass];
    if (lapAdmin.doProfiling) {
        if (lapAdmin.lapCount > 0U) {
            c_laptimeReport(lapAdmin.lapTimer, "");
        }
        c_laptimeDelete(lapAdmin.lapTimer);
    }
}

static void
nw_configurationFinalizeProfiling(
    nw_configuration configuration)
{
    struct nw_profilingConfig *profConfig;

    if (configuration) {
        profConfig = &(configuration->profilingConfig);
        if (profConfig->doProfiling) {
            /* Looping over enum value not allowed by QAC,
             * so type a lot here */
            nw_finalizeLapAdmin(profConfig, PR(BridgeWrite));
            nw_finalizeLapAdmin(profConfig, PR(BridgeRead_1));
            nw_finalizeLapAdmin(profConfig, PR(BridgeRead_2));
            nw_finalizeLapAdmin(profConfig, PR(PlugWrite));
            nw_finalizeLapAdmin(profConfig, PR(PlugRead_1));
            nw_finalizeLapAdmin(profConfig, PR(PlugRead_2));
            nw_finalizeLapAdmin(profConfig, PR(Frag));
            nw_finalizeLapAdmin(profConfig, PR(Defrag));
            nw_finalizeLapAdmin(profConfig, PR(DefragCleanUp));
            nw_finalizeLapAdmin(profConfig, PR(SendTo));
            nw_finalizeLapAdmin(profConfig, PR(RecvFrom));
            nw_finalizeLapAdmin(profConfig, PR(Serialization));
            nw_finalizeLapAdmin(profConfig, PR(Deserialization));
            fclose(profConfig->outFile);
        }
    }
}
#endif /* NW_PROFILING */

#ifdef NW_LOOPBACK
static void
nw_configurationInitializeLoopback(
    nw_configuration configuration)
{
    const char *root;
    if (configuration) {
        root = NWCF_ROOT(Debugging);
        /* Loopback testing is supported conditionally */
        configuration->useLoopback = NWCF_SIMPLE_PARAM(Bool, root, UseLoopback);
        if (configuration->useLoopback) {
            /* Generate extra info because this option is for testing only */
            NW_REPORT_INFO(0, "Using loopback interface, for testing "
                              "purposes only");
        }

        configuration->useComplementPartitions =
            NWCF_SIMPLE_PARAM(Bool, root, UseComplementPartitions);
        if (configuration->useComplementPartitions) {
            /* Generate extra info, this is for testing only */
            NW_REPORT_INFO(0, "Using complement partitions, for testing "
                              "purposes only");
        }
    }

}
#endif /* NW_LOOPBACK */


static void
nw_configurationInitializeLossiness(
    nw_configuration configuration)
{
    const char *root;

    if (configuration) {
        root = NWCF_ROOT(LossySending);
        configuration->sendingLossiness.beLossy =
            NWCF_SIMPLE_PARAM( Bool, root, BeLossy);
        if (configuration->sendingLossiness.beLossy) {
            configuration->sendingLossiness.count = 0;
            configuration->sendingLossiness.threshold =
                NWCF_SIMPLE_PARAM(ULong, root, Threshold);
            if (configuration->sendingLossiness.threshold < NWCF_MIN(Threshold)) {
                NW_REPORT_WARNING_2("Configuration",
                    "Value %d for lossy sending threshold is too small, "
                    "switching to %d",
                    configuration->sendingLossiness.threshold,
                    NWCF_MIN(Threshold));
                configuration->sendingLossiness.threshold = NWCF_MIN(Threshold);
            }
        }

        root = NWCF_ROOT(LossyReceiving);
        configuration->receivingLossiness.beLossy =
            NWCF_SIMPLE_PARAM( Bool, root, BeLossy);
        if (configuration->receivingLossiness.beLossy) {
            configuration->receivingLossiness.count = 0;
            configuration->receivingLossiness.threshold =
                NWCF_SIMPLE_PARAM( ULong, root, Threshold);
            if (configuration->receivingLossiness.threshold < NWCF_MIN(Threshold)) {
                NW_REPORT_WARNING_2("Configuration",
                    "Value %d for lossy receiving threshold is too small, "
                    "switching to %d",
                    configuration->receivingLossiness.threshold,
                    NWCF_MIN(Threshold));
                configuration->receivingLossiness.threshold = NWCF_MIN(Threshold);
            }
        }
    }
}

#ifdef NW_DEBUGGING
static void
nw_configurationInitializeNoPacking(
    nw_configuration configuration)
{
    const char * root = NWCF_ROOT(Debugging);
    if (configuration) {
        configuration->noPacking = NWCF_SIMPLE_PARAM(Bool, root, NoPacking);
    }
}

static void
nw_configurationTestParameterTypes(
    nw_configuration configuration)
{
    char *str;
    const char *root;

    if (configuration) {
        root = NWCF_ROOT(Debugging);
        NWCF_SIMPLE_PARAM(Bool, root, Bool);
        NWCF_SIMPLE_PARAM(Long, root, Long);
        NWCF_SIMPLE_PARAM(ULong, root, ULong);
        NWCF_SIMPLE_PARAM(Float, root, Float);
        str = NWCF_SIMPLE_PARAM( String, root, String);
        NW_CONFIDENCE(str);
        if (str) {
            os_free(str);
        }
    }
}

#endif /* NW_DEBUG */

static void
nw_configurationInitializeConditionals(
    nw_configuration configuration)
{
#ifdef NW_TRACING
    nw_configurationWaitIfRequested(configuration);
    nw_configurationInitializeTracing(configuration);
#endif /* NW_TRACING */
#ifdef NW_LOOPBACK
    nw_configurationInitializeLoopback(configuration);
#endif /* NW_LOOPBACK */
    nw_configurationInitializeLossiness(configuration);
#ifdef NW_DEBUGGING
    nw_configurationInitializeNoPacking(configuration);
    nw_configurationTestParameterTypes(configuration);
#endif /* NW_DEBUGGING */
#ifdef NW_PROFILING
        nw_configurationInitializeProfiling(configuration);
#endif /* NW_PROFILING */

    if (configuration) {
        /* Verbosity */
        c_char * report_level;
        report_level = NWCF_DEFAULTED_PARAM(String, NWCF_ROOT(Reporting), Level, "None");
        configuration->reporting.verbosity = LookupReportLevel(report_level);
        os_free(report_level);

        configuration->reporting.events = NWCF_DEFAULTED_PARAM( Bool, NWCF_ROOT(Reporting),
                                                              Events, FALSE);
        configuration->reporting.periodic = NWCF_DEFAULTED_PARAM( Bool, NWCF_ROOT(Reporting),
                                                                Periodic, FALSE);
        configuration->reporting.oneShot = NWCF_DEFAULTED_PARAM( Bool, NWCF_ROOT(Reporting),
                                                               OneShot, FALSE);
    }
}

#ifdef NW_TRACING

static c_bool
nw_configurationInterested(
    nw_configuration configuration,
    nw_traceClass traceClass,
    c_ulong level)
{
    c_bool result = FALSE;

    if (configuration) {
        result = (c_bool)(configuration->traceConfig.levels[traceClass] >= level);
    }

    return result;
}

#endif /* NW_TRACING */

c_bool
nw_configurationLoseSentMessage(
    void)
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->sendingLossiness.beLossy) {
        /* No thread protection, doesn't matter because we are lossy anyway :-) */
        configuration->sendingLossiness.count++;
        if (configuration->sendingLossiness.count >
            configuration->sendingLossiness.threshold) {
            configuration->sendingLossiness.count = 0;
            result = TRUE;
        }
    }

    return result;
}

c_bool
nw_configurationLoseReceivedMessage(
    void)
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->receivingLossiness.beLossy) {
        /* No thread protection, doesn't matter because we are lossy anyway :-) */
        configuration->receivingLossiness.count++;
        if (configuration->receivingLossiness.count >
            configuration->receivingLossiness.threshold) {
            configuration->receivingLossiness.count = 0;
            result = TRUE;
        }
    }

    return result;
}

#ifdef NW_DEBUGGING
c_bool
nw_configurationNoPacking(
    void)
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->noPacking) {
        result = TRUE;
    }

    return result;
}
#endif

/* Helper functions for retrieving parameter values */

static c_bool
nw_configurationGetElementInternal(
    u_cfElement element,
    const c_char *elementPath,
    const char *elementName,
    u_cfElement *resultElement)
{
    u_cfElement helperElement;
    c_iter elementList;
    os_uint fullNameSize;
    c_char *elementFullName;

    *resultElement = NULL;

    if (element!= NULL && elementName != NULL ) {
        /* First build the complete name of the element */
        /* Note that the 0-terminator is included in the sizeof */
        if (elementPath != NULL) {
            fullNameSize = strlen(elementPath) + strlen(NWCF_SEP) + strlen(elementName) + 1;
        } else {
            fullNameSize = strlen(elementName) + 1;
        }
        elementFullName = os_malloc(fullNameSize);
        if (elementFullName != NULL) {
            if (elementPath != NULL) {
                os_sprintf(elementFullName, "%s%s%s", elementPath, NWCF_SEP, elementName);
            } else {
                os_sprintf(elementFullName, "%s", elementName);
            }
            /* Get complete list of interesting elements */
            elementList = u_cfElementXPath(element, elementFullName);
            /* Take first element and store it */
            *resultElement = u_cfElement(c_iterTakeFirst(elementList));
            /* Take all remaining elements and let them go */
            helperElement = u_cfElement(c_iterTakeFirst(elementList));
            while (helperElement) {
                u_cfElementFree(helperElement);
                helperElement = u_cfElement(c_iterTakeFirst(elementList));
            }
            c_iterFree(elementList);
            os_free(elementFullName);
        }
    }

    return (*resultElement != NULL);
}


static u_cfData
nw_configurationGetElementDataInternal(
    u_cfElement element)
{
    c_iter dataElements;
    u_cfData result = NULL;
    u_cfData helperElement;

    if (element) {
        /* Get data */
        dataElements = u_cfElementGetChildren(element);
        /* Take first element and store it */
        result = u_cfData(c_iterTakeFirst(dataElements));
        /* Take all remaining elements and let them go */
        helperElement = u_cfData(c_iterTakeFirst(dataElements));
        while (helperElement) {
            u_cfDataFree(helperElement);
            helperElement = u_cfData(c_iterTakeFirst(dataElements));
        }
        c_iterFree(dataElements);
    }

    return result;

}


static u_cfData
nw_configurationGetParameterData(
    const c_char *parameterPath,
    const c_char *parameterName)
{
    u_cfData result = NULL;
    nw_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->networkingElement) {
        found = nw_configurationGetElementInternal(
                    configuration->networkingElement,
                    parameterPath, parameterName, &element);
        if (found) {
            result = nw_configurationGetElementDataInternal(element);
        }
        u_cfElementFree(element);
    }

    return result;
}

static u_cfAttribute
nw_configurationGetParameterAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_bool *elementFound)
{
    u_cfAttribute result = NULL;
    nw_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    element = NULL;
    *elementFound = FALSE;
    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->networkingElement) {
        found = nw_configurationGetElementInternal(
                    configuration->networkingElement,
                    NULL, parameterPath, &element);
        if (found) {
            result = u_cfElementAttribute(element, attributeName);
            u_cfElementFree(element);
            *elementFound = TRUE;
        }
    }
   return result;
}

/* --------------------------------- Public --------------------------------- */

/* nw_configuration.h */
c_bool nw_configurationIsDiscoveryChannel(u_cfElement channel)
{
    c_iter elementList;
    c_bool result;

    elementList = u_cfElementXPath(channel, "Discovery");
    result = (c_iterLength(elementList)>0)?TRUE:FALSE;

    while (c_iterLength(elementList) > 0 ) {
        u_cfElementFree(u_cfElement(c_iterTakeFirst(elementList)));
    }
    c_iterFree(elementList);

    return result;
}



#define NW_SERV_NAME_ATTR "%s[@name='%s']"

void
nw_configurationInitialize(
    u_service service,
    const char *serviceName,
    const char *URI)
{
    nw_configuration configuration;
    u_cfElement topLevelElement;
    char * path;

    configuration = nw_configurationGetConfiguration();
    if (configuration) {
        /* Get the networkingelement and store it */
        topLevelElement = u_participantGetConfiguration(u_participant(service));

        /* Get element with tagname NetworkingService and corresponding name attribute*/
        path = os_malloc(strlen(NWCF_ROOT_NetworkingService)+strlen(NW_SERV_NAME_ATTR)+strlen(serviceName));
        os_sprintf(path, NW_SERV_NAME_ATTR, NWCF_ROOT_NetworkingService, serviceName);
        nw_configurationGetElementInternal(topLevelElement, NULL, path, &configuration->networkingElement);
        nw_configurationGetElementInternal(topLevelElement, NULL, NWCF_ROOT_Domain, &configuration->domainElement);

        os_free(path);
        u_cfElementFree(topLevelElement);
        nw_configurationInitializeConditionals(configuration);

    }
}


void
nw_configurationFinalize(
    void)
{
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    if (configuration) {
#ifdef NW_TRACING
        nw_configurationFinalizeTracing(configuration);
#endif /* NW_TRACING */
#ifdef NW_PROFILING
        nw_configurationFinalizeProfiling(configuration);
#endif /* NW_PROFILING */
        u_cfElementFree(configuration->networkingElement);
    }
}


/* Commonly used parameter */

c_bool
nw_configurationLevelIsInteresting(
    c_ulong level)
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)(configuration->reporting.verbosity >= level);
    }

    return result;
}


#ifdef NW_LOOPBACK

/* Functions for testing purposes only */

c_bool
nw_configurationUseLoopback()
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)((os_int)configuration->useLoopback == TRUE);
    }

    return result;
}


c_bool
nw_configurationUseComplementPartitions()
{
    c_bool result = FALSE;
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)((os_int)configuration->useComplementPartitions == TRUE);
    }

    return result;
}

#endif /* NW_LOOPBACK */

v_qos nw_configurationGetQos(void)
{
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    return configuration->qos;
}

/**
* Gets whether this service is configured to use IPv6
* or not.
* N.B. Currently this is process wide.
* @todo See dds#2692
*/
c_bool
nw_configurationGetIsIPv6(void)
{
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    return configuration->isIPv6;
}

/**
* Sets whether this service is configured to use IPv6
* or not.
* N.B. Currently this is process wide.
* @todo See dds#2692
*/
void
nw_configurationSetIsIPv6(c_bool isIPv6)
{
    nw_configuration configuration;

    configuration = nw_configurationGetConfiguration();
    configuration->isIPv6 = isIPv6;
}

/* Generic parameters */

c_bool
nw_configurationGetBoolParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_bool defaultValue)
{
    c_bool result;
    c_ulong longresult;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataBoolValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %s",
                       parameterPath, parameterName, (result ? "TRUE" : "FALSE"));
        } else {
            /* also accept integer values */
            success = u_cfDataULongValue(data, &longresult);
            if (success) {
                result = (c_bool)longresult;
                NW_TRACE_3(Configuration, 1,
                               "Retrieved parameter %s/%s, using value %s",
                               parameterPath, parameterName, (result ? "TRUE" : "FALSE"));
            } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for boolean parameter %s/%s,  "
                                "switching to default value %s",
                                parameterPath, parameterName, (defaultValue ? "TRUE" : "FALSE"));
            }
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %s",
            parameterPath, parameterName, (defaultValue ? "TRUE" : "FALSE"));
    }

    return result;
}

c_bool
nw_configurationGetBoolAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_bool defaultValueNoElmt,
    c_bool defaultValueNoAttr)
{
    c_bool result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        NW_CONFIDENCE(elmtFound);
        success = u_cfAttributeBoolValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %s",
                       parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for boolean attribute %s[@%s],  "
                                "switching to default value %s",
                                parameterPath, attributeName, (defaultValueNoAttr ? "TRUE" : "FALSE"));
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %s",
                parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        } else {
            result = defaultValueNoElmt;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %s",
                parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        }
    }

    return result;
}


c_long
nw_configurationGetLongParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_long defaultValue)
{
    c_long result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataLongValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %d",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long parameter %s/%s,  "
                                "switching to default value %d",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %d",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}


c_long nw_configurationGetLongAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_long defaultValueNoElmt,
    c_long defaultValueNoAttr)
{
    c_long result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        NW_CONFIDENCE(elmtFound);
        success = u_cfAttributeLongValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %d",
                       parameterPath, attributeName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %d",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %d",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %d",
                parameterPath, attributeName, result);
        }
    }

    return result;
}


c_ulong
nw_configurationGetULongParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_ulong defaultValue)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataULongValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %u",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for unsigned long parameter %s/%s,  "
                                "switching to default value %u",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %u",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_size
nw_configurationGetSizeParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_size defaultValue)
{
    c_size result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataSizeValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value " PA_SIZEFMT,
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for unsigned long parameter %s/%s,  "
                                "switching to default value " PA_SIZEFMT,
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value " PA_SIZEFMT,
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_ulong nw_configurationGetULongAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_ulong defaultValueNoElmt,
    c_ulong defaultValueNoAttr)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        NW_CONFIDENCE(elmtFound);
        success = u_cfAttributeULongValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %u",
                       parameterPath, attributeName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %u",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %u",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %u",
                parameterPath, attributeName, result);
        }
    }

    return result;
}

c_size nw_configurationGetSizeAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_size defaultValueNoElmt,
    c_size defaultValueNoAttr)
{
    c_size result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        NW_CONFIDENCE(elmtFound);
        success = u_cfAttributeSizeValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value " PA_SIZEFMT,
                       parameterPath, attributeName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value " PA_SIZEFMT,
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value " PA_SIZEFMT,
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value " PA_SIZEFMT,
                parameterPath, attributeName, result);
        }
    }

    return result;
}

c_float
nw_configurationGetFloatParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataFloatValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %f",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for float parameter %s/%s,  "
                                "switching to default value %f",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %f",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_float nw_configurationGetFloatAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_float defaultValueNoElmt,
    c_float defaultValueNoAttr)
{
    c_float result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        NW_CONFIDENCE(elmtFound);
        success = u_cfAttributeFloatValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %f",
                       parameterPath, attributeName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %f",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %f",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %f",
                parameterPath, attributeName, result);
        }
    }

    return result;
}


/* Do not forget to os_free the result after use */
c_string
nw_configurationGetStringParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    const c_char *defaultValue)
{
    c_string result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataStringValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %s",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for string parameter %s/%s,  "
                                "switching to default value %s",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = nw_stringDup(defaultValue);
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %s",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

/* Do not forget to os_free the result after use */
c_string
nw_configurationGetStringAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    const c_char *defaultValueNoElmt,
    const c_char *defaultValueNoAttr)
{
    c_string result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = nw_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        success = u_cfAttributeStringValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s/%s, using value %s",
                       parameterPath, attributeName, result );
        } else {
            NW_REPORT_WARNING_3("retrieving configuration attributes",
                                "incorrect format for boolean attribute %s,  "
                                "switching to default value %s",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = nw_stringDup(defaultValueNoAttr);
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s/%s, switching to default value %s",
                parameterPath, attributeName, (result ? result : "(null)"));
        } else {
            result = nw_stringDup(defaultValueNoElmt);
            NW_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %s",
                parameterPath, attributeName, (result ? result : "(null)"));
        }
    }

    return result;
}


NW_STRUCT(nw_nameList) {
    c_ulong nofNames;
    c_char **names;
};

c_bool
nw_configurationElementHasChildElementWithName(
    const c_char* path,
    const c_char* tagName)
{
    nw_configuration configuration;
    c_iter elementList = NULL;
    c_iter childElementList = NULL;
    c_object element;
    c_bool found = FALSE;

    NW_CONFIDENCE(path != NULL);
    NW_CONFIDENCE(tagName != NULL);

    configuration = nw_configurationGetConfiguration();
    if (configuration && configuration->networkingElement) {
        elementList = u_cfElementXPath(configuration->networkingElement, path);
    }
    element = c_iterTakeFirst(elementList);
    while (element != NULL) {
        childElementList = u_cfElementXPath(element, tagName);
        if(c_iterLength(childElementList)>0){
            found = TRUE;
            while (c_iterLength(childElementList) > 0 ) {
                u_cfElementFree(u_cfElement(c_iterTakeFirst(childElementList)));
            }
            c_iterFree(childElementList);
        }
        u_cfElementFree(element);
        element = c_iterTakeFirst(elementList);
    }
    c_iterFree(elementList);

    return found;
}

c_iter
nw_configurationGetElements(
    const c_char* path)
{
    nw_configuration configuration;
    c_iter elementList = NULL;
    c_iter elementListOrdered = NULL;
    c_object element;

    NW_CONFIDENCE(path != NULL);
    configuration = nw_configurationGetConfiguration();
    if (configuration && configuration->networkingElement) {
        elementList = u_cfElementXPath(configuration->networkingElement, path);
    }
    /* XPath inverts the order, so re-invert here */
    element = c_iterTakeFirst(elementList);
    while (element != NULL) {
        elementListOrdered = c_iterInsert(elementListOrdered, element);
        element = c_iterTakeFirst(elementList);
    }
    c_iterFree(elementList);

    return elementListOrdered;
}

nw_nameList
nw_configurationGetChildElementPaths(
    const c_char *parameterPath,
    const c_char *attribName)
{
    nw_nameList result = NULL;
    c_iter elementList;
    u_cfElement element;
    nw_configuration configuration;
    int nofElements;
    int nofElementsFound;
    int i;
    int pathSize;
    c_char *fullPath;
    int sRes;
    u_cfAttribute attrib;
    c_char *attribValue;
    c_bool attribRes;
    c_bool error;

    NW_CONFIDENCE(parameterPath != NULL);
    NW_CONFIDENCE(attribName != NULL);

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->networkingElement) {
        elementList = u_cfElementXPath(configuration->networkingElement, parameterPath);
        nofElements = c_iterLength(elementList);
        nofElementsFound = 0;
        if (nofElements > 0) {
            result = os_malloc(sizeof(*result));
            result->names = os_malloc(nofElements*sizeof(*result->names));
            for (i=0; i<nofElements; i++) {
                error = FALSE;
                element = u_cfElement(c_iterTakeFirst(elementList));
                NW_CONFIDENCE(element != NULL);
                attrib = u_cfElementAttribute(element, attribName);
                if (attrib != NULL) {
                    attribRes = u_cfAttributeStringValue(attrib, &attribValue);
                    NW_CONFIDENCE(attribRes);
#define NW_PATH_FORMAT "%s[@%s='%s']"
#define NW_PATH_STRING "[@='']"
                    pathSize = strlen(parameterPath) +
                        strlen(attribName) + strlen(attribValue) +
                        sizeof(NW_PATH_STRING);
                    fullPath = os_malloc(pathSize);
                    sRes = snprintf(fullPath, pathSize, NW_PATH_FORMAT,
                        parameterPath, attribName, attribValue);
                    NW_CONFIDENCE(sRes+1 == pathSize);
#undef NW_PATH_FORMAT
#undef NW_PATH_STRING
                    result->names[nofElementsFound] = fullPath;
                    nofElementsFound++;
                    os_free(attribValue);
                    u_cfAttributeFree(attrib);
                } else {
                    error = TRUE;
                    NW_REPORT_WARNING_2("parsing configuration",
                        "element %s does not contain required attribute %s",
                        parameterPath, attribName);
                }
                u_cfElementFree(element);
            };
            assert(c_iterLength(elementList) == 0);
            result->nofNames = nofElementsFound;
        }
        c_iterFree(elementList);
    }

    return result;
}

int
nw_nameListGetCount(
    nw_nameList nameList)
{
    int result = 0;

    if (nameList != NULL) {
        result = nameList->nofNames;
    }

    return result;
}

const c_char *
nw_nameListGetName(
    nw_nameList nameList,
    c_ulong index)
{
    const char *result = NULL;

    if ((nameList != NULL) && (nameList->nofNames > index)) {
        result = nameList->names[index];
    }

    return result;
}


void
nw_nameListFree(
    nw_nameList nameList)
{
    c_ulong i;

    if (nameList != NULL) {
        for (i=0; i<nameList->nofNames; i++) {
            os_free(nameList->names[i]);
        }
        os_free(nameList->names);
        os_free(nameList);
    }
}

static os_uint32
LookupReportLevel(
    const c_char * report_level)
{
    os_uint32 result = 0;
    os_uint32 i;

    for (i=0; i<(sizeof(ReportLevelMap)/sizeof(c_char*)); i++)
    {
        if (strcmp(report_level, ReportLevelMap[i]) == 0)
        {
            result = i;
            break;
        }
    }
    return result;
}

/* --------------------------------- Tracing -------------------------------- */

/* implements nw_report.h */

#ifdef NW_TRACING

void
nw_reportTrace(
    nw_traceClass traceClass,
    c_ulong level,
    const c_char *context,
    const char *description, ...)
{
    nw_configuration configuration;
    va_list ap;
    os_time useTime;

    configuration = nw_configurationGetConfiguration();

    if (configuration) {
        if (nw_configurationInterested(configuration, traceClass, level) &&
            configuration->traceConfig.outFile) {
            if (configuration->traceConfig.timestamps) {
                if (configuration->traceConfig.relTimestamps) {
                    useTime = os_timeSub(os_timeGet(), configuration->traceConfig.startTime);
                } else {
                    useTime = os_timeGet();
                }
                fprintf(configuration->traceConfig.outFile, "%5d.%3.3d ",
                        useTime.tv_sec, useTime.tv_nsec/1000000);
            }
            fprintf(configuration->traceConfig.outFile, "%-14s (%d) ",
                    context, level);
            va_start(ap, description);
            vfprintf(configuration->traceConfig.outFile, description, ap);
            va_end(ap);
            fflush(configuration->traceConfig.outFile);
        }

    }
}

#endif /* NW_TRACING */

/* ------------------------------- Profiling -------------------------------- */

/* implements nw_profiling.h */

#ifdef NW_PROFILING

void nw_profilingLapStart(
    nw_profilingClass profilingClass)
{
    nw_configuration configuration;
    struct nw_lapAdmin *lapAdmin;


    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->profilingConfig.doProfiling) {
        lapAdmin = &(configuration->profilingConfig.lapAdmins[profilingClass]);
        if (lapAdmin->doProfiling) {
            if (lapAdmin->running) {
                NW_REPORT_ERROR_1("nw_profilingLapStart",
                    "Laptimer %s is already running",
                    nw_profClassName(profilingClass));

            } else {
                c_laptimeStart(lapAdmin->lapTimer);
                lapAdmin->running = TRUE;
            }
        }
    }
}

void nw_profilingLapStop(
    nw_profilingClass profilingClass)
{
    nw_configuration configuration;
    struct nw_lapAdmin *lapAdmin;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->profilingConfig.doProfiling) {
        lapAdmin = &(configuration->profilingConfig.lapAdmins[profilingClass]);
        if (lapAdmin->doProfiling) {
            if (!(int)lapAdmin->running) {
                NW_REPORT_ERROR_1("nw_profilingLapStop",
                    "Laptimer %s is not running",
                    nw_profClassName(profilingClass));

            } else {
                c_laptimeStop(lapAdmin->lapTimer);
                lapAdmin->running = FALSE;
                lapAdmin->lapCount++;
                if (lapAdmin->lapCount == lapAdmin->reportOnceEvery) {
                    c_laptimeReport(lapAdmin->lapTimer, "");
                    c_laptimeReset(lapAdmin->lapTimer);
                    lapAdmin->lapCount = 0;
                }
            }
        }
    }
}


#endif /* NW_PROFILING */


/* ----------------------------- Domain parameters -------------------------- */

static u_cfData
nw_configurationGetDomainParameterData(
    const c_char *parameterPath,
    const c_char *parameterName)
{
    u_cfData result = NULL;
    nw_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->domainElement) {
        found = nw_configurationGetElementInternal(
                    configuration->domainElement,
                    parameterPath, parameterName, &element);
        if (found) {
            result = nw_configurationGetElementDataInternal(element);
        }
        u_cfElementFree(element);
    }

    return result;
}


static c_float
nw_configurationGetDomainFloatParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetDomainParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataFloatValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %f",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for float parameter %s/%s,  "
                                "switching to default value %f",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %f",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

static c_string
nw_configurationGetDomainStringParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_string defaultValue)
{
    c_string result;
    c_bool success = FALSE;
    u_cfData data;

    data = nw_configurationGetDomainParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataStringValue(data, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %s",
                       parameterPath, parameterName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for string parameter %s/%s,  "
                                "switching to default value %s",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %s",
            (parameterPath ? parameterPath : "(null)") , parameterName, defaultValue);
    }

    return result;
}



static u_cfAttribute
nw_configurationGetDomainParameterAttribute(
    const c_char *parameterPath,
    const c_char *attributeName)
{
    u_cfAttribute result = NULL;
    nw_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    element = NULL;
    configuration = nw_configurationGetConfiguration();

    if (configuration && configuration->domainElement) {
        found = nw_configurationGetElementInternal(
                    configuration->domainElement,
                    NULL, parameterPath, &element);
        if (found) {
            result = u_cfElementAttribute(element, attributeName);
            u_cfElementFree(element);
        }
    }
   return result;
}


static c_float
nw_configurationGetDomainFloatAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfAttribute attr;

    attr = nw_configurationGetDomainParameterAttribute(parameterPath, attributeName);
    if (attr) {
        success = u_cfAttributeFloatValue(attr, &result);
        if (success) {
            NW_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %f",
                       parameterPath, attributeName, result);
        } else {
            NW_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %f",
                                parameterPath, attributeName, defaultValue);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        result = defaultValue;
        NW_TRACE_3(Configuration, 2,
            "Could not retrieve attribute %s[@%s], switching to default value %f",
            parameterPath, attributeName, defaultValue);
    }

    return result;
}



#define LEASE_PATH         "Lease"
#define EXPIRYTIME_NAME    "ExpiryTime"
#define UPDATEFACTOR_NAME  "update_factor"
#define ROLE_NAME          "Role"

#define DEF_EXPIRYTIME     (10.0F)
#define MIN_EXPIRYTIME     ( 0.2F)

#define DEF_UPDATEFACTOR   (0.10F)
#define MIN_UPDATEFACTOR   (0.05F)
#define MAX_UPDATEFACTOR   (0.90F)

#define DEF_ROLE           ("")

static c_float
nw_configurationGetUpdateFactor()
{
    c_float result;

    result = nw_configurationGetDomainFloatAttribute(LEASE_PATH "/" EXPIRYTIME_NAME, UPDATEFACTOR_NAME, DEF_UPDATEFACTOR);
    if (result < MIN_UPDATEFACTOR) {
        result = MIN_UPDATEFACTOR;
    }
    if (result > MAX_UPDATEFACTOR) {
        result = MAX_UPDATEFACTOR;
    }

    return result;
}


c_float
nw_configurationGetDomainLeaseExpiryTime()
{
    c_float result;

    result =  nw_configurationGetDomainFloatParameter(LEASE_PATH, EXPIRYTIME_NAME, DEF_EXPIRYTIME);
    if (result < MIN_EXPIRYTIME) {
        result = MIN_EXPIRYTIME;
    }
    return result;
}

c_float
nw_configurationGetDomainLeaseUpdateTime()
{
   c_float result;
   c_float updateFactor;
   c_float expiryTime;


   updateFactor = nw_configurationGetUpdateFactor();
   expiryTime = nw_configurationGetDomainLeaseExpiryTime();
   result = updateFactor * expiryTime;

   return result;
}

c_string
nw_configurationGetDomainRole()
{
    c_string result;

    result =  nw_configurationGetDomainStringParameter(NULL, ROLE_NAME, DEF_ROLE);
    return result;
}


