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
#ifndef IN_CONFIG_NETWORK_PARTITION_H
#define IN_CONFIG_NETWORK_PARTITION_H

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
