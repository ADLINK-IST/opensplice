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
#ifndef SPLICED_H
#define SPLICED_H

#include "s_types.h"
#include "u_user.h"
#include "sr_serviceInfo.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define spliced(o) ((spliced)o)

/* These defines of exit codes are mirrored in the following files:
 * - src/tools/ospl/unix/code/ospl.c
 * - src/tools/ospl/win32/code/ospl.c
 * - src/services/spliced/code/spliced.h
 */
#define SPLICED_EXIT_CODE_OK 0

#define SPLICED_EXIT_CODE_RECOVERABLE_ERROR -1

#define SPLICED_EXIT_CODE_UNRECOVERABLE_ERROR -2

s_configuration
splicedGetConfiguration(
    spliced spliceDaemon);

u_spliced
splicedGetService(
    spliced spliceDaemon);

u_serviceManager
splicedGetServiceManager(
    spliced spliceDaemon);

void
splicedDoSystemHalt(
    spliced spliceDaemon,
    int code);

sr_serviceInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name);

#if defined (__cplusplus)
}
#endif

#endif /* SPLICED_H */
