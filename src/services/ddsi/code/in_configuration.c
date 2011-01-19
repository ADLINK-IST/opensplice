
/* Interface */
#include "in_configuration.h"

/* Implementation */
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "os_stdlib.h"
#include "os_time.h"
#include "os_heap.h"
#include "c_typebase.h"
#include "c_laptime.h"
#include "u_user.h"
#include "in_report.h"
#include "in_profiling.h"
#include "in_misc.h"

static FILE* outputFile = NULL;
/* --------------------------------- Private -------------------------------- */


static c_char* ReportLevelMap[] = { "None",
                                    "Basic",
                                    "Low_frequent",
                                    "Medium_frequent",
                                    "High_frequent",
                                    "Full"
                                  };

static unsigned int
LookupReportLevel(
    const c_char * report_level);

#ifdef IN_TRACING

struct in_traceConfig {
   FILE *outFile;
   c_bool timestamps;
   c_bool relTimestamps;
   os_time startTime;
   c_ulong levels[TC(Count)];
};

#endif /* IN_TRACING */

#ifdef IN_DEBUGGING
struct in_lossyConfig {
    c_bool beLossy;
    c_ulong count;
    c_ulong threshold;
};
#endif

#ifdef IN_PROFILING
struct in_lapAdmin {
    c_bool    doProfiling;
    c_laptime lapTimer;
    c_ulong   lapCount;
    c_ulong   reportOnceEvery;
    c_bool    running;
};

struct in_profilingConfig {
    c_bool doProfiling;
    FILE *outFile;
    struct in_lapAdmin lapAdmins[PR(Count)];
};
#endif

struct in_reportingConfig {
    c_ulong verbosity;
    c_bool events;
    c_bool periodic;
    c_bool oneShot;
};


typedef struct in_configuration_s {
    u_cfElement domainElement;
    u_cfElement networkingElement;
    /* Store commonly used parameters */
    struct in_reportingConfig reporting;
#ifdef IN_LOOPBACK
    c_bool useLoopback;
    c_bool useComplementPartitions;
#endif /* IN_LOOPBACK */
#ifdef IN_TRACING
    struct in_traceConfig traceConfig;
#endif /* IN_TRACING */
#ifdef IN_DEBUGGING
    struct in_lossyConfig sendingLossiness;
    struct in_lossyConfig receivingLossiness;
    c_bool noPacking;
#endif
#ifdef IN_PROFILING
    struct in_profilingConfig profilingConfig;
#endif
    v_qos qos;
} *in_configuration;


/* theConfiguration is hidden for users */
static in_configuration
in_configurationGetConfiguration(
    void)
{
    static struct in_configuration_s theConfiguration;
    return &theConfiguration;
}

/* -------------------------- trace/verbosity/profiling stuff --------------- */

#ifdef IN_TRACING


