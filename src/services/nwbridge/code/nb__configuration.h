/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NB__CONFIGURATION_H_
#define NB__CONFIGURATION_H_

#include "nb__object.h"
#include "nb__log.h"

#include "vortex_os.h"

#include "u_participant.h"

#define NB_MAXTHREADS 30

nb_configuration nb_configurationNew(nb_service service);
#define          nb_configurationFree(s) nb_objectFree(s)
void             nb_configurationPrint(nb_configuration _this);

os_char*    nb_configurationTracingFileName(nb_configuration _this);
nb_logcat   nb_configurationTracingCategories(nb_configuration _this);
c_bool      nb_configurationTracingAppend(nb_configuration _this);

char **     nb_configurationIncludes(nb_configuration _this)
                                                    __nonnull_all__
                                                    __attribute_malloc__
                                                    __attribute_returns_nonnull__;

char **     nb_configurationExcludes(nb_configuration _this)
                                                    __nonnull_all__
                                                    __attribute_malloc__
                                                    __attribute_returns_nonnull__;

os_threadAttr nb_configurationWatchdogSchedAttr(nb_configuration _this);

os_duration nb_configurationLeaseUpdateInterval(nb_configuration _this);
os_duration nb_configurationLeaseExpiryTime(nb_configuration _this);

C_STRUCT(nb_logConfig)
{
    struct {
        FILE *file;
        nb_logcat categories;
    } tracing;
};

void        nb_logConfigInit(nb_logConfig _this) __nonnull_all__;

void        nb_logConfigDeinit(nb_logConfig _this) __nonnull_all__;


#endif /* NB__CONFIGURATION_H_ */
