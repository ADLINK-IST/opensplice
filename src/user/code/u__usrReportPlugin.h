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

#ifndef U__USRREPORTPLUGIN_H
#define U__USRREPORTPLUGIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "cf_element.h"

os_int32
u_usrReportPluginReadAndRegister (
    cf_element config
    );

void
u_usrReportPluginUnregister ();

#if defined (__cplusplus)
}
#endif

#endif /* U__USRREPORTPLUGIN_H */