static void
in_configurationInitializeTracing(
    in_configuration configuration)
{
#define IN_STDOUT "stdout"
    struct in_traceConfig *traceConfig;
    c_ulong defLvl;
    c_char *outFileName = NULL;
    const c_char *root;
    c_bool tracingEnabled;
    unsigned int index;

    if (configuration) {
        traceConfig = &configuration->traceConfig;

        /* Node: Tracing */
        root = INCF_ROOT(Tracing);
        tracingEnabled  = INCF_SIMPLE_ATTRIB( Bool, root, enabled);

        if (tracingEnabled) {

            outFileName = INCF_SIMPLE_PARAM(String, root, OutputFile);

            if (strncmp(outFileName, IN_STDOUT, (unsigned int)sizeof(IN_STDOUT)) == 0) {
                traceConfig->outFile = stdout;
            } else {
                traceConfig->outFile = fopen(outFileName, "w");
                if (!traceConfig->outFile) {
                     IN_REPORT_WARNING_2("Configuration",
                         "Can not open trace outputfile %s, "
                         "errno = %d. Switching to stdout.",
                         outFileName, errno);
                     traceConfig->outFile = stdout;
                }
            }
            /* os_free(outFileName); outFileName is freed later */

            traceConfig->timestamps = INCF_SIMPLE_PARAM(Bool, root, Timestamps);
            traceConfig->startTime = os_timeGet();
            /* In the old configuration timestamps had a relative property.
               In the new configuration timestamps have an absolute property.
               To minimize implementation changes, the value is simply negated.
            */
            traceConfig->relTimestamps = ! INCF_SIMPLE_ATTRIB( Bool, INCF_ROOT_Tracing INCF_SEP INCF_NAME_Timestamps, absolute);


            /* Change root to categories tag */
            root = INCF_ROOT(Categories);

            /* Get the default tracing level as a string so it can be used as
             * default value for other parameters */
            defLvl = INCF_SIMPLE_PARAM(ULong, root, Default);

            traceConfig->levels[TC(Configuration)] = INCF_DEFAULTED_PARAM(ULong, root, Configuration, defLvl);

            /* Write the settings to trace-log because all trace messages have
             * been lost until now */
            IN_TRACE_3(Configuration, 1,
                       "Processed parameter %s"INCF_SEP"%s, value is %s",
                       INCF_ROOT(Tracing), INCF_NAME(OutputFile), outFileName);
            os_free(outFileName);
            IN_TRACE_3(Configuration, 1,
                       "Processed parameter %s"INCF_SEP"%s, value is %s",
                       INCF_ROOT(Tracing), INCF_NAME(Timestamps),
                       (traceConfig->timestamps ? "TRUE" : "FALSE"));
            IN_TRACE_3(Configuration, 1,
                       "Processed parameter %s"INCF_SEP"%s, value is %s",
                       INCF_ROOT(Tracing), INCF_ATTRIB(absolute),
                       ((!(traceConfig->relTimestamps)) ? "TRUE" : "FALSE"));
            IN_TRACE_3(Configuration, 1,
                       "Processed parameter %s"INCF_SEP"%s, value is %d",
                       root, INCF_NAME(Default), defLvl);
            IN_TRACE_3(Configuration, 1,
                       "Processed parameter %s"INCF_SEP"%s, value is %d",
                       root, INCF_NAME(Configuration),
                       traceConfig->levels[TC(Configuration)]);
            /* After this, stop printing because the configuration parameters have
             * been set properly; printing will now happen automatically */

            traceConfig->levels[TC(Construction)]     = INCF_DEFAULTED_PARAM(ULong, root, Construction, defLvl);
            traceConfig->levels[TC(Destruction)]      = INCF_DEFAULTED_PARAM(ULong, root, Destruction, defLvl);
            traceConfig->levels[TC(Mainloop)]         = INCF_DEFAULTED_PARAM(ULong, root, Mainloop, defLvl);
            traceConfig->levels[TC(Groups)]           = INCF_DEFAULTED_PARAM(ULong, root, Groups, defLvl);
            traceConfig->levels[TC(Send)]             = INCF_DEFAULTED_PARAM(ULong, root, Send, defLvl);
            traceConfig->levels[TC(Receive)]          = INCF_DEFAULTED_PARAM(ULong, root, Receive, defLvl);
            traceConfig->levels[TC(Discovery)       ] = INCF_DEFAULTED_PARAM(ULong, root, DiscoveryTracing, defLvl);
            traceConfig->levels[TC(Test)]             = INCF_DEFAULTED_PARAM(ULong, root, Test, defLvl);

        } else {
            for (index=0; index<TC(Count); index++) {
                 traceConfig->levels[index] = 0;
            }
        }
    }
#undef IN_STDOUT
}

static void
in_configurationFinalizeTracing(
    in_configuration configuration)
{
    if (configuration) {
        if (configuration->traceConfig.outFile) {
           fclose(configuration->traceConfig.outFile);
        }
    }
}

#endif /* IN_TRACING */

#ifdef IN_PROFILING

static const char *
in_profClassName(
    in_profilingClass profClass)
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
        default: assert(FALSE); break;
    }
    return result;
}

static void
in_initLapAdmin(
    struct in_profilingConfig *profConfig,
    in_profilingClass profClass)
{
    struct in_lapAdmin *lapAdmin;

    lapAdmin = &(profConfig->lapAdmins[profClass]);
    if (lapAdmin->doProfiling) {
        lapAdmin->lapTimer = c_laptimeCreate(in_profClassName(profClass));
        lapAdmin->lapCount = 0;
        lapAdmin->running = FALSE;
        profConfig->doProfiling = TRUE;
    }
}

