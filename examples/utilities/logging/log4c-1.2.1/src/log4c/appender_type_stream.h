/* $Id$
 *
 * appender_type_stream.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_type_stream_h
#define log4c_appender_type_stream_h

/**
 * @file appender_type_stream.h
 *
 * @brief Log4c stream appender interface.
 *
 * The stream appender uses a file handle @c FILE* for logging. The
 * appender's name is used as the file name which will be opened at first
 * log. An appender can also be associated to an opened file handle using
 * the log4c_appender_set_udata() method to update the appender user data
 * field. In this last case, the appender name has no meaning. 2 default
 * stream appenders are defined: @c "stdout" and @c "stderr".
 *
 * The following examples shows how to define and use stream appenders.
 * 
 * @li the simple way
 * @code
 *
 * log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("myfile.log");
 * log4c_appender_set_type(myappender, &log4c_appender_type_stream);
 * 
 * @endcode
 *
 * @li the sophisticated way
 * @code
 *
 * log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("myappender");
 *    
 * log4c_appender_set_type(myappender, &log4c_appender_type_stream);
 * log4c_appender_set_udata(myappender, fopen("myfile.log", "w"));
 *
 * @endcode
 *
 **/

#include <log4c/defs.h>
#include <log4c/appender.h>

__LOG4C_BEGIN_DECLS

/**
 * Stream appender type definition.
 *
 * This should be used as a parameter to the log4c_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log4c_appender_type_t log4c_appender_type_stream;

__LOG4C_END_DECLS

#endif
