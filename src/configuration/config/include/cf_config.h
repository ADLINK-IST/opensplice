/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef CF_CONFIG_H
#define CF_CONFIG_H

#include "cf_node.h"
#include "cf_element.h"
#include "cf_data.h"
#include "cf_attribute.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define CFG_PLATFORM_ENV  "OSPL_PLATFORM_URI"

#define CFG_PLATFORM      "Platform"
#define CFG_DOMAIN        "Domain"
#define CFG_DATABASE      "Database"
#define CFG_NAME          "Name"
#define CFG_SIZE          "Size"
#define CFG_THRESHOLD     "Threshold"
#define CFG_ADDRESS       "Address"
#define CFG_LOCKING       "Locking"
#define CFG_SINGLEPROCESS "SingleProcess"
#define CFG_CPUAFFINITY   "CPUAffinity"
#define CFG_BUILTINTOPICS "BuiltinTopics"
#define CFG_PRIOINHER     "PriorityInheritance"
#define CFG_LEASE         "Lease"
#define CFG_EXPIRYTIME    "ExpiryTime"
#define CFG_TERMPERIOD    "ServiceTerminatePeriod"
#define CFG_ID            "Id"
#define CFG_REPORT        "Report"
#define CFG_MAINTAINOBJECTCOUNT "MaintainObjectCount"

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_CONFIG_H */