static void
in_configurationInitializeProfiling(
    in_configuration configuration)
{
#define IN_STDOUT "stdout"
    struct in_profilingConfig *profConfig;
    c_char *outFileName = NULL;
    c_bool profDefaultDo;
    c_ulong profDefaultEvery;
    struct in_lapAdmin lapAdmin;
    const c_char *root;

    if (configuration) {
        profConfig = &configuration->profilingConfig;
        root = INCF_ROOT(ProfDefault);

        profDefaultDo = INCF_SIMPLE_PARAM(Bool, root, DoProfiling);
        profDefaultEvery = INCF_SIMPLE_PARAM(ULong, root, ReportLaps);

        memset(&lapAdmin, 0, (unsigned int)sizeof(lapAdmin));

        root = INCF_ROOT(Bridge);
        lapAdmin.doProfiling = INCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo );
        lapAdmin.reportOnceEvery = INCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(BridgeWrite)] = lapAdmin;
        profConfig->lapAdmins[PR(BridgeRead_1)] = lapAdmin;
        profConfig->lapAdmins[PR(BridgeRead_2)] = lapAdmin;

        root = INCF_ROOT(Plug);
        lapAdmin.doProfiling = INCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = INCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(PlugWrite)] = lapAdmin;
        profConfig->lapAdmins[PR(PlugRead_1)] = lapAdmin;
        profConfig->lapAdmins[PR(PlugRead_2)] = lapAdmin;

        root = INCF_ROOT(Fragmentation);
        lapAdmin.doProfiling = INCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = INCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(Frag)] = lapAdmin;
        profConfig->lapAdmins[PR(Defrag)] = lapAdmin;
        profConfig->lapAdmins[PR(DefragCleanUp)] = lapAdmin;

        root = INCF_ROOT(NetworkStack);
        lapAdmin.doProfiling = INCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = INCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(SendTo)] = lapAdmin;
        profConfig->lapAdmins[PR(RecvFrom)] = lapAdmin;

        root = INCF_ROOT(Serialization);
        lapAdmin.doProfiling = INCF_DEFAULTED_PARAM( Bool, root, DoProfiling, profDefaultDo);
        lapAdmin.reportOnceEvery = INCF_DEFAULTED_PARAM( ULong, root, ReportLaps,
                                                       profDefaultEvery);
        profConfig->lapAdmins[PR(Serialization)] = lapAdmin;
        profConfig->lapAdmins[PR(Deserialization)] = lapAdmin;

        /* Looping over enum variable not allowed by QAC, so call function
         * separately for each value */
        profConfig->doProfiling = FALSE;
        in_initLapAdmin(profConfig, PR(BridgeWrite));
        in_initLapAdmin(profConfig, PR(BridgeRead_1));
        in_initLapAdmin(profConfig, PR(BridgeRead_2));
        in_initLapAdmin(profConfig, PR(PlugWrite));
        in_initLapAdmin(profConfig, PR(PlugRead_1));
        in_initLapAdmin(profConfig, PR(PlugRead_2));
        in_initLapAdmin(profConfig, PR(Frag));
        in_initLapAdmin(profConfig, PR(Defrag));
        in_initLapAdmin(profConfig, PR(DefragCleanUp));
        in_initLapAdmin(profConfig, PR(SendTo));
        in_initLapAdmin(profConfig, PR(RecvFrom));
        in_initLapAdmin(profConfig, PR(Serialization));
        in_initLapAdmin(profConfig, PR(Deserialization));

        if (profConfig->doProfiling) {
            root = INCF_ROOT(Profiling);
            outFileName = INCF_SIMPLE_PARAM(String, root, ProfOutFile);
            if (strncmp(outFileName, IN_STDOUT, (unsigned int)sizeof(IN_STDOUT)) == 0) {
                profConfig->outFile = stdout;
            } else {
                profConfig->outFile = fopen(outFileName, "w");
                if (!profConfig->outFile) {
                     IN_REPORT_WARNING_2("Configuration",
                         "Can not open profiling outputfile %s, "
                         "errno = %d. Switching to stdout.",
                         outFileName, errno);
                     profConfig->outFile = stdout;
                }
            }
            os_free(outFileName);
        }
    }
#undef IN_STDOUT
}


static void
in_finalizeLapAdmin(
    struct in_profilingConfig *profConfig,
    in_profilingClass profClass)
{
    struct in_lapAdmin lapAdmin;

    lapAdmin = profConfig->lapAdmins[profClass];
    if (lapAdmin.doProfiling) {
        if (lapAdmin.lapCount > 0U) {
            c_laptimeReport(lapAdmin.lapTimer, "");
        }
        c_laptimeDelete(lapAdmin.lapTimer);
    }
}

