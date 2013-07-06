static const char version[] = "$Id$";

/*
 * appender_stream2.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */
#include <stdio.h>
#include <string.h>

#include <sd/malloc.h>

#include <log4c/appender.h>
#include <log4c/appender_type_stream2.h>

typedef struct stream2_udata {
    FILE * s2u_fp;
    int  s2u_flags;
#define STREAM2_MY_FP 0x01
    int s2u_state;
} log4c_stream2_udata_t;

/* xxx would be nice to run-time check the type here */
#define stream2_get_udata(this) \
 (log4c_stream2_udata_t *)log4c_appender_get_udata(this)

/*******************************************************************************/
static log4c_stream2_udata_t* stream2_make_udata(void);
static void stream2_free_udata(log4c_stream2_udata_t* s2up);
static log4c_stream2_udata_t * stream2_get_or_make_udata(log4c_appender_t* this);
static int stream2_init(log4c_appender_t* this);

/*******************************************************************************/
static int stream2_init(log4c_appender_t* this){
    log4c_stream2_udata_t *s2up = stream2_make_udata();

    log4c_appender_set_udata(this, s2up);

    return(0);
}

/*******************************************************************************/
static int stream2_open(log4c_appender_t* this)
{
    log4c_stream2_udata_t *s2up = NULL;
    FILE * fp = NULL;
    int flags = 0;

    if ( !this){
	return(-1);
    }
    s2up = stream2_get_or_make_udata(this);
    /* s2up = stream2_get_udata(this); */

    fp = s2up->s2u_fp;
    flags = s2up->s2u_flags;

    if ( !fp ) {
	if ( (fp = fopen(log4c_appender_get_name(this), "w+")) == NULL){
	    fp = stderr;
	} else {
	    s2up->s2u_state |= STREAM2_MY_FP;
	}
	s2up->s2u_fp = fp;
    }

    if ( flags &  LOG4C_STREAM2_UNBUFFERED){/* unbuffered mode by default */
	setbuf(fp, NULL);
    }

    return 0;
}

/*******************************************************************************/
static int stream2_append(log4c_appender_t* this,
			 const log4c_logging_event_t* a_event)
{
    log4c_stream2_udata_t *s2up = log4c_appender_get_udata(this);

    if ( !s2up ) {
	return(-1);
    }

    return fprintf(s2up->s2u_fp, "[%s] %s", log4c_appender_get_name(this),
		   a_event->evt_rendered_msg);
}

/*******************************************************************************/
static int stream2_close(log4c_appender_t* this)
{
    log4c_stream2_udata_t *s2up = log4c_appender_get_udata(this);
    int rc = -1;

    if ( !this){
	return rc;
    }
    s2up = stream2_get_udata(this);
    if ( !s2up){
	return(rc);
    }

    if ( s2up->s2u_fp && (s2up->s2u_state & STREAM2_MY_FP) ){
	rc = fclose(s2up->s2u_fp);
    } else {
	rc = 0;
    }
    /* Free up and reset any data associated with this stream2 appender */
    stream2_free_udata(s2up);
    log4c_appender_set_udata(this, NULL);

    return (rc);
}

/*******************************************************************************/

static log4c_stream2_udata_t* stream2_make_udata(){

    log4c_stream2_udata_t* s2up =
	(log4c_stream2_udata_t*) sd_calloc(1, sizeof(log4c_stream2_udata_t));
    return(s2up);
}

/*******************************************************************************/

static void stream2_free_udata(log4c_stream2_udata_t* s2up){

    free(s2up);
}

/*******************************************************************************/

static log4c_stream2_udata_t * stream2_get_or_make_udata(log4c_appender_t* this){
    log4c_stream2_udata_t *s2up;

    s2up = log4c_appender_get_udata(this);

    if ( !s2up) {
        stream2_init(this);
    }

    return(stream2_get_udata(this));
}

/*******************************************************************************/
extern void log4c_stream2_set_fp(log4c_appender_t* this, FILE *fp){
    log4c_stream2_udata_t *s2up;

    if ( !this){
	return;
    }
    s2up = stream2_get_or_make_udata(this);

    s2up->s2u_fp = fp;
    s2up->s2u_state &= !(STREAM2_MY_FP);
}
/*******************************************************************************/
extern FILE* log4c_stream2_get_fp(log4c_appender_t* this){
    log4c_stream2_udata_t *s2up;

    if ( !this){
	return NULL;
    }
    s2up = stream2_get_udata(this);
    if ( s2up){
	return s2up->s2u_fp;
    } else {
	return NULL;
    }
}
/*******************************************************************************/
extern int log4c_stream2_get_flags(log4c_appender_t* this){
    log4c_stream2_udata_t *s2up;

    if ( !this){
	return -1;
    }
    s2up = stream2_get_udata(this);
    if ( s2up){
	return s2up->s2u_flags;
    } else {
	return -1;
    }
}
/*******************************************************************************/
extern void log4c_stream2_set_flags(log4c_appender_t* this, int flags){
    log4c_stream2_udata_t *s2up;

    if ( !this){
	return;
    }
    s2up = stream2_get_or_make_udata(this);
    if ( !s2up){
	return;
    }
    s2up->s2u_flags = flags;
}

/*******************************************************************************/
const log4c_appender_type_t log4c_appender_type_stream2 = {
    "stream2",
    stream2_open,
    stream2_append,
    stream2_close,
};

