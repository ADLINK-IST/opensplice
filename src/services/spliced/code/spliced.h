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
#ifndef SPLICED_H
#define SPLICED_H

#include "s_types.h"
#include "u_user.h"
#include "sr_serviceInfo.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define spliced(o) ((spliced)o)

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
splicedTerminate(
    spliced spliceDaemon);

sr_serviceInfo
splicedGetServiceInfo(
    spliced spliceDaemon,
    const c_char *name);

#if defined (__cplusplus)
}
#endif

#endif /* SPLICED_H */