static void
in_configurationFinalizeProfiling(
    in_configuration configuration)
{
    struct in_profilingConfig *profConfig;

    if (configuration) {
        profConfig = &(configuration->profilingConfig);
        if (profConfig->doProfiling) {
            /* Looping over enum value not allowed by QAC,
             * so type a lot here */
            in_finalizeLapAdmin(profConfig, PR(BridgeWrite));
            in_finalizeLapAdmin(profConfig, PR(BridgeRead_1));
            in_finalizeLapAdmin(profConfig, PR(BridgeRead_2));
            in_finalizeLapAdmin(profConfig, PR(PlugWrite));
            in_finalizeLapAdmin(profConfig, PR(PlugRead_1));
            in_finalizeLapAdmin(profConfig, PR(PlugRead_2));
            in_finalizeLapAdmin(profConfig, PR(Frag));
            in_finalizeLapAdmin(profConfig, PR(Defrag));
            in_finalizeLapAdmin(profConfig, PR(DefragCleanUp));
            in_finalizeLapAdmin(profConfig, PR(SendTo));
            in_finalizeLapAdmin(profConfig, PR(RecvFrom));
            in_finalizeLapAdmin(profConfig, PR(Serialization));
            in_finalizeLapAdmin(profConfig, PR(Deserialization));
            fclose(profConfig->outFile);
        }
    }
}
#endif /* IN_PROFILING */


#ifdef IN_DEBUGGING

static void
in_configurationInitializeLossiness(
    in_configuration configuration)
{
    const char *root;

    if (configuration) {
        root = INCF_ROOT(LossySending);
        configuration->sendingLossiness.beLossy =
            INCF_SIMPLE_PARAM( Bool, root, BeLossy);
        if (configuration->sendingLossiness.beLossy) {
            configuration->sendingLossiness.count = 0;
            configuration->sendingLossiness.threshold =
                INCF_SIMPLE_PARAM(ULong, root, Threshold);
            if (configuration->sendingLossiness.threshold < INCF_MIN(Threshold)) {
                IN_REPORT_WARNING_2("Configuration",
                    "Value %d for lossy sending threshold is too small, "
                    "switching to %d",
                    configuration->sendingLossiness.threshold,
                    INCF_MIN(Threshold));
                configuration->sendingLossiness.threshold = INCF_MIN(Threshold);
            }
        }

        root = INCF_ROOT(LossyReceiving);
        configuration->receivingLossiness.beLossy =
            INCF_SIMPLE_PARAM( Bool, root, BeLossy);
        if (configuration->receivingLossiness.beLossy) {
            configuration->receivingLossiness.count = 0;
            configuration->receivingLossiness.threshold =
                INCF_SIMPLE_PARAM( ULong, root, Threshold);
            if (configuration->receivingLossiness.threshold < INCF_MIN(Threshold)) {
                IN_REPORT_WARNING_2("Configuration",
                    "Value %d for lossy receiving threshold is too small, "
                    "switching to %d",
                    configuration->receivingLossiness.threshold,
                    INCF_MIN(Threshold));
                configuration->receivingLossiness.threshold = INCF_MIN(Threshold);
            }
        }
    }
}

static void
in_configurationInitializeNoPacking(
    in_configuration configuration)
{
    const char * root = INCF_ROOT(Debugging);
    if (configuration) {
        configuration->noPacking = INCF_SIMPLE_PARAM(Bool, root, NoPacking);
    }
}

static void
in_configurationTestParameterTypes(
    in_configuration configuration)
{
    char *str;
    const char *root;

    if (configuration) {
        root = INCF_ROOT(Debugging);
        INCF_SIMPLE_PARAM(Bool, root, Bool);
        INCF_SIMPLE_PARAM(Long, root, Long);
        INCF_SIMPLE_PARAM(ULong, root, ULong);
        INCF_SIMPLE_PARAM(Float, root, Float);
        str = INCF_SIMPLE_PARAM( String, root, String);
        assert(str);
        if (str) {
            os_free(str);
        }
    }
}

#endif /* IN_DEBUG */

static void
in_configurationInitializeConditionals(
    in_configuration configuration)
{
#ifdef IN_TRACING
    in_configurationInitializeTracing(configuration);
#endif /* IN_TRACING */

#ifdef IN_DEBUGGING
    in_configurationInitializeLossiness(configuration);
    in_configurationInitializeNoPacking(configuration);
    in_configurationTestParameterTypes(configuration);
#endif /* IN_DEBUGGING */
#ifdef IN_PROFILING
        in_configurationInitializeProfiling(configuration);
#endif /* IN_PROFILING */

    if (configuration) {
        /* Verbosity */
        c_char * report_level;
        report_level = INCF_DEFAULTED_PARAM(String, INCF_ROOT(Reporting), Level, "None");
        configuration->reporting.verbosity = LookupReportLevel(report_level);
        os_free(report_level);

        configuration->reporting.events = INCF_DEFAULTED_PARAM( Bool, INCF_ROOT(Reporting),
                                                              Events, FALSE);
        configuration->reporting.periodic = INCF_DEFAULTED_PARAM( Bool, INCF_ROOT(Reporting),
                                                                Periodic, FALSE);
        configuration->reporting.oneShot = INCF_DEFAULTED_PARAM( Bool, INCF_ROOT(Reporting),
                                                               OneShot, FALSE);
    }
}

