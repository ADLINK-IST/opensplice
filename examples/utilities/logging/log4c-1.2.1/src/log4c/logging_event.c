static const char version[] = "$Id$";

/*
 * logging_event.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <log4c/logging_event.h>
#include <log4c/category.h>
#include <stdlib.h>
#include <sd/malloc.h>
#include <sd/sd_xplatform.h>

/*******************************************************************************/
extern log4c_logging_event_t* log4c_logging_event_new(
    const char* a_category,
    int		a_priority,
    const char*	a_message)
{
    log4c_logging_event_t* evt;

    evt 		= sd_calloc(1, sizeof(log4c_logging_event_t));    
    evt->evt_category	= a_category;
    evt->evt_priority	= a_priority;
    evt->evt_msg	= a_message;
    
    SD_GETTIMEOFDAY(&evt->evt_timestamp, NULL);

    return evt;
}

/*******************************************************************************/
extern void log4c_logging_event_delete(log4c_logging_event_t* this)
{
    if (!this)
	return;

    free(this);
}

