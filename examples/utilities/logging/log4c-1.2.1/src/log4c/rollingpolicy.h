
/*
 * rollingpolicy.h
 *
 * See the COPYING file for the terms of usage and distribution.
*/

#ifndef log4c_rollingpolicy_h
#define log4c_rollingpolicy_h

/**
 * @file rollingpolicy.h
 *
 * @brief Log4c rolling policy interface. Defines the interface for
 * managing and providing rolling policies.
 *
 * A rolling policy is used to confogure a rollingfile appender to tell
 * it when to trigger a rolover event.
*/ 

#include <stdio.h>
#include <log4c/defs.h>
#include <log4c/layout.h>

__LOG4C_BEGIN_DECLS

struct __log4c_rollingpolicy;

/**
 * log4c rollingpolicy type 
 */
typedef struct __log4c_rollingpolicy log4c_rollingpolicy_t;


#define ROLLINGFILE_DEFAULT_LOG_DIR "."
#define ROLLINGFILE_DEFAULT_LOG_PREFIX "log"

typedef struct __rollingfile_udata rollingfile_udata_t; /* opaque */


/**
 * @brief log4c rollingpolicy type.  Defines the interface a specific policy
 * must provide to the rollingfile appender.
 *
 * Attributes description:
 * 
 * @li @c name rollingpolicy type name 
 * @li @c init() init the rollingpolicy
 * @li @c is_triggering_event()
 * @li @c rollover()
 *
 **/
typedef struct log4c_rollingpolicy_type {
  const char*	name;
  int (*init)(log4c_rollingpolicy_t *a_this, rollingfile_udata_t* rfudatap );
  int (*is_triggering_event)( log4c_rollingpolicy_t* a_policy,
			      const log4c_logging_event_t*,
			      long current_file_size );
  int (*rollover)(log4c_rollingpolicy_t* a_policy, FILE **);  
  int (*fini)(log4c_rollingpolicy_t *a_this);
} log4c_rollingpolicy_type_t;

/**
 * Get a new rolling policy
 * @param policy_name a name for the policy
 * @return a new rolling policy, otherwise NULL.
 */ 
LOG4C_API log4c_rollingpolicy_t* log4c_rollingpolicy_get(
                                  const char* policy_name);

/**
 * Use this function to register a rollingpolicy type with log4c.
 * Once this is done you may refer to this type by name both 
 * programmatically and in the log4c configuration file.
 *
 * @param a_type a pointer to the new rollingpolicy type to register.
 * @returns a pointer to the previous rollingpolicy type of same name.
 *
 * Example code fragment: 
 * @code
 * 
 * const log4c_rollingpolicy_type_t log4c_rollingpolicy_type_sizewin = {
 *   "sizewin",
 *   sizewin_init,
 *   sizewin_is_triggering_event,
 *   sizewin_rollover
 * };
 *
 * log4c_rollingpolicy_type_set(&log4c_rollingpolicy_type_sizewin);
 * @endcode
 * 
 */
LOG4C_API const log4c_rollingpolicy_type_t* log4c_rollingpolicy_type_set(
                                    const log4c_rollingpolicy_type_t* a_type);
                                  
/**
 * Configure a rolling policy with a specific policy.
 * @param policyp pointer to the rolling policy
 * @param udatap a specific policy type, for example sizewin.
 * @return zero if successful, non-zero otherwise.
 */                                  
LOG4C_API void log4c_rollingpolicy_set_udata(log4c_rollingpolicy_t* policyp,
					  void *udatap);
/**
 * Call the initialization code of a rolling policy.
 * @param policyp pointer to the rolling policy
 * @param app the rolling appender this policy is used with
 * @return zero if successful, non-zero otherwise.
*/
LOG4C_API int log4c_rollingpolicy_init(log4c_rollingpolicy_t *policyp,
                                       rollingfile_udata_t* rfup );

/**
 * Call the un initialization code of a rolling policy.
 * This will call the fini routine of the particular rollingpolicy type
 * to allow it to free up resources.  If the call to fini in the 
 * rollingpolicy type fails then the rollingpolicy is not uninitialized.
 * Try again later model...
 * @param policyp pointer to the rolling policy
 * @return zero if successful, non-zero otherwise.
*/
LOG4C_API int log4c_rollingpolicy_fini(log4c_rollingpolicy_t *a_this);

/**
 * Determine if a logging event should trigger a rollover according to
 * the given policy.
 * @param policyp pointer to the rolling policy
 * @param evtp the logging event pointer.
 * @param current_file_size the size of the current file being logged to.
 * @return non-zero if rollover required, zero otherwise.
 */ 
LOG4C_API int log4c_rollingpolicy_is_triggering_event(
		     log4c_rollingpolicy_t* policyp,
                     const log4c_logging_event_t* evtp,
		     long current_file_size );
/**
 * Effect a rollover according to policyp on the given file stream.
 * @param policyp pointer to the rolling policy
 * @param fp filestream to rollover.
 * @return zero if successful, non-zero otherwise.
 * The policy can return an indication that something went wrong but
 * that the rollingfile appender can stull go ahead and log by returning an
 * error code <= ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG.  Anything greater than
 * means that the rolling file appender will not try to log it's message.
 */        

#define  ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG 0x05
LOG4C_API int log4c_rollingpolicy_rollover(log4c_rollingpolicy_t* policyp,
                                            FILE ** fp);

/**
 * sets the rolling policy type
 *
 * @param a_rollingpolicy the log4c_rollingpolicy_t object
 * @param a_type the new rollingpolicy type
 * @return the previous appender type
 **/
LOG4C_API const log4c_rollingpolicy_type_t* log4c_rollingpolicy_set_type(
    log4c_rollingpolicy_t* a_rollingpolicy,
    const log4c_rollingpolicy_type_t* a_type);
    
/**
 * Get a pointer to an existing rollingpolicy type.
 *
 * @param a_name the name of the rollingpolicy type to return.  
 * @returns a pointer to an existing rollingpolicy type, or NULL if no 
 * rollingpolicy type with the specified name exists.
 */
LOG4C_API const log4c_rollingpolicy_type_t* log4c_rollingpolicy_type_get(
    const char* a_name);
                                            
/**
 * Get the rolling policy configuration.
 * @param policyp pointer to the rolling policy
 * @return pointer to the rolling policy configuration.
*/                                             
LOG4C_API void* log4c_rollingpolicy_get_udata(
                        const log4c_rollingpolicy_t* policyp);
                        
/**
 * Get the rollingfile appender associated with this policy.
 * @param policyp pointer to the rolling policy
 * @return pointer to the rolling file appender associated with this policy
*/                          
LOG4C_API rollingfile_udata_t* log4c_rollingpolicy_get_rfudata(
                        const log4c_rollingpolicy_t* policyp);
                        
LOG4C_API void* log4c_rollingpolicy_get_name(const log4c_rollingpolicy_t* a_this);                        

LOG4C_API log4c_rollingpolicy_t* log4c_rollingpolicy_new(const char* a_name);
LOG4C_API void log4c_rollingpolicy_delete(log4c_rollingpolicy_t* a_this);
LOG4C_API void log4c_rollingpolicy_print(const log4c_rollingpolicy_t* a_this,
FILE* a_stream);

LOG4C_API int log4c_rollingpolicy_is_initialized(log4c_rollingpolicy_t* a_this);
LOG4C_API void log4c_rollingpolicy_types_print(FILE *fp);
/**
 * @internal
 **/
struct __sd_factory;
LOG4C_API struct __sd_factory* log4c_rollingpolicy_factory;

__LOG4C_END_DECLS
#endif