#ifdef IN_TRACING

static c_bool
in_configurationInterested(
    in_configuration configuration,
    in_traceClass traceClass,
    c_ulong level)
{
    c_bool result = FALSE;

    if (configuration) {
        result = (c_bool)(configuration->traceConfig.levels[traceClass] >= level);
    }

    return result;
}

#endif /* IN_TRACING */

#ifdef IN_DEBUGGING
c_bool
in_configurationLoseSentMessage(
    void)
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();

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
in_configurationLoseReceivedMessage(
    void)
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();

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

c_bool
in_configurationNoPacking(
    void)
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->noPacking) {
        result = TRUE;
    }

    return result;
}
#endif

/* Helper functions for retrieving parameter values */

static c_bool
in_configurationGetElementInternal(
    u_cfElement element,
    const c_char *elementPath,
    const char *elementName,
    u_cfElement *resultElement)
{
    u_cfElement helperElement;
    c_iter elementList;
    int fullNameSize;
    c_char *elementFullName;

    *resultElement = NULL;

    if (element!= NULL && elementName != NULL ) {
        /* First build the complete name of the element */
        /* Note that the 0-terminator is included in the sizeof */
        if (elementPath != NULL) {
            fullNameSize = strlen(elementPath) + strlen(INCF_SEP) + strlen(elementName) + 1;
        } else {
            fullNameSize = strlen(elementName) + 1;
        }
        elementFullName = os_malloc(fullNameSize);
        if (elementFullName != NULL) {
            if (elementPath != NULL) {
                os_sprintf(elementFullName, "%s%s%s", elementPath, INCF_SEP, elementName);
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
in_configurationGetElementDataInternal(
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
in_configurationGetParameterData(
    const c_char *parameterPath,
    const c_char *parameterName)
{
    u_cfData result = NULL;
    in_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->networkingElement) {
        found = in_configurationGetElementInternal(
                    configuration->networkingElement,
                    parameterPath, parameterName, &element);
        if (found) {
            result = in_configurationGetElementDataInternal(element);
        }
        u_cfElementFree(element);
    }

    return result;
}

static u_cfAttribute
in_configurationGetParameterAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_bool *elementFound)
{
    u_cfAttribute result = NULL;
    in_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    element = NULL;
    *elementFound = FALSE;
    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->networkingElement) {
        found = in_configurationGetElementInternal(
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

/* in_configuration.h */
c_bool in_configurationIsDiscoveryChannel(u_cfElement channel)
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



#define IN_SERV_NAME_ATTR "%s[@name='%s']"

void
in_configurationInitialize(
    u_service service,
    const char *serviceName,
    const char *URI)
{
    in_configuration configuration;
    u_cfElement topLevelElement;
    char * path;

    configuration = in_configurationGetConfiguration();
    if (configuration) {
        /* Get the networkingelement and store it */
        topLevelElement = u_participantGetConfiguration(u_participant(service));
        /* Get element with tagname NetworkingService and corresponding name attribute*/
        path = os_malloc(strlen(INCF_ROOT_NetworkingService)+strlen(IN_SERV_NAME_ATTR)+strlen(serviceName));
        os_sprintf(path, IN_SERV_NAME_ATTR, INCF_ROOT_NetworkingService, serviceName);
        in_configurationGetElementInternal(topLevelElement, NULL, path, &configuration->networkingElement);
        in_configurationGetElementInternal(topLevelElement, NULL, INCF_ROOT_Domain, &configuration->domainElement);
        os_free(path);
        u_cfElementFree(topLevelElement);
        in_configurationInitializeConditionals(configuration);
    }
}


void
in_configurationFinalize(
    void)
{
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();
    if (configuration) {
#ifdef IN_TRACING
        in_configurationFinalizeTracing(configuration);
#endif /* IN_TRACING */
#ifdef IN_PROFILING
        in_configurationFinalizeProfiling(configuration);
#endif /* IN_PROFILING */
        u_cfElementFree(configuration->networkingElement);
    }
}


/* Commonly used parameter */

c_bool
in_configurationLevelIsInteresting(
    c_ulong level)
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)(configuration->reporting.verbosity >= level);
    }

    return result;
}


#ifdef IN_LOOPBACK

/* Functions for testing purposes only */

c_bool
in_configurationUseLoopback()
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)((int)configuration->useLoopback == TRUE);
    }

    return result;
}


