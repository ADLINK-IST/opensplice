/* Prevent failure due to multiple inclusion of this file. */
#ifndef IN_CONFIG_PARTITION_MAPPING_H
#define IN_CONFIG_PARTITION_MAPPING_H

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
#define in_configPartitionMapping(_this) ((in_configPartitionMapping)_this)

in_configPartitionMapping
in_configPartitionMappingNew(
    os_char* nwPartition,
    os_char* partitionTopicCombo);

os_char*
in_configPartitionMappingGetPathName(
    in_configPartitionMapping _this);

os_char*
in_configPartitionMappingGetNetworkPartitionName(
    in_configPartitionMapping _this);

os_char*
in_configPartitionMappingGetDcpsPartitionTopic(
    in_configPartitionMapping _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_PARTITION_MAPPING_H */
