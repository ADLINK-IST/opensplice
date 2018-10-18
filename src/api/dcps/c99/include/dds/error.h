#ifndef DDS_ERROR_H
#define DDS_ERROR_H

/** @file error.h
 *  @brief Vortex Lite Error defines header
 */

#include <stdbool.h>
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#undef DDS_EXPORT
#ifdef OSPL_BUILD_DCPSC99
#define DDS_EXPORT OS_API_EXPORT
#else
#define DDS_EXPORT OS_API_IMPORT
#endif

/* Error masks for returned status values */

#define DDS_ERR_NO_MASK    0x000000ff
#define DDS_ERR_MOD_MASK   0x0000ff00
#define DDS_ERR_MINOR_MASK 0x00ff0000

/* State is unchanged following a function call returning an error
 * other than UNSPECIFIED, OUT_OF_RESOURCES and ALREADY_DELETED.
 *
 * Error handling functions. Three components to returned int status value.
 *
 * 1 - The DDS_ERR_xxx error number
 * 2 - A module identifier
 * 3 - A minor number
 *
 * The minor number allows every place in the implementation code to
 * use a uniue minor code so errors can be traced exactly.
 *
 * All functions return >= 0 on success, < 0 on error
 */
/** @name Return codes
  @{**/
#define DDS_RETCODE_OK                   0 /**< Success */
#define DDS_RETCODE_ERROR                1 /**< Non specific error */
#define DDS_RETCODE_UNSUPPORTED          2 /**< Feature unsupported */
#define DDS_RETCODE_BAD_PARAMETER        3 /**< Bad parameter value */
#define DDS_RETCODE_PRECONDITION_NOT_MET 4 /**< Precondition for operation not met */
#define DDS_RETCODE_OUT_OF_RESOURCES     5 /**< When an operation fails because of a lack of resources */
#define DDS_RETCODE_NOT_ENABLED          6 /**< When a configurable feature is not enabled */
#define DDS_RETCODE_IMMUTABLE_POLICY     7 /**< When an attempt is made to modify an immutable policy */
#define DDS_RETCODE_INCONSISTENT_POLICY  8 /**< When a policy is used with inconsistent values */
#define DDS_RETCODE_ALREADY_DELETED      9 /**< When an attempt is made to delete something more than once */
#define DDS_RETCODE_TIMEOUT              10 /**< When a timeout has occurred */
#define DDS_RETCODE_NO_DATA              11 /*< When expected data is not provided */
#define DDS_RETCODE_ILLEGAL_OPERATION    12 /*< When a function is called when it should not be */
/** @}*/

/* For backwards compatability */

#define DDS_SUCCESS DDS_RETCODE_OK

/** @name DDS_Error_Type
  @{**/
#define DDS_CHECK_REPORT 0x01
#define DDS_CHECK_FAIL 0x02
#define DDS_CHECK_EXIT 0x04
/** @}*/

/* Error code handling functions */

/** @name Macros for error handling
  @{**/
#define DDS_TO_STRING(n) #n
#define DDS_INT_TO_STRING(n) DDS_TO_STRING(n)
/** @}*/

/** Macro to extract error number */
#define dds_err_no(e) ((-e) & DDS_ERR_NO_MASK)

/** Macro to extract the error minor number */
#define dds_err_minor(e) (((-e) & DDS_ERR_MINOR_MASK) >> 16)

/**
 * Description : This operation takes the error value and outputs a string
 * corresponding to it.
 *
 * Arguments :
 *   -# err Error value to be converted to a string
 *   -# Returns a string corresponding to the error value
 */
DDS_EXPORT const char * dds_err_str (int err);

/**
 * Description : This operation takes the error value and returns the module name
 * corresponding to it.
 *
 * Arguments :
 *   -# err Error value
 *   -# Returns the module name corresponding to the value
 */
DDS_EXPORT const char * dds_err_mod_str (int err);

/**
 * Description : This operation takes the error number, error type and filename and line number and formats it to
 * a string which can be used for debugging.
 *
 * Arguments :
 *   -# err Error number
 *   -# flags that indicates Fail, Exit or Report
 *   -# where file and line number
 */

DDS_EXPORT bool dds_err_check (int err, unsigned flags, const char * where);

/**
 * Macro that defines dds_err_check function
 */
#define DDS_ERR_CHECK(e, f) (dds_err_check ((e), (f), __FILE__ ":" DDS_INT_TO_STRING(__LINE__)))

/* Failure handling */

/**
 * Failure handler
 */
typedef void (*dds_fail_fn) (const char *, const char *);

/**
 * Macro that defines dds_fail function
 */
#define DDS_FAIL(m) (dds_fail (m, __FILE__ ":" DDS_INT_TO_STRING (__LINE__)))

/**
 * Description : Set the failure function
 *
 * Arguments :
 *   -# fn The pointer to the failure function
 */
DDS_EXPORT void dds_fail_set (dds_fail_fn fn);

/**
 * Description : Get the failure function
 *
 * Arguments :
 *   -# Returns the failure function set
 */
DDS_EXPORT dds_fail_fn dds_fail_get (void);

/**
 * Description : This operation handles failure through an installed failure handler
 *
 * Arguments :
 *   -# msg The pointer to the failure message
 *   -# where The pointer to the file and location
 */
DDS_EXPORT void dds_fail (const char * msg, const char * where);

#undef DDS_EXPORT

#if defined (__cplusplus)
}
#endif
#endif
