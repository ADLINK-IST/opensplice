
/*
 * appender_type_rollingfile.c
 *
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <log4c/appender.h>
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy.h>
#include <sd/malloc.h>
#include <sd/error.h>
#include <sd/sd_xplatform.h>

/* Internal structs that defines the conf and the state info
 * for an instance of the appender_type_rollingfile type.
 */
struct __rollingfile_conf {
  const char* rfc_logdir;
  const char* rfc_files_prefix;
  log4c_rollingpolicy_t* rfc_policy;
};

struct __rollingfile_udata {
  struct __rollingfile_conf rfu_conf;
  long rfu_current_file_size;
  FILE *rfu_current_fp;
  char *rfu_base_filename;
  pthread_mutex_t rfu_mutex;
};

static int rollingfile_open_zero_file(char *filename, long *fsp, FILE **fpp);
static char *rollingfile_make_base_name(const char *log_dir, const char* prefix);

/***************************************************************************
                Appender Interface functions: open, append, close

****************************************************************************/
static int rollingfile_open(log4c_appender_t* this)
{
  int rc = 0;
  rollingfile_udata_t *rfup = log4c_appender_get_udata(this);

  sd_debug("rollingfile_appender_open[");

 /* xxx Make the init here more fine grained */

 if (rfup == NULL ){

   /* No info provided so set defaults */
   sd_debug("making new rollingfile conf object, with default logdir/logprefix");
   rfup = rollingfile_make_udata();

   rollingfile_udata_set_logdir(rfup, ROLLINGFILE_DEFAULT_LOG_DIR);
   rollingfile_udata_set_files_prefix(rfup, ROLLINGFILE_DEFAULT_LOG_PREFIX);

 } else {
    sd_debug("appender has udata already: logdir='%s' logprefix='%s'",
      rollingfile_udata_get_logdir(rfup),
      rollingfile_udata_get_files_prefix(rfup));
 }

 rfup->rfu_current_file_size = 0;
 pthread_mutex_init(&rfup->rfu_mutex, NULL);

 /* this will open the right file and set the current fp */
 rfup->rfu_base_filename = rollingfile_make_base_name(
				       rfup->rfu_conf.rfc_logdir,
				       rfup->rfu_conf.rfc_files_prefix);

 /* xxx A policy exists, but is it initalised ?
    Do we need such a concept ?
    I think so because we need to be sure
  */

 /*
  * If a policy exists defer to it to open the first file:
  * it may wish to apply a policy to existing log files.
 */
 if ( rfup->rfu_conf.rfc_policy ) {
   sd_debug("rollingfile udata has a policy '%s'--calling rollover",
          log4c_rollingpolicy_get_name( rfup->rfu_conf.rfc_policy)  );

   /*
   * If the policy is not yet initialized force it now to default values
   */
   if (!log4c_rollingpolicy_is_initialized(rfup->rfu_conf.rfc_policy)){
     sd_debug("policy not initialized, calling init now");
		   log4c_rollingpolicy_init(rfup->rfu_conf.rfc_policy,
			   rfup);
   }

   if ( log4c_rollingpolicy_rollover(rfup->rfu_conf.rfc_policy,
			       &rfup->rfu_current_fp)){
                                rc = 1; /* rollover error */
   } else {
    rfup->rfu_current_file_size = 0;
   }
 } else {
   /* No policy defined, open it ourselves */
   rollingfile_open_zero_file( rfup->rfu_base_filename,
			      &rfup->rfu_current_file_size,
			       &rfup->rfu_current_fp);
 }

 sd_debug("]");

 return rc;

}

/***************************************************************************/

