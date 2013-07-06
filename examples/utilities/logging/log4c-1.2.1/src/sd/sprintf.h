/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_sprintf_h
#define __sd_sprintf_h

/**
 * @file sprintf.h
 *
 * @brief Formatted output conversion
 *
 * These functions write the output under the control of a format
 * string that specifies how subsequent arguments (or arguments
 * accessed via the variable-length argument facilities of stdarg(2))
 * are converted for output.
 *
 * They do not write more than \a size bytes, including the trailing
 * \c '\0'.
 *
 * These functions return the number of characters printed (not
 * including the trailing \c `\0' used to end output to strings). They
 * return -1 if the output was truncated due to the @a size limit.
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <sd/defs.h>

__SD_BEGIN_DECLS

/**
 * Same as fprintf(3) with auto-allocation of the resulting buffer,
 * and output directly in a file, not a stream.
 */
extern int sd_fprintf(int fd, const char *fmt, ...);

/**
 * Same as sprintf(3) with auto-allocation of the resulting buffer.
 */
extern char* sd_sprintf(const char* a_fmt, ...);

/**
 * Same as vsprintf(3) with auto-allocation of the resulting buffer.
 */
extern char* sd_vsprintf(const char* a_fmt, va_list a_arg);

#if defined(__osf__)
extern int snprintf(char* str, size_t size, const char* fmt, ...);
extern int vsnprintf(char* str, size_t size, const char* fmt, va_list arg);
#endif

__SD_END_DECLS

#endif
