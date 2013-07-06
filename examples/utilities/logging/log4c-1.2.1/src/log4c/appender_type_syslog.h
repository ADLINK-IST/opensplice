/* $Id$
 *
 * appender_type_syslog.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_type_syslog_h
#define log4c_appender_type_syslog_h

/**
 * @file appender_type_syslog.h
 *
 * @brief Log4c syslog(3) appender interface.
 *
 * The syslog appender uses the syslog(3) interface for logging. The log4c
 * priorities are mapped to the syslog priorities and the appender name is
 * used as a syslog identifier. 1 default syslog appender is defined: @c
 * "syslog".
 *
 * The following examples shows how to define and use syslog appenders.
 * 
 * @code
 *
 * log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("myappender");
 * log4c_appender_set_type(myappender, &log4c_appender_type_syslog);
 * 
 * @endcode
 *
 **/

#include <log4c/defs.h>
#include <log4c/appender.h>

__LOG4C_BEGIN_DECLS

/**
 * Syslog appender type definition.
 *
 * This should be used as a parameter to the log4c_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log4c_appender_type_t log4c_appender_type_syslog;

__LOG4C_END_DECLS

#endif
