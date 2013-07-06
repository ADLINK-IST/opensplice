/* 
 *
 * appender_type_stream.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_type_stream2_h
#define log4c_appender_type_stream2_h

/**
 * @file appender_type_stream2.h
 *
 * @brief Log4c stream2 appender interface.
 *
 * The stream2 appender uses a file handle @c FILE* for logging.
 * It can be used with @c stdout, @c stderr or a normal file.
 * It is pretty primitive as it does not do file rotation, or have a maximum
 * configurable file size etc. It improves on the stream appender in a few
 * ways that make it a better starting point for new stream based appenders.
 *
 * It enhances the stream appender by allowing
 * the default file pointer to be used in buffered or unbuffered mode.
 * Also when you set the file pointer stream2 will not attempt to close
 * it on exit which avoids it fighting with the owner of the file pointer.
 * stream2 is configured via setter functions--the udata is
 * not exposed directly.  This means that new options (eg. configure the open
 * mode ) could be added to stream2 while maintaining backward compatability.
 *
 * The appender can be used with default values, for example as follows:
 *
 * @code
 * log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("/var/logs/mylog.log");
 * log4c_appender_set_type(myappender,log4c_appender_type_get("stream2"));
 *
 * @endcode
 *
 * In this case the appender will  be configured automatically with default 
 * values: 
 * @li the filename is the same as the name of the appender,
 * @c "/var/logs/mymlog.log"
 * @li the file is opened in "w+" mode
 * @li the default system buffer is used (cf; @c setbuf() ) in buffered mode
 *
 * The stream2 appender can be configured by passing it a file pointer
 * to use.  In this case you manage the file pointer yourself--open,
 * option setting, closing.  If you set the file pointer log4c will
 * not close the file on exiting--you must do this:
 * 
 * @code
 * log4c_appender_t* myappender;
 * FILE * fp = fopen("myfile.log", "w");
 *
 * myappender = log4c_appender_get("myappender");
 * log4c_appender_set_type(myappender, log4c_appender_type_get("stream2"));
 * log4c_stream2_set_fp(stream2_appender,myfp);
 *
 * @endcode
 *
 * The default file pointer can be configured to use unbuffered mode.
 * Buffered mode is typically 25%-50% faster than unbuffered mode but
 * unbuffered mode is useful if your preference is for a more synchronized 
 * log file:
 *
 * @code log4c_appender_t* myappender;
 *
 * myappender = log4c_appender_get("/var/logs/mylog.log");
 * log4c_appender_set_type(myappender,log4c_appender_type_get("stream2"));
 * log4c_stream2_set_flags(myappender, LOG4C_STREAM2_UNBUFFERED);
 *
 * @endcode
 *
 **/

#include <log4c/defs.h>
#include <log4c/appender.h>

__LOG4C_BEGIN_DECLS

/**
 * Stream2 appender type definition.
 *
 * This should be used as a parameter to the log4c_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
LOG4C_API const log4c_appender_type_t log4c_appender_type_stream2;

/**
 * Set the file pointer for this appender.
 * @param this a pointer to the appender
 * @param fp the file pointer this appender will use.  The caller is
 * responsible for managing the file pointer (open, option setting, closing).
 */      
LOG4C_API void log4c_stream2_set_fp(log4c_appender_t* a_this, FILE *fp);

/**
 * Get the file pointer for this appender.
 * @param this a pointer to the appender
 * @return the file pointer for this appender.  If there's a problem
 * returns NULL.
 * 
 */ 
LOG4C_API FILE * log4c_stream2_get_fp(log4c_appender_t* a_this);


/**
 * Set the flags for this appender.
 * @param this a pointer to the appender
 * @param flags ar teh flags to set. These will overwrite the existing flags.
 * Currently supported flags:  LOG4C_STREAM2_UNBUFFERED
 * 
 */
LOG4C_API void log4c_stream2_set_flags(log4c_appender_t* a_this, int flags);
#define LOG4C_STREAM2_UNBUFFERED 0x01

/**
 * Get the flags for this appender.
 * @param this a pointer to the appender
 * @return the flags for this appender. returns -1 if there was a problem.
 */
LOG4C_API int log4c_stream2_get_flags(log4c_appender_t* a_this);

__LOG4C_END_DECLS

#endif
