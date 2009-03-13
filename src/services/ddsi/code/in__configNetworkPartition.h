/* Prevent failure due to multiple inclusion of this file. */
#ifndef IN_CONFIG_NETWORK_PARTITION_H
#define IN_CONFIG_NETWORK_PARTITION_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"
#include "in__configTypes.h"

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
#define in_configNetworkPartition(_this) ((in_configNetworkPartition)_this)

in_configNetworkPartition
in_configNetworkPartitionNew(
    os_char* name,
    os_char* address,
    os_boolean isConnected);

os_char*
in_configNetworkPartitionGetName(
    in_configNetworkPartition _this);

os_char*
in_configNetworkPartitionGetAddress(
    in_configNetworkPartition _this);

os_char*
in_configNetworkPartitionGetPathName(
    in_configNetworkPartition _this);

os_boolean
in_configNetworkPartitionIsConnected(
    in_configNetworkPartition _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_NETWORK_PARTITION_H */
