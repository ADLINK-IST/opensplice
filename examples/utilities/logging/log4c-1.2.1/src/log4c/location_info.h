/* $Id$
 *
 * location_info.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.


 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_location_info_h
#define log4c_location_info_h

/**
 * @file location_info.h
 *
 * @brief The internal representation of caller location information.
 * 
 * When a affirmative logging decision is made a log4c_location_info_t is
 * created and is passed around the different log4c components.
 **/

#include <log4c/defs.h>

__LOG4C_BEGIN_DECLS

/**
 * @brief logging location information
 *
 * Attributes description:
 * 
 * @li @c loc_file file name
 * @li @c loc_line file line
 * @li @c loc_function function name
 * @li @c loc_data user data
 *
 * @todo this is not used
 **/
typedef struct 
{
    const char* loc_file;
    int loc_line;
    const char* loc_function;
    void* loc_data;

} log4c_location_info_t;

/**
 * log4c_location_info_t initializer 
 **/
#ifdef __GNUC__
#   define LOG4C_LOCATION_INFO_INITIALIZER(user_data) { __FILE__, __LINE__, __FUNCTION__, user_data }
#else
#   define LOG4C_LOCATION_INFO_INITIALIZER(user_data) { __FILE__, __LINE__, "(nil)", user_data }
#endif

#define __log4c_str(n) #n

#ifdef __GNUC__
#   define __log4c_location(n)	__FUNCTION__ "() at " __FILE__ ":" __log4c_str(n)
#else
#   define __log4c_location(n)	__FILE__ ":" __log4c_str(n)
#endif

/**
 * This macro returns the literal representation of a logging event
 * location
 **/
#define log4c_location __log4c_location(__LINE__)

__LOG4C_END_DECLS

#endif
