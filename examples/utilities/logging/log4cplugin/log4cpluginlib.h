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

#include "LOG4CPLUGIN_Export.h"

#include /* $(OSPL_HOME)/include/sys */ "os_report.h"

LIB4CPLUGIN_Export
int log4c_plugin_init(const char *argument, void **context);

LIB4CPLUGIN_Export
int log4c_plugin_typedreport(void *context, os_reportEvent report);

LIB4CPLUGIN_Export
int log4c_plugin_shutdown(void *context);
