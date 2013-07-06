static const char version[] = "$Id$";

/*
 * layout.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <log4c/layout.h>
#include <log4c/priority.h>
#include <sd/sprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sd/sd_xplatform.h>

/*******************************************************************************/
static const char* dated_r_format(
    const log4c_layout_t*  	a_layout,
    const log4c_logging_event_t*a_event)
{
    int n, i;
    struct tm	tm;

#ifndef _WIN32
    gmtime_r(&a_event->evt_timestamp.tv_sec, &tm);
     n = snprintf(a_event->evt_buffer.buf_data, a_event->evt_buffer.buf_size,
         "%04d%02d%02d %02d:%02d:%02d.%03ld %-8s %s - %s\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec,
         a_event->evt_timestamp.tv_usec / 1000,
         log4c_priority_to_string(a_event->evt_priority),
         a_event->evt_category, a_event->evt_msg);
#else
    /* xxx Need a CreateMutex/ReleaseMutex or something here
     */

     SYSTEMTIME stime;

        if ( FileTimeToSystemTime(&a_event->evt_timestamp, &stime)){

    n = snprintf(a_event->evt_buffer.buf_data, a_event->evt_buffer.buf_size, "%04d%02d%02d %02d:%02d:%02d.%03ld %-8s %s- %s\n",
             stime.wYear, stime.wMonth , stime.wDay,
             stime.wHour, stime.wMinute, stime.wSecond,
             stime.wMilliseconds,
             log4c_priority_to_string(a_event->evt_priority),
             a_event->evt_category, a_event->evt_msg);
        }

#endif



    if (n >= a_event->evt_buffer.buf_size) {
	/*
	 * append '...' at the end of the message to show it was
	 * trimmed
	 */
	for (i = 0; i < 3; i++)
	    a_event->evt_buffer.buf_data[a_event->evt_buffer.buf_size - 4 + i] = '.';
    }

    return a_event->evt_buffer.buf_data;
}

/*******************************************************************************/
const log4c_layout_type_t log4c_layout_type_dated_r = {
    "dated_r",
    dated_r_format,
};

