/* $Id$
 *
 * layout_type_basic.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log4c_layout_type_basic_h
#define log4c_layout_type_basic_h

/**
 * @file layout_type_basic.h
 *
 * @brief Implement a basic layout.
 *
 * In @c log4j.PatternLayout conventions, the basic layout has the following
 * conversion pattern: @c "%P %c - %m\n".
 *
 * Where 
 * @li @c "%P" is the priority of the logging event
 * @li @c "%c" is the category of the logging event
 * @li @c "%m" is the application supplied message associated with the
 * logging event
 * 
 **/

#include <log4c/defs.h>
#include <log4c/layout.h>

__LOG4C_BEGIN_DECLS

extern const log4c_layout_type_t log4c_layout_type_basic;

__LOG4C_END_DECLS

#endif
