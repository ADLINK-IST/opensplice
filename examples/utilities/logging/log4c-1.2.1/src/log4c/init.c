static const char version[] = "$Id$";

/*
 * init.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <log4c/init.h>
#include <log4c/category.h>
#include <log4c/appender.h>
#include <log4c/layout.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rc.h>
#include <sd/error.h>
#include <sd/sprintf.h>
#include <sd/factory.h>
#include <sd/sd_xplatform.h>
#include <stdlib.h>

#include <appender_type_stream.h>
#include <appender_type_stream2.h>
#include <appender_type_syslog.h>
#include <appender_type_mmap.h>
#include <appender_type_rollingfile.h>
#include <rollingpolicy_type_sizewin.h>
#include <layout_type_basic.h>
#include <layout_type_dated.h>
#include <layout_type_basic_r.h>
#include <layout_type_dated_r.h>

#if defined(__LOG4C_DEBUG__) && defined(__GLIBC__)
#include <mcheck.h>
#endif

static const log4c_layout_type_t * const layout_types[] = {
    &log4c_layout_type_basic,
    &log4c_layout_type_dated,
    /* These appenders are not yet ported with msvc */
#ifndef _WIN32
    &log4c_layout_type_basic_r,
    &log4c_layout_type_dated_r
#endif
};
static size_t nlayout_types = sizeof(layout_types) / sizeof(layout_types[0]);

static const log4c_appender_type_t * const appender_types[] = {
    &log4c_appender_type_stream,
    &log4c_appender_type_stream2,
#ifdef HAVE_MMAP
    &log4c_appender_type_mmap,
#endif
#ifdef HAVE_SYSLOG_H
    &log4c_appender_type_syslog,
#endif
#ifdef WITH_ROLLINGFILE
    &log4c_appender_type_rollingfile
#endif
};
static size_t nappender_types = sizeof(appender_types) / sizeof(appender_types[0]);

#ifdef WITH_ROLLINGFILE
static const log4c_rollingpolicy_type_t * const rollingpolicy_types[] = {
    &log4c_rollingpolicy_type_sizewin
};
static size_t nrollingpolicy_types =
    sizeof(rollingpolicy_types) / sizeof(rollingpolicy_types[0]);
#endif

static int log4c_is_init = 0;
typedef struct rcfile
{
	char name[256];
	time_t ctime;
	int    exists;
} rcfile_t;

static rcfile_t rcfiles[] = {
	{ "$LOG4C_RCPATH/log4crc" },
	{  "$HOME/.log4crc" },
	{ "./log4crc" }
	};

static const int nrcfiles = sizeof(rcfiles) / sizeof(rcfiles[0]);


/******************************************************************************/
extern int log4c_init(void)
{
    size_t i;
    int ret = 0;

    sd_debug("log4c_init[");

    /* activate GLIBC allocation debugging */
#if defined(__LOG4C_DEBUG__) && defined(__GLIBC__)
    mtrace();
#endif

    if (log4c_is_init){
	sd_debug("log4c already initialized ]");
	return 0;
    }

    log4c_is_init++;

    /* Initialize default types: layouts, appenders, rollingpolicies */
    sd_debug("intializing default types: appenders, layouts, rollingpolicies");
    for (i = 0; i < nlayout_types; i++)
	log4c_layout_type_set(layout_types[i]);

    for (i = 0; i < nappender_types; i++)
	log4c_appender_type_set(appender_types[i]);
#ifdef WITH_ROLLINGFILE
    for (i = 0; i < nrollingpolicy_types; i++)
	log4c_rollingpolicy_type_set(rollingpolicy_types[i]);
#endif


    /* load configuration files */
    {
	int i;
	sd_debug("looking for conf files...");
	snprintf(rcfiles[0].name, sizeof(rcfiles[0].name) - 1, "%s/log4crc",
		 getenv("LOG4C_RCPATH") ? getenv("LOG4C_RCPATH") : ".");
	snprintf(rcfiles[1].name, sizeof(rcfiles[1].name) - 1, "%s/.log4crc",
		 getenv("HOME") ? getenv("HOME") : "");

	for (i = 0; i < nrcfiles; i++) {
	    sd_debug("checking for conf file at '%s'", rcfiles[i].name);
	    if (SD_ACCESS_READ(rcfiles[i].name)) continue;
	    if (SD_STAT_CTIME(rcfiles[i].name,&rcfiles[i].ctime) != 0)
		sd_error("sd_stat_ctime %s failed", rcfiles[i].name);
	    rcfiles[i].exists=1;
	    if (log4c_load(rcfiles[i].name) == -1) {
		sd_error("loading %s failed", rcfiles[i].name);
		ret = -1;
	    }
	    else
		sd_debug("loading %s succeeded", rcfiles[i].name);
	}
    }

    /* override configuration with environment variables */
    {
	const char* priority;
	const char* appender;

	sd_debug("checking environment variables...");
	if ( (priority = getenv("LOG4C_PRIORITY")) != NULL){
	    sd_debug("setting priority of root category to '%s'",
		     priority);
	    log4c_category_set_priority(log4c_category_get("root"),
					log4c_priority_to_int(priority));
	}

	if ( (appender = getenv("LOG4C_APPENDER")) != NULL){
	    sd_debug("setting appender of root category to '%s'",
		     appender);
	    log4c_category_set_appender(log4c_category_get("root"),
					log4c_appender_get(appender));
	}
    }

    /*
     *   For debug dump current types and instances:
     *   this allows the caller of log4c_init() to see:
     *   1. all types currently registered, including perhaps his own.
     *   2. all instances instantiated via the configuration file.
     *
     *   If the caller goes on to programmatically create instaces then he
     *   can call log4c_dump_all_instances() later himself to
     *   verify that they were created as expected.
     *
     */
#ifdef __SD_DEBUG__
    if( getenv("SD_DEBUG")){
	log4c_dump_all_types(stderr);
	log4c_dump_all_instances(stderr);
    }
#endif


    sd_debug("]");
    return ret;
}