c_bool
in_configurationUseComplementPartitions()
{
    c_bool result = FALSE;
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();
    if (configuration) {
        result = (c_bool)((int)configuration->useComplementPartitions == TRUE);
    }

    return result;
}

#endif /* IN_LOOPBACK */

v_qos in_configurationGetQos(void)
{
    in_configuration configuration;

    configuration = in_configurationGetConfiguration();
    return configuration->qos;
}

/* Generic parameters */

c_bool
in_configurationGetBoolParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_bool defaultValue)
{
    c_bool result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataBoolValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %s",
                       parameterPath, parameterName, (result ? "TRUE" : "FALSE"));
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for boolean parameter %s,  "
                                "switching to default value %s",
                                parameterPath, parameterName, (defaultValue ? "TRUE" : "FALSE"));
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %s",
            parameterPath, parameterName, (defaultValue ? "TRUE" : "FALSE"));
    }

    return result;
}

c_bool
in_configurationGetBoolAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_bool defaultValueNoElmt,
    c_bool defaultValueNoAttr)
{
    c_bool result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        assert(elmtFound);
        success = u_cfAttributeBoolValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %s",
                       parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for boolean attribute %s[@%s],  "
                                "switching to default value %s",
                                parameterPath, attributeName, (defaultValueNoAttr ? "TRUE" : "FALSE"));
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %s",
                parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        } else {
            result = defaultValueNoElmt;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %s",
                parameterPath, attributeName, (result ? "TRUE" : "FALSE"));
        }
    }

    return result;
}


c_long
in_configurationGetLongParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_long defaultValue)
{
    c_long result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataLongValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %d",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long parameter %s/%s,  "
                                "switching to default value %d",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %d",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}


c_long in_configurationGetLongAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_long defaultValueNoElmt,
    c_long defaultValueNoAttr)
{
    c_long result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        assert(elmtFound);
        success = u_cfAttributeLongValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %d",
                       parameterPath, attributeName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %d",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %d",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %d",
                parameterPath, attributeName, result);
        }
    }

    return result;
}


c_ulong
in_configurationGetULongParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_ulong defaultValue)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataULongValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %u",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for unsigned long parameter %s/%s,  "
                                "switching to default value %u",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %u",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_ulong
in_configurationGetSizeParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_ulong defaultValue)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataSizeValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %u",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for unsigned long parameter %s/%s,  "
                                "switching to default value %u",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %u",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_ulong in_configurationGetULongAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_ulong defaultValueNoElmt,
    c_ulong defaultValueNoAttr)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        assert(elmtFound);
        success = u_cfAttributeULongValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %u",
                       parameterPath, attributeName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %u",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %u",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %u",
                parameterPath, attributeName, result);
        }
    }

    return result;
}

c_ulong in_configurationGetSizeAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_ulong defaultValueNoElmt,
    c_ulong defaultValueNoAttr)
{
    c_ulong result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        assert(elmtFound);
        success = u_cfAttributeSizeValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %u",
                       parameterPath, attributeName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %u",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %u",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %u",
                parameterPath, attributeName, result);
        }
    }

    return result;
}

c_float
in_configurationGetFloatParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataFloatValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %f",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for float parameter %s/%s,  "
                                "switching to default value %f",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %f",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

c_float in_configurationGetFloatAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_float defaultValueNoElmt,
    c_float defaultValueNoAttr)
{
    c_float result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        assert(elmtFound);
        success = u_cfAttributeFloatValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %f",
                       parameterPath, attributeName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %f",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = defaultValueNoAttr;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s[@%s], switching to default value %f",
                parameterPath, attributeName, result);
        } else {
            result = defaultValueNoElmt;
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %f",
                parameterPath, attributeName, result);
        }
    }

    return result;
}


