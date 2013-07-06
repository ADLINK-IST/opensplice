/* $Id$
 *
 * buffer.h
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __log4c_buffer_h
#define __log4c_buffer_h

/**
 * @file buffer.h
 *
 * @brief log4c buffer
 *
 **/

#include <log4c/defs.h>
#include <stddef.h>

__LOG4C_BEGIN_DECLS

/**
 * @brief buffer object
 *
 * Attributes description:
 *
 * @li @c size current size of the buffer
 * @li @c maxsize maximum size of the buffer. 0 means no limitation.
 * @li @c data raw data
 **/
typedef struct
{
    size_t buf_size;
    size_t buf_maxsize;
    char*  buf_data;

} log4c_buffer_t;

#define LOG4C_BUFFER_SIZE_DEFAULT  512


__LOG4C_END_DECLS

#endif
