/* $Id$
 *
 * layout.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_layout_h
#define log4c_layout_h

/**
 * @file layout.h
 *
 * @brief Interface for user specific layout format of log4c_logging_event
 * events. 
 *
 * @todo the layout interface needs a better configuration system
 * depending on the layout type. The udata field is a just a trick.
 *
 * @todo a pattern layout would be welcomed !!
 **/

#include <log4c/defs.h>
#include <log4c/logging_event.h>
#include <stdio.h>

__LOG4C_BEGIN_DECLS

struct __log4c_layout;

/**
 * log4c layout class 
 **/
typedef struct __log4c_layout log4c_layout_t;

/**
 * @brief log4c layout type class
 *
 * Attributes description:
 * 
 * @li @c name layout type name 
 * @li @c format 
 **/
typedef struct log4c_layout_type {
    const char* name;
    const char* (*format) (const log4c_layout_t*, const log4c_logging_event_t*);
} log4c_layout_type_t;

/**
 * Get a pointer to an existing layout type.
 *
 * @param a_name the name of the layout type to return.  
 * @returns a pointer to an existing layout type, or NULL if no layout
 * type with the specified name exists.
 **/
LOG4C_API const log4c_layout_type_t* log4c_layout_type_get(const char* a_name);

/**
 * Use this function to register a layout type with log4c.
 * Once this is done you may refer to this type by name both 
 * programatically and in the log4c configuration file.
 *
 * @param a_type a pointer to the new layout type to set.
 * @returns a pointer to the previous layout type of same name.
 *
 * Example code fragment: 
 * @code
 * 
 * const log4c_layout_type_t log4c_layout_type_xml = {
 *    "s13_xml",
 *    xml_format,
 * };
 *  
 * log4c_layout_type_set(&log4c_layout_type_xml);
 *
 * @endcode
 **/
LOG4C_API const log4c_layout_type_t* log4c_layout_type_set(
    const log4c_layout_type_t* a_type);

/**
 * Get a pointer to an existing layout.
 *
 * @param a_name the name of the layout to return.
 * @returns a pointer to an existing layout, or NULL if no layout
 * with the specfied name exists.
 **/
LOG4C_API log4c_layout_t* log4c_layout_get(const char* a_name);

/**
 * Constructor for layout. 
 **/
LOG4C_API log4c_layout_t* log4c_layout_new(const char* a_name);

/**
 * Destructor for layout.
 **/
LOG4C_API void log4c_layout_delete(log4c_layout_t* a_layout);

/**
 * @param a_layout the log4c_layout_t object
 * @return the layout name
 **/
LOG4C_API const char* log4c_layout_get_name(const log4c_layout_t* a_layout);

/**
 * @param a_layout the log4c_layout_t object
 * @return a log4c_layout_type_t object
 **/
LOG4C_API const log4c_layout_type_t* log4c_layout_get_type(
    const log4c_layout_t* a_layout);

/**
 * sets the layout type
 *
 * @param a_layout the log4c_layout_t object
 * @param a_type the new layout type
 * @return the previous layout type
 *
 **/
LOG4C_API const log4c_layout_type_t* log4c_layout_set_type(
    log4c_layout_t* a_layout,
    const log4c_layout_type_t* a_type);

/**
 * @param a_layout the log4c_layout_t object
 * @return the layout user data
 **/
LOG4C_API void* log4c_layout_get_udata(const log4c_layout_t* a_layout);

/**
 * sets the layout user data
 *
 * @param a_layout the log4c_layout_t object
 * @param a_udata the new layout user data
 * @return the previous layout user data
 **/
LOG4C_API void* log4c_layout_set_udata(log4c_layout_t*	a_layout, 
				    void*		a_udata);
/**
 * format a log4c_logging_event events to a string.
 *
 * @param a_layout the log4c_layout_t object
 * @param a_event a logging_event_t object
 * @returns an appendable string.
 **/
LOG4C_API const char* log4c_layout_format(
    const log4c_layout_t*		a_layout,
    const log4c_logging_event_t*	a_event);

/**
 * prints the layout on a stream
 * @param a_layout the log4c_layout_t object
 * @param a_stream the stream
 **/
LOG4C_API void log4c_layout_print(
    const log4c_layout_t* a_layout, FILE* a_stream);

/**
 * prints all the current registered layout types on a stream
 *
 * @param fp the stream
 **/                            
LOG4C_API void log4c_layout_types_print(FILE *fp);

/**
 * Helper macro to define static layout types.
 *
 * @param a_type the log4c_layout_type_t object to define
 * @warning needs GCC support: otherwise this macro does nothing
 * @deprecated This macro, and the static initialialization
 * of layouts in general, is deprecated. Use rather
 * the log4c_layout_type_set() function to initialize your appenders
 * before calling log4c_init() 
 **/
#ifdef __GNUC__
#   define log4c_layout_type_define(a_type) \
    typedef int log4c_layout_type_define_##a_type __attribute__((deprecated)); \
    static log4c_layout_type_define_##a_type __unsused_var __attribute__((unused));
#else
#   define log4c_layout_type_define(a_type)
#endif

/**
 * @internal
 **/
struct __sd_factory;
LOG4C_API struct __sd_factory* log4c_layout_factory;

__LOG4C_END_DECLS

#endif
