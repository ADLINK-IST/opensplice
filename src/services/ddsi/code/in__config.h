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
#ifndef IN_CONFIG_H
#define IN_CONFIG_H

#include "in__object.h"
#include "in__configDdsiService.h"

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
#define INCF_DEF_GLOBAL_PARTITION              "239.255.0.1"
#define INCF_DEF_INTERFACE                     "first available"
#define INCF_DEF_DATA_CHANNEL_PORT             (7412)
#define INCF_DEF_FRAGMENT_SIZE                 (2600U)/* TODO temp higher default */
#define INCF_MIN_FRAGMENT_SIZE                 (200U)
#define INCF_DEF_DISCOVERY_CHANNEL_PORT        (7400)
#define INCF_DEF_RECEIVE_BUFFER_SIZE           (1000000U)
#define INCF_DEF_DIFFERENTIATED_SERVICES_FIELD (0)
#define INCF_DEF_BROADCAST_EXPR                "broadcast"

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_config(_this) ((in_config)_this)

in_config
in_configGetInstance(
    );

void
in_configFree (
    in_config _this);

in_configTracing
in_configGetConfigTracing (
    );

os_boolean
in_configIsTracingEnabled (
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

Coll_List*
in_configGetDdsiServices(
    in_config _this);

os_char*
in_configGetPathName(
    in_config _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_H */
