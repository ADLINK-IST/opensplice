static const char version[] = "$Id$";

/*
 * appender_stream.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <log4c/appender.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************/
static int stream_open(log4c_appender_t* this)
{
    FILE* fp = log4c_appender_get_udata(this);
    
    if (fp)
	return 0;
    
    if ( (fp = fopen(log4c_appender_get_name(this), "w+")) == NULL)
	fp = stderr;
    
    /* unbuffered mode */
    setbuf(fp, NULL);
    
    log4c_appender_set_udata(this, fp);
    return 0;
}

/*******************************************************************************/
static int stream_append(log4c_appender_t* this, 
			 const log4c_logging_event_t* a_event)
{
    FILE* fp = log4c_appender_get_udata(this);
    
    return fprintf(fp, "[%s] %s", log4c_appender_get_name(this),
		   a_event->evt_rendered_msg);
}

/*******************************************************************************/
static int stream_close(log4c_appender_t* this)
{
    FILE* fp = log4c_appender_get_udata(this);
    

    if (!fp || fp == stdout || fp == stderr)
	return 0;

    return fclose(fp);
}

/*******************************************************************************/
const log4c_appender_type_t log4c_appender_type_stream = {
    "stream",
    stream_open,
    stream_append,
    stream_close,
};

