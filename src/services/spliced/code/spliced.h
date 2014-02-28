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
#ifndef SPLICED_H
#define SPLICED_H

#include "s_types.h"
#include "u_user.h"
#include "sr_componentInfo.h"
#include "os.h"

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
 * - src/tools/ospl/unix/code/ospl.c
 * - src/tools/ospl/win32/code/ospl.c
 * - src/services/spliced/code/spliced.h
 */
#define SPLICED_EXIT_CODE_OK 0

#define SPLICED_EXIT_CODE_RECOVERABLE_ERROR 1

#define SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR 2

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

os_char*
splicedGetDomainName(
    void);

u_serviceManager
splicedGetServiceManager(
    spliced spliceDaemon);

void
splicedDoSystemHalt(
    int code);

sr_componentInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name);

void
splicedRemoveKnownService(
    const c_char *name);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* SPLICED_H */
