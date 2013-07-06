/* $Id: appender_type_rollingfile.h
 *
 * appender_type_rollingfile.h
 * 
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_appender_type_rollingfile_h
#define log4c_appender_type_rollingfile_h

/**
 * @file appender_type_rollingfile.h
 *
 * @brief Log4c rolling file appender interface.
 *
 * The rolling file appender implements a logging mechanism of
 * a list of files up to a maximum number.
 *
 * The files are written by default to the current directory with logging
 * names folowing the pattern log.1, log.2 etc.  These parameters may
 * be changed using the appropriate setter functions.
 *
 * If the appender fails to open logfiles for writing then the
 * messages are logged to stderr--it will continue to try to open
 * the zero-th file for writing at rollover events so if it succeeds
 * at some point to open that file the messages will start to appear therein
 * and will no longer be sent to stderr.
 *
 * Switching from logging from one file
 * to the next is referred to as a 'rollover event'.
 *
 * The policy that determines when a rollover event should happen is
 * called a 'rolling policy'.
 *
 * A mechanism is provided to allow different rolling policies to be defined.
 *
 * Log4c ships with (and defaults to) the classic size-window rollover policy:
 * this triggers rollover when files reach a maximum size.  The first file in
 * the list is
 * always the current file; when a rollover event occurs files are shifted up
 * by one position in the list--if the number of files in the list has already
 * reached the max then the oldest file is rotated out of the window.
 *
 * See the documentation in the rollingpolicy_type_sizewin.h file for
 * more details on the size-win rollover policy.
 *
*/

#include <log4c/defs.h>
#include <log4c/appender.h>
#include <log4c/rollingpolicy.h>

__LOG4C_BEGIN_DECLS

/**
 * rollingfile appender type definition.
 *
 * This should be used as a parameter to the log4c_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
LOG4C_API const log4c_appender_type_t log4c_appender_type_rollingfile;

/**
 * Get a new rolling file appender configuration object.
 * @return a new rolling file appender configuration object, otherwise NULL.
*/
LOG4C_API rollingfile_udata_t *rollingfile_make_udata(void);

/**
 * Set the logging directory in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @param logdir the logging directory to set.
 * @return zero if successful, non-zero otherwise.
 */
LOG4C_API int rollingfile_udata_set_logdir(
                rollingfile_udata_t *rfudatap,
                char* logdir);
/**
 * Set the prefix string in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @param prefix the logging files prfix to use.
 * @return zero if successful, non-zero otherwise.
 */                            
LOG4C_API int rollingfile_udata_set_files_prefix(
              rollingfile_udata_t *rfudatap, char * prefix);
/**
 * Set the rolling policy in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @param policyp the logging files prfix to use.
 * @return zero if successful, non-zero otherwise.
 */                          
LOG4C_API int rollingfile_udata_set_policy(rollingfile_udata_t* rfudatap,
				      log4c_rollingpolicy_t* policyp);                       
/**
 * Get the logging directory in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @return the logging directory.
 */                              
LOG4C_API const char* rollingfile_udata_get_logdir(rollingfile_udata_t* rfudatap);
              
/**
 * Get the prefix string in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @return the prefix.
 */ 
LOG4C_API const char* rollingfile_udata_get_files_prefix(
                            rollingfile_udata_t* rfudatap);

/**
 * Get the prefix string in this rolling file appender configuration.
 * @param rfudatap the rolling file appender configuration object.
 * @return the current size of the file being logged to.
 */ 
LOG4C_API long  rollingfile_get_current_file_size( rollingfile_udata_t* rfudatap);

__LOG4C_END_DECLS

#endif
