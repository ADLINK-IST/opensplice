/* Prevent failure due to multiple inclusion of this file. */
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

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_TRACING_H */
