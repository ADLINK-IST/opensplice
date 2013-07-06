/* $Id$
 *
 * appender.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_h
#define log4c_appender_h

/**
 * @file appender.h
 *
 * @brief Implement this interface for your own strategies for printing log
 * statements.
 *
 * @todo the appender interface needs a better configuration system
 * depending on the layout type. The udata field is a just a trick.
 **/

#include <log4c/defs.h>
#include <log4c/layout.h>
#include <stdio.h>

__LOG4C_BEGIN_DECLS

struct __log4c_appender;

/**
 * log4c appender class 
 **/
typedef struct __log4c_appender log4c_appender_t;

/**
 * @brief log4c appender type class
 *
 * Attributes description:
 * 
 * @li @c name appender type name 
 * @li @c open
 * @li @c append
 * @li @c close
 **/
typedef struct log4c_appender_type {
    const char*	  name;
    int (*open)	  (log4c_appender_t*);
    int (*append) (log4c_appender_t*, const log4c_logging_event_t*);
    int (*close)  (log4c_appender_t*);
} log4c_appender_type_t;

/**
 * Get a pointer to an existing appender type.
 *
 * @param a_name the name of the appender type to return.  
 * @returns a pointer to an existing appender type, or NULL if no appender
 * type with the specified name exists.
 **/
LOG4C_API const log4c_appender_type_t* log4c_appender_type_get(const char* a_name);

/**
 * Use this function to register an appender type with log4c.
 * Once this is done you may refer to this type by name both 
 * programmatically and in the log4c
 * configuration file.
 *
 * @param a_type a pointer to the new appender type to set.
 * @returns a pointer to the previous appender type of same name.
 * 
 * Example code fragment: 
 * @code
 * 
 * const log4c_appender_type_t log4c_appender_type_s13_file = {
 *   "s13_file",
 *   s13_file_open,
 *   s13_file_append,
 *   s13_file_close,
 * };
 *  
 *  log4c_appender_type_set(&log4c_appender_type_s13_file);
 * @endcode
 **/
LOG4C_API const log4c_appender_type_t* log4c_appender_type_set(
    const log4c_appender_type_t* a_type);

/**
 * Get a pointer to an existing appender.
 *
 * @param a_name the name of the appender to return.
 * @returns a pointer to an existing appender, or NULL if no appender
 * with the specfied name exists.
 **/
LOG4C_API log4c_appender_t* log4c_appender_get(const char* a_name);

/**
 * Constructor for log4c_appender_t. 
 **/
LOG4C_API log4c_appender_t* log4c_appender_new(const char* a_name);

/**
 * Destructor for log4c_appender_t.
 **/
LOG4C_API void log4c_appender_delete(log4c_appender_t* a_appender);

/**
 * @param a_appender the log4c_appender_t object
 * @return the appender name
 **/
LOG4C_API const char* log4c_appender_get_name(const log4c_appender_t* a_appender);

/**
 * @param a_appender the log4c_appender_t object
 * @return the appender operations
 **/
LOG4C_API const log4c_appender_type_t* log4c_appender_get_type(
    const log4c_appender_t* a_appender);

/**
 * @param a_appender the log4c_appender_t object
 * @return the appender layout
 **/
LOG4C_API const log4c_layout_t* log4c_appender_get_layout(
    const log4c_appender_t* a_appender);

/**
 * @param a_appender the log4c_appender_t object
 * @return the appender user data
 **/
LOG4C_API void* log4c_appender_get_udata(const log4c_appender_t* a_appender);

/**
 * sets the appender type
 *
 * @param a_appender the log4c_appender_t object
 * @param a_type the new appender type
 * @return the previous appender type
 **/
LOG4C_API const log4c_appender_type_t* log4c_appender_set_type(
    log4c_appender_t* a_appender,
    const log4c_appender_type_t* a_type);

/**
 * sets the appender user data
 *
 * @param a_appender the log4c_appender_t object
 * @param a_udata the new appender user data
 * @return the previous appender user data
 **/
LOG4C_API void* log4c_appender_set_udata(log4c_appender_t*	a_appender, 
				      void* a_udata);

/**
 * sets the appender layout
 *
 * @param a_appender the log4c_appender_t object
 * @param a_layout the new appender layout
 * @return the previous appender layout
 **/
LOG4C_API const log4c_layout_t* log4c_appender_set_layout(
    log4c_appender_t* a_appender,
    const log4c_layout_t* a_layout);

/**
 * opens the appender.
 *
 * @param a_appender the log4c_appender_t object
 **/
LOG4C_API int log4c_appender_open(log4c_appender_t* a_appender);

/**
 * log in appender specific way.
 *
 * @param a_appender the log4c_appender object
 * @param a_event the log4c_logging_event_t object to log.
 **/
LOG4C_API int log4c_appender_append(
    log4c_appender_t* a_appender,
    log4c_logging_event_t* a_event);

/**
 * closes the appender
 *
 * @param a_appender the log4c_appender_t object
 * @return zero if successful, -1 otherwise
 **/
LOG4C_API int log4c_appender_close(log4c_appender_t* a_appender);

/**
 * prints the appender on a stream
 *
 * @param a_appender the log4c_appender_t object
 * @param a_stream the stream
 **/
LOG4C_API void log4c_appender_print(const log4c_appender_t* a_appender, 
				 FILE* a_stream);
     
/**
 * prints all the current registered appender types on a stream
 *
 * @param fp the stream
 **/                            
LOG4C_API void log4c_appender_types_print(FILE *fp);

/**
 * Helper macro to define static appender types.
 *
 * @param a_type the log4c_appender_type_t object to define
 * @warning needs GCC support: otherwise this macro does nothing
 * @deprecated This macro, and the static initialialization
 * of appenders in general, is deprecated. Use rather
 * the log4c_appender_type_set() function to initialize your appenders
 * before calling log4c_init() 
 *
 **/
#ifdef __GNUC__
#   define log4c_appender_type_define(a_type) \
    typedef int log4c_appender_type_define_##a_type __attribute__((deprecated)); \
    static log4c_appender_type_define_##a_type __unsused_var __attribute__ ((unused));
#else
#   define log4c_appender_type_define(a_type)
#endif

/**
 * @internal
 **/
struct __sd_factory;
LOG4C_API struct __sd_factory* log4c_appender_factory;

__LOG4C_END_DECLS

#endif
