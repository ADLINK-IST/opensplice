/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/* Prevent failure due to multiple inclusion of this file. */
#ifndef IN_CONFIG_H
#define IN_CONFIG_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"

/* DDSi configuration includes */
#include "in__configDdsiService.h"
#include "in__configTypes.h"

/* DDSI common includes */
#include "in_result.h"

#include "u_service.h"


/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/* Default settings */
#define INCF_DEF_GLOBAL_PARTITON   "239.255.0.1"
#define INCF_DEF_INTERFACE "first available"
#define INCF_DEF_DATA_CHANNEL_PORT  (7412)
#define INCF_DEF_DISCOVERY_CHANNEL_PORT (7400)
#define INCF_DEF_RECEIVE_BUFFER_SIZE (1000000U)
#define INCF_DEF_DIFFERENTIATED_SERVICES_FIELD (0)

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_config(_this) ((in_config)_this)

in_config
in_configGetInstance(
    );

in_result
in_configConvertDomTree(
    in_config _this,
    const os_char* pathname,
    u_service service);

in_configDdsiService
in_configGetDdsiServiceByName(
    in_config _this,
    const os_char* name);

os_char*
in_configGetPathName(
    in_config _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_H */
