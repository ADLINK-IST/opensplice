/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
#define CFG_SYSTEMID      "SystemId"
#define CFG_SYSTEMIDRANGE   "Range"
#define CFG_SYSTEMIDENTROPY "UserEntropy"
#define CFG_LEASE         "Lease"
#define CFG_EXPIRYTIME    "ExpiryTime"
#define CFG_TERMPERIOD    "ServiceTerminatePeriod"
#define CFG_ID            "Id"
#define CFG_REPORT        "Report"
#define CFG_IN_PROC_EXC   "InProcessExceptionHandling"
#define CFG_MAINTAINOBJECTCOUNT "MaintainObjectCount"
#define CFG_Y2038READY    "y2038_ready"
#define CFG_SERVICE       "Service"
#define CFG_COMMAND       "Command"

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_CONFIG_H */
