static const char version[] = "$Id$";

/*
 * version.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <log4c/version.h>

const int log4c_major_version = LOG4C_MAJOR_VERSION;
const int log4c_minor_version = LOG4C_MINOR_VERSION;
const int log4c_micro_version = LOG4C_MICRO_VERSION;

/*******************************************************************************/
extern const char* log4c_version(void)
{
    return VERSION;
}