/******************************************************************************/
void __log4c_reread(void)
{
    time_t file_ctime;
    int i;

    for (i = 0; i < nrcfiles; i++){
	/* only reread files that existed when we first initialized */
	if (rcfiles[i].exists && SD_STAT_CTIME(rcfiles[i].name,&file_ctime) == 0){
	    /* time_t is number of second since epoch, just compare for == */
	    if (file_ctime != rcfiles[i].ctime){
		sd_debug("Need reread on file %s\n",rcfiles[i].name);
		SD_STAT_CTIME(rcfiles[i].name,&rcfiles[i].ctime);
		if (log4c_rc_load(log4c_rc, rcfiles[i].name) == -1){
		    sd_error("re-loading config file %s failed", rcfiles[i].name);
		}
	    }
	}
    }
}

/******************************************************************************/
/*
 * Rereads any log4crc files that have changed
 */
void log4c_reread(void)
{
#ifdef __ENABLE_REREAD__
    if (0 != log4c_rc->config.reread){
	__log4c_reread();
    }
#endif
}



/******************************************************************************/
extern int log4c_fini(void)
{
    int rc = 0;

    /* Some acceptable use of goto here to avoid lots of nested ifs
     * when we need a quick exit
     */
    sd_debug("log4c_fini[");
    if (log4c_rc->config.nocleanup){
	sd_debug("not cleaning up--nocleanup specified in conf");
	rc = -1;
	goto log4c_fini_exit;
    }

    if (!log4c_is_init){
	rc = 0;
	sd_debug("not cleaning up--log4c not initialized");
	goto log4c_fini_exit;
    }

    log4c_is_init--;

    sd_debug("cleaning up category, appender, layout and"
	     "rollingpolicy instances");
    if (log4c_category_factory) {
	sd_factory_delete(log4c_category_factory);
	log4c_category_factory = NULL;
    }

    if (log4c_appender_factory) {
	sd_factory_delete(log4c_appender_factory);
	log4c_appender_factory = NULL;
    }

    if (log4c_layout_factory) {
	sd_factory_delete(log4c_layout_factory);
	log4c_layout_factory = NULL;
    }

    if (log4c_rollingpolicy_factory) {
	sd_factory_delete(log4c_rollingpolicy_factory);
	log4c_rollingpolicy_factory = NULL;
    }

#ifdef __SD_DEBUG__
    if( getenv("SD_DEBUG")){
	sd_debug("Instance dump after cleanup:");
	log4c_dump_all_instances(stderr);
    }
#endif
#if defined(__LOG4C_DEBUG__) && defined(__GLIBC__)
    muntrace();
#endif

log4c_fini_exit:
    sd_debug("]");

    return rc;
}


/******************************************************************************/
#ifdef __GNUC__
extern void __attribute__ ((constructor)) __log4c_init(void)
{
#ifdef WITH_CONSTRUCTORS
    log4c_init();
#endif
}

extern void __attribute__ ((destructor)) __log4c_fini(void)
{
#ifdef WITH_CONSTRUCTORS
    log4c_fini();
#endif
}
#endif

/******************************************************************************/

extern void log4c_dump_all_types(FILE *fp){
    /*
     *
     * For debug, dump all the types that have been registered during init. We just
     * display the name of the the type for the moment--the rest of the type info
     * right now consists of functions to call, so not really printable.
     */

    log4c_appender_types_print(fp);
    log4c_layout_types_print(fp);
#ifdef WITH_ROLLINGFILE
    log4c_rollingpolicy_types_print(fp);
#endif
}

extern void log4c_dump_all_instances(FILE *fp){

    /*
     * Also dump any instances that were created during init by
     * reading the conf file.
     *
     * An instances of a type consists of the base
     * type information (name plus function table) and an instance name and
     * configuration.  For example one can have an instance of the rollingfile
     * appender which logs to /var/tmp and another instance which logs to
     * /usr/tmp.  They are both of type rollingfile, but are distinct instances of
     * it
     */
    fprintf(fp, "instance dump follows (may be empty):\n");
    sd_factory_print(log4c_category_factory, fp);
    sd_factory_print(log4c_appender_factory, fp);
    sd_factory_print(log4c_layout_factory, fp);
#ifdef WITH_ROLLINGFILE
    sd_factory_print(log4c_rollingpolicy_factory, fp);
#endif
}
