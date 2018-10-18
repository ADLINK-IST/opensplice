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

#ifndef U__USRREPORTPLUGIN_H
#define U__USRREPORTPLUGIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "cf_element.h"

u_result
u_usrReportPluginReadAndRegister (
    const cf_element config,
    os_int32 domainId,
	c_iter*
    );

void
u_usrReportPluginUnregister (c_iter);

#if defined (__cplusplus)
}
#endif

#endif /* U__USRREPORTPLUGIN_H */