/* Do not forget to os_f ree the result after use */
c_string
in_configurationGetStringParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    const c_char *defaultValue)
{
    c_string result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataStringValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %s",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for string parameter %s/%s,  "
                                "switching to default value %s",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = in_stringDup(defaultValue);
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %s",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}

/* Do not forget to os_f ree the result after use */
c_string
in_configurationGetStringAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    const c_char *defaultValueNoElmt,
    const c_char *defaultValueNoAttr)
{
    c_string result;
    c_bool success = FALSE;
    u_cfAttribute attr;
    c_bool elmtFound;

    attr = in_configurationGetParameterAttribute(parameterPath, attributeName, &elmtFound);
    if (attr) {
        success = u_cfAttributeStringValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s/%s, using value %s",
                       parameterPath, attributeName, result );
        } else {
            IN_REPORT_WARNING_3("retrieving configuration attributes",
                                "incorrect format for boolean attribute %s,  "
                                "switching to default value %s",
                                parameterPath, attributeName, defaultValueNoAttr);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        if (elmtFound) {
            result = in_stringDup(defaultValueNoAttr);
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve attribute %s/%s, switching to default value %s",
                parameterPath, attributeName, result);
        } else {
            result = in_stringDup(defaultValueNoElmt);
            IN_TRACE_3(Configuration, 2,
                "Could not retrieve element %s for attribute %s, switching to default value %s",
                parameterPath, attributeName, result);
        }
    }

    return result;
}


OS_STRUCT(in_nameList) {
    c_ulong nofNames;
    c_char **names;
};