static int rollingfile_append(log4c_appender_t* this,
			 const log4c_logging_event_t* a_event)
{
  rollingfile_udata_t* rfup = log4c_appender_get_udata(this);

  int rc = 0;

  sd_debug("rollingfile_append[");

  pthread_mutex_lock(&rfup->rfu_mutex);  /***** LOCK ****/

  if ( rfup->rfu_conf.rfc_policy != NULL) {

    /* some policy set */
    sd_debug("check trigger, currfs=%ld",rfup->rfu_current_file_size);

    if( log4c_rollingpolicy_is_triggering_event(rfup->rfu_conf.rfc_policy,
					 a_event,
					 rfup->rfu_current_file_size)){
#ifdef __SD_DEBUG__
      sd_debug("non-buffered rotate event len=%ld, currfs=%ld",
         strlen(a_event->evt_rendered_msg), rfup->rfu_current_file_size);
#endif

      if ( (rc = log4c_rollingpolicy_rollover(rfup->rfu_conf.rfc_policy,
			    &rfup->rfu_current_fp)) <= ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG){
        rfup->rfu_current_file_size = 0;
      }
    } else {
      /* no need to rotate up--stick with the current fp */
      /*sd_debug("non-buffered not rotate event");*/
    }
  }else {
    /* no rotation policy, just continue using the current fp */
  }

  /* only attempt the write if the policy implem says I can */
  if ( rc <= ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG ) {
   rc = fprintf(rfup->rfu_current_fp, a_event->evt_rendered_msg);
   rfup->rfu_current_file_size += strlen(a_event->evt_rendered_msg);

   /*
    * the fprintf needs to be inside the lock
    * because otherwise we might be writing to a file and another
    * thread could be renaming it in some rotation policy code
    * ...which wouldn't be nice
    */
    } else {
      sd_error("not logging--something went wrong (trigger check or"
                " rollover failed)");
    }
   sd_debug("]");
  pthread_mutex_unlock(&rfup->rfu_mutex);  /****** UNLOCK *****/
  return (rc);

}
/****************************************************************************/
static int rollingfile_close(log4c_appender_t* this)
{
  int rc = 0;
  rollingfile_udata_t* rfup = NULL;

  sd_debug("rollingfile_close[");
  if(!this){
    rc = -1;
  } else {

    rfup = log4c_appender_get_udata(this);

    pthread_mutex_lock(&rfup->rfu_mutex);  /***** LOCK ****/
    rc = (rfup->rfu_current_fp ? fclose(rfup->rfu_current_fp) : 0);
    rfup->rfu_current_fp = NULL;

    rfup->rfu_current_file_size = 0;
    if( rfup->rfu_base_filename) {
      free( (char *)rfup->rfu_base_filename);
      rfup->rfu_base_filename = NULL;
    }
    if( rfup->rfu_conf.rfc_logdir) {
      free( (char *)rfup->rfu_conf.rfc_logdir);
      rfup->rfu_conf.rfc_logdir = NULL;
    }
    if( rfup->rfu_conf.rfc_files_prefix) {
      free( (char *)rfup->rfu_conf.rfc_files_prefix);
      rfup->rfu_conf.rfc_files_prefix = NULL;
    }
    if ( rfup->rfu_conf.rfc_policy){
      if (!log4c_rollingpolicy_fini(rfup->rfu_conf.rfc_policy)){
        rfup->rfu_conf.rfc_policy = NULL;
      }else{
        sd_debug("rollingpolicy fini failed");
        rc = -1;
      }
    }

    pthread_mutex_unlock(&rfup->rfu_mutex);  /****** UNLOCK *****/
  }
  sd_debug("]");
  return(rc);
}

/****************************************************************************
                     rollingfile specific conf functions
****************************************************************************/

int rollingfile_udata_set_policy(rollingfile_udata_t * rfup,
			       log4c_rollingpolicy_t *polp){

  rfup->rfu_conf.rfc_policy = polp;
  return(0);
}

/*******************************************************************************/

rollingfile_udata_t *rollingfile_make_udata(void){
  rollingfile_udata_t *rfup = NULL;
  rfup = (rollingfile_udata_t *)sd_calloc(1, sizeof(rollingfile_udata_t));

  return(rfup);
}

/*******************************************************************************/

LOG4C_API int rollingfile_udata_set_logdir(rollingfile_udata_t* rfup, char *logdir){

  rfup->rfu_conf.rfc_logdir = strdup(logdir);

  return(0);
}
/*******************************************************************************/

LOG4C_API const char * rollingfile_udata_get_logdir(rollingfile_udata_t* rfup){

  return(rfup->rfu_conf.rfc_logdir);

}
/*******************************************************************************/

LOG4C_API int rollingfile_udata_set_files_prefix( rollingfile_udata_t* rfup, char* fprefix){

  rfup->rfu_conf.rfc_files_prefix = strdup(fprefix);

  return(0);
}
/*******************************************************************************/

LOG4C_API const char*  rollingfile_udata_get_files_prefix( rollingfile_udata_t* rfup){

  return(rfup->rfu_conf.rfc_files_prefix);

}
/*******************************************************************************/

LOG4C_API long  rollingfile_get_current_file_size( rollingfile_udata_t* rfup){

  return(rfup->rfu_current_file_size);

}

/*****************************************************************************
                           Private functions
*****************************************************************************/
#if 0
/* this function currently unused but would be handy for sanity
 * checking what we think is the file size against the actual size
 */
static int rollingfile_get_system_filesize(FILE *fp){
  struct stat	info;
  int		rv;

  if ((rv = fstat(fileno(fp), &info)) != 0) {
    return -1;
  }
  return info.st_size;
}
#endif
/*******************************************************************************/

static int rollingfile_open_zero_file(char *filename, long *fsp, FILE **fpp ){

  sd_debug("rollingfile_open_zero_file[");
  if ( (*fpp = fopen(filename, "w+")) == NULL){
   *fpp = stderr;
  }
  *fsp = 0;

  /* unbuffered mode at the filesystem level
   xxx make this configurable from the outside ?
  */
  setbuf(*fpp, NULL);

  sd_debug("]");
  return(0);

}
/*******************************************************************************/

static char *rollingfile_make_base_name(const char *logdir, const char* prefix){

  int filename_len = 0;
  char* s = NULL;

  filename_len = strlen(logdir) + 1 +
    strlen(prefix) + 1 + 10; /* a margin */

  s = (char *)malloc(filename_len);
  sprintf( s, "%s%s%s", logdir,
	   FILE_SEP, prefix);

  return(s);
}

/****************************************************************************/
const log4c_appender_type_t log4c_appender_type_rollingfile = {
    "rollingfile",
    rollingfile_open,
    rollingfile_append,
    rollingfile_close
};

