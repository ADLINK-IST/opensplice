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
#ifndef IN_CONFIG_TRACING_H
#define IN_CONFIG_TRACING_H

/* OS abstraction includes. */
#include "in__object.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif



/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_configTracing(_this) ((in_configTracing)_this)

in_configTracing
in_configTracingNew(
    );

void
in_configTracingFree(
    in_configTracing _this);

os_char*
in_configTracingGetPathName(
    in_configTracing _this);

os_boolean
in_configTracingIsEnabled(
    in_configTracing _this);

os_char*
in_configTracingGetOutputFileName(
    in_configTracing _this);

os_uint32
in_configTracingGetInitLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetDeinitLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetMainloopLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetGroupsLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetWritingLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetReadingLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetTestLevel(
    in_configTracing _this);

os_uint32
in_configTracingGetDiscoveryLevel(
    in_configTracing _this);

in_configTimestamps
in_configTracingGetTimeStamps(
    in_configTracing _this);

void
in_configTracingSetDefaultLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetConfigurationLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetInitLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetDeinitLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetMainloopLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetGroupsLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetWritingLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetReadingLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetTestLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetDiscoveryLevel(
    in_configTracing _this,
    os_uint32 level);

void
in_configTracingSetOutputFile(
    in_configTracing _this,
    os_char* outputFileName);

void
in_configTracingOpenOutputFile(
    in_configTracing _this);

void
in_configTracingSetEnabled(
    in_configTracing _this,
    os_boolean isEnabled);

#define TC(name) TC_##name

typedef enum in_traceClass_e {
  TC(Undefined),
  TC(Configuration),
  TC(Construction),
  TC(Destruction),
  TC(Mainloop),
  TC(Groups),
  TC(Send),
  TC(Receive),
  TC(Discovery),
  TC(Test),
  TC(Connectivity),
  TC(Count)/* Last element, keep this at the end */
} in_traceClass;

void
in_configTracingReport(
    in_traceClass traceClass,
    c_ulong level,
    const c_char *context,
    const char *description, ...);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_TRACING_H */
