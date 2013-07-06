/* $Id$
 *
 * appender_type_mmap.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_type_mmap_h
#define log4c_appender_type_mmap_h

/**
 * @file appender_type_mmap.h
 *
 * @brief Log4c mmap(2) appender interface.
 *
 * The mmap appender uses a fixed length memory mapped file for
 * logging. The appender's name is used as the file name which will be
 * opened and mapped to memory at first use. The memory mapped file is then
 * used as a rotating buffer in which logging events are written.
 *
 * The following examples shows how to define and use mmap appenders.
 * 
 * @code
 *
 * log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("myfile.log");
 * log4c_appender_set_type(myappender, &log4c_appender_type_mmap);
 * 
 * @endcode
 *
 * @warning the file is not created at first use. It should already exist
 * and have a reasonable size, a mutilple of a page size.
 *
 **/
#include <log4c/defs.h>
#include <log4c/appender.h>

__LOG4C_BEGIN_DECLS

/**
 * Mmap appender type definition.
 *
 * This should be used as a parameter to the log4c_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log4c_appender_type_t log4c_appender_type_mmap;

__LOG4C_END_DECLS

#endif
