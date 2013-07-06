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
#include <sd/sd_xplatform.h>

/*******************************************************************************/
static const char* basic_r_format(
    const log4c_layout_t*	  	a_layout,
    const log4c_logging_event_t*	a_event)
{
    int n, i;

    n = snprintf(a_event->evt_buffer.buf_data, a_event->evt_buffer.buf_size,
		 "%-8s %s - %s\n",
		 log4c_priority_to_string(a_event->evt_priority),
		 a_event->evt_category, a_event->evt_msg);

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
const log4c_layout_type_t log4c_layout_type_basic_r = {
    "basic_r",
    basic_r_format,
};