c_bool
in_configurationElementHasChildElementWithName(
    const c_char* path,
    const c_char* tagName)
{
    in_configuration configuration;
    c_iter elementList = NULL;
    c_iter childElementList = NULL;
    c_object element;
    c_bool found = FALSE;

    assert(path != NULL);
    assert(tagName != NULL);

    configuration = in_configurationGetConfiguration();
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
in_configurationGetElements(
    const c_char* path)
{
    in_configuration configuration;
    c_iter elementList = NULL;
    c_iter elementListOrdered = NULL;
    c_object element;

    assert(path != NULL);
    configuration = in_configurationGetConfiguration();
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

in_nameList
in_configurationGetChildElementPaths(
    const c_char *parameterPath,
    const c_char *attribName)
{
    in_nameList result = NULL;
    c_iter elementList;
    u_cfElement element;
    in_configuration configuration;
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

    assert(parameterPath != NULL);
    assert(attribName != NULL);

    configuration = in_configurationGetConfiguration();

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
                assert(element != NULL);
                attrib = u_cfElementAttribute(element, attribName);
                if (attrib != NULL) {
                    attribRes = u_cfAttributeStringValue(attrib, &attribValue);
                    assert(attribRes);
#define IN_PATH_FORMAT "%s[@%s='%s']"
#define IN_PATH_STRING "[@='']"
                    pathSize = strlen(parameterPath) +
                        strlen(attribName) + strlen(attribValue) +
                        sizeof(IN_PATH_STRING);
                    fullPath = os_malloc(pathSize);
                    sRes = snprintf(fullPath, pathSize, IN_PATH_FORMAT,
                        parameterPath, attribName, attribValue);
                    assert(sRes+1 == pathSize);
#undef IN_PATH_FORMAT
#undef IN_PATH_STRING
                    result->names[nofElementsFound] = fullPath;
                    nofElementsFound++;
                    os_free(attribValue);
                    u_cfAttributeFree(attrib);
                } else {
                    error = TRUE;
                    IN_REPORT_WARNING_2("parsing configuration",
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
in_nameListGetCount(
    in_nameList nameList)
{
    int result = 0;

    if (nameList != NULL) {
        result = nameList->nofNames;
    }

    return result;
}

const c_char *
in_nameListGetName(
    in_nameList nameList,
    c_ulong index)
{
    const char *result = NULL;

    if ((nameList != NULL) && (nameList->nofNames > index)) {
        result = nameList->names[index];
    }

    return result;
}


void
in_nameListFree(
    in_nameList nameList)
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

static unsigned int
LookupReportLevel(
    const c_char * report_level)
{
    unsigned int result = 0;
    unsigned int i;

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

/* ------------------------------- Profiling -------------------------------- */

/* implements in_profiling.h */

#ifdef IN_PROFILING

void in_profilingLapStart(
    in_profilingClass profilingClass)
{
    in_configuration configuration;
    struct in_lapAdmin *lapAdmin;


    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->profilingConfig.doProfiling) {
        lapAdmin = &(configuration->profilingConfig.lapAdmins[profilingClass]);
        if (lapAdmin->doProfiling) {
            if (lapAdmin->running) {
                IN_REPORT_ERROR_1("in_profilingLapStart",
                    "Laptimer %s is already running",
                    in_profClassName(profilingClass));

            } else {
                c_laptimeStart(lapAdmin->lapTimer);
                lapAdmin->running = TRUE;
            }
        }
    }
}

void in_profilingLapStop(
    in_profilingClass profilingClass)
{
    in_configuration configuration;
    struct in_lapAdmin *lapAdmin;

    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->profilingConfig.doProfiling) {
        lapAdmin = &(configuration->profilingConfig.lapAdmins[profilingClass]);
        if (lapAdmin->doProfiling) {
            if (!(int)lapAdmin->running) {
                IN_REPORT_ERROR_1("in_profilingLapStop",
                    "Laptimer %s is not running",
                    in_profClassName(profilingClass));

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


#endif /* IN_PROFILING */


/* ----------------------------- Domain parameters -------------------------- */

static u_cfData
in_configurationGetDomainParameterData(
    const c_char *parameterPath,
    const c_char *parameterName)
{
    u_cfData result = NULL;
    in_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->domainElement) {
        found = in_configurationGetElementInternal(
                    configuration->domainElement,
                    parameterPath, parameterName, &element);
        if (found) {
            result = in_configurationGetElementDataInternal(element);
        }
        u_cfElementFree(element);
    }

    return result;
}


static c_float
in_configurationGetDomainFloatParameter(
    const c_char *parameterPath,
    const c_char *parameterName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfData data;

    data = in_configurationGetDomainParameterData(parameterPath, parameterName);
    if (data) {
        success = u_cfDataFloatValue(data, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved parameter %s/%s, using value %f",
                       parameterPath, parameterName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for float parameter %s/%s,  "
                                "switching to default value %f",
                                parameterPath, parameterName, defaultValue);
        }
        u_cfDataFree(data);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve parameter %s/%s, switching to default value %f",
            parameterPath, parameterName, defaultValue);
    }

    return result;
}




static u_cfAttribute
in_configurationGetDomainParameterAttribute(
    const c_char *parameterPath,
    const c_char *attributeName)
{
    u_cfAttribute result = NULL;
    in_configuration configuration;
    u_cfElement element;
    c_bool found = FALSE;

    element = NULL;
    configuration = in_configurationGetConfiguration();

    if (configuration && configuration->domainElement) {
        found = in_configurationGetElementInternal(
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
in_configurationGetDomainFloatAttribute(
    const c_char *parameterPath,
    const c_char *attributeName,
    c_float defaultValue)
{
    c_float result;
    c_bool success = FALSE;
    u_cfAttribute attr;

    attr = in_configurationGetDomainParameterAttribute(parameterPath, attributeName);
    if (attr) {
        success = u_cfAttributeFloatValue(attr, &result);
        if (success) {
            IN_TRACE_3(Configuration, 1,
                       "Retrieved attribute %s[@%s], using value %f",
                       parameterPath, attributeName, result);
        } else {
            IN_REPORT_WARNING_3("retrieving configuration parameters",
                                "incorrect format for long attribute %s[@%s],  "
                                "switching to default value %f",
                                parameterPath, attributeName, defaultValue);
        }
        u_cfAttributeFree(attr);
    }

    if (!success) {
        result = defaultValue;
        IN_TRACE_3(Configuration, 2,
            "Could not retrieve attribute %s[@%s], switching to default value %f",
            parameterPath, attributeName, defaultValue);
    }

    return result;
}



#define LEASE_PATH         "Lease"
#define EXPIRYTIME_NAME    "ExpiryTime"
#define UPDATEFACTOR_NAME  "update_factor"

#define DEF_EXPIRYTIME     (10.0F)
#define MIN_EXPIRYTIME     ( 0.2F)

#define DEF_UPDATEFACTOR   (0.10F)
#define MIN_UPDATEFACTOR   (0.05F)
#define MAX_UPDATEFACTOR   (0.90F)

static c_float
in_configurationGetUpdateFactor()
{
    c_float result;

    result = in_configurationGetDomainFloatAttribute(LEASE_PATH "/" EXPIRYTIME_NAME, UPDATEFACTOR_NAME, DEF_UPDATEFACTOR);
    if (result < MIN_UPDATEFACTOR) {
        result = MIN_UPDATEFACTOR;
    }
    if (result > MAX_UPDATEFACTOR) {
        result = MAX_UPDATEFACTOR;
    }

    return result;
}


c_float
in_configurationGetDomainLeaseExpiryTime()
{
    c_float result;

    result =  in_configurationGetDomainFloatParameter(LEASE_PATH, EXPIRYTIME_NAME, DEF_EXPIRYTIME);
    if (result < MIN_EXPIRYTIME) {
        result = MIN_EXPIRYTIME;
    }
    return result;
}

c_float
in_configurationGetDomainLeaseUpdateTime()
{
   c_float result;
   c_float updateFactor;
   c_float expiryTime;


   updateFactor = in_configurationGetUpdateFactor();
   expiryTime = in_configurationGetDomainLeaseExpiryTime();
   result = updateFactor * expiryTime;

   return result;
}

