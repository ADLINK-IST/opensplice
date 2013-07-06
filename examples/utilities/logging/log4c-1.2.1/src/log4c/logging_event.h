/* $Id$
 *
 * logging_event.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_logging_event_h
#define log4c_logging_event_h

/**
 * @file logging_event.h
 *
 * @brief the internal representation of logging events. 
 * 
 * When a affirmative logging decision is made a log4c_logging_event
 * instance is created. This instance is passed around the different log4c
 * components.
 **/

#include <log4c/defs.h>
#include <log4c/buffer.h>
#include <log4c/location_info.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

__LOG4C_BEGIN_DECLS

struct __log4c_category;

/**
 * @brief logging event object
 * 
 * Attributes description:
 * 
 * @li @c evt_category category name. 
 * @li @c evt_priority priority of logging event.
 * @li @c evt_msg The application supplied message of logging event.
 * @li @c evt_buffer a pre allocated buffer to be used by layouts to
 *        format in a multi-thread environment.
 * @li @c evt_rendered_msg The application supplied message after layout format.
 * @li @c evt_timestamp The number of seconds elapsed since the epoch
 * (1/1/1970 00:00:00 UTC) until logging event was created.
 * @li @c evt_loc The event's location information 
 **/
typedef struct 
{
    const char* evt_category;
    int	evt_priority;
    const char* evt_msg;
    const char* evt_rendered_msg;
    log4c_buffer_t evt_buffer;
/* ok, this is probably not a good way to do it--should define a common type here
and have the base acessor function do the mapping
*/
#ifndef _WIN32
    struct timeval evt_timestamp;
#else
    FILETIME evt_timestamp;
#endif
    const log4c_location_info_t* evt_loc;

} log4c_logging_event_t;

/**
 * Constructor for a logging event.
 *
 * @param a_category the category name
 * @param a_priority the category initial priority
 * @param a_message the message of this event
 *
 * @todo need to handle multi-threading (NDC)
 **/
LOG4C_API log4c_logging_event_t* log4c_logging_event_new(
    const char* a_category,
    int		a_priority,
    const char*	a_message);
/**
 * Destructor for a logging event.
 * @param a_event the logging event object
 **/
LOG4C_API void log4c_logging_event_delete(log4c_logging_event_t* a_event);

__LOG4C_END_DECLS

#endif
