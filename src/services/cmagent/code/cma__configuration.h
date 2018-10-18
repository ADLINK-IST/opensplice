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
#ifndef CMA__CONFIGURATION_H_
#define CMA__CONFIGURATION_H_

#include "vortex_os.h"

#include "cma__object.h"
#include "cma__log.h"

#include "u_participant.h"

#define CMA_MAXTHREADS 30

cma_configuration
cma_configurationNew(
    cma_service service) __nonnull_all__;

#define cma_configurationFree(s) cma_objectFree(s)

void
cma_configurationPrint(
    cma_configuration _this) __nonnull_all__;

/***** Lease params *****/

os_duration
cma_configurationLeaseUpdateInterval(
    cma_configuration _this) __nonnull_all__;

os_duration
cma_configurationLeaseExpiryTime(
    cma_configuration _this);

/***** Tracing params *****/

os_char*
cma_configurationTracingFileName(
    cma_configuration _this) __nonnull_all__;

cma_logcat
cma_configurationTracingCategories(
    cma_configuration _this) __nonnull_all__;

c_bool
cma_configurationTracingAppend(
    cma_configuration _this) __nonnull_all__;

/********************/

C_STRUCT(cma_logConfig)
{
    struct {
        FILE *file;
        cma_logcat categories;
    } tracing;
};

void
cma_logConfigInit(
    cma_logConfig _this) __nonnull_all__;

void
cma_logConfigDeinit(
    cma_logConfig _this) __nonnull_all__;

#endif /* CMA__CONFIGURATION_H_ */
