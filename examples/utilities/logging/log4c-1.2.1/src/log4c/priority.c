static const char version[] = "$Id$";

/*
 * priority.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <string.h>
#include <log4c/priority.h>
#include <sd/sd_xplatform.h>

static const char* const priorities[] = {
    "FATAL", 
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE",
    "NOTSET",
    "UNKNOWN"
};

static const size_t npriorities = sizeof(priorities) / sizeof(priorities[0]);

/*******************************************************************************/
extern const char* log4c_priority_to_string(int a_priority)
{
    a_priority /= 100;
    if ( (a_priority < 0) || (a_priority > 10) ) 
	a_priority = 10;

    return priorities[a_priority];
}    

/*******************************************************************************/
extern int log4c_priority_to_int(const char* a_priority_name)
{
    size_t i; 

    if (a_priority_name) {
	for (i = 0; i < npriorities; i++) {
	    if (!strncasecmp(priorities[i], a_priority_name, strlen(priorities[i])))
		return i * 100;
	}
    }
    
    return LOG4C_PRIORITY_UNKNOWN;
}    

