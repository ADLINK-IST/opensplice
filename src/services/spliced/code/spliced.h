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
#ifndef SPLICED_H
#define SPLICED_H

#include "s_types.h"
#include "u_user.h"
#include "sr_componentInfo.h"
#include "vortex_os.h"
#include "ut_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_SPLICED
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define spliced(o) ((spliced)o)

/* These defines of exit codes are mirrored in the following files:
 * - src/tools/ospl/tool/include/ospl_exitcodes.h
 * - src/services/spliced/code/spliced.h
 */

#define SPLICED_EXIT_CODE_CONTINUE (-1)

#define SPLICED_EXIT_CODE_OK (0)

#define SPLICED_EXIT_CODE_ALREADY_OPERATIONAL (1)

#define SPLICED_EXIT_CODE_RECOVERABLE_ERROR (3)

#define SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR (33)

#define SPLICED_EXIT_CODE_INAPPLICABLE_CONFIGURATION (34)

#define SPLICED_SHM_OK  (1)
#define SPLICED_SHM_NOK (0)

OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_spliced);

OS_API void
ospl_splicedAtExit(
    void);


s_configuration
splicedGetConfiguration(
    spliced spliceDaemon);

u_spliced
splicedGetService(
    spliced spliceDaemon);

ut_threads
splicedGetThreads(
    spliced spliceDaemon);

const os_char*
splicedGetDomainName(
    spliced spliceDaemon);

u_serviceManager
splicedGetServiceManager(
    spliced spliceDaemon);

s_shmMonitor
splicedGetShmMonitor(
    spliced _this);

void
splicedSignalTerminate(
    spliced spliceDaemon,
    int code,
    int shmClean);

os_boolean
splicedIsDoingSystemHalt(
    spliced spliceDaemon);

sr_componentInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name);

void
splicedRemoveKnownService(
    spliced spliceDaemon,
    const c_char *name);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* SPLICED_H */
