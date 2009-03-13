/* Prevent failure due to multiple inclusion of this file. */
#ifndef IN_CONFIG_TYPES_H
#define IN_CONFIG_TYPES_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

OS_CLASS(in_config);
OS_CLASS(in_configChannel);
OS_CLASS(in_configDataChannel);
OS_CLASS(in_configDdsiService);
OS_CLASS(in_configDebug);
OS_CLASS(in_configDiscoveryChannel);
OS_CLASS(in_configNetworkPartition);
OS_CLASS(in_configPartitioning);
OS_CLASS(in_configPartitionMapping);
OS_CLASS(in_configTimestamps);
OS_CLASS(in_configTracing);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_TYPES_H */
