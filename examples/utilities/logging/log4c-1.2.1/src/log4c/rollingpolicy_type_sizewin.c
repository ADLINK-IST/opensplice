
/*
 * rollingpolicy_type_sizewin.c
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
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <log4c/appender.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rollingpolicy_type_sizewin.h>

#include "appender_type_rollingfile.h"
#include <sd/malloc.h>
#include <sd/error.h>
#include <sd/sd_xplatform.h>

/* Internal structs that defines the conf and the state info
 * for an instance of the appender_type_rollingfile type.
 */    
struct __sizewin_conf {  
  long swc_file_maxsize;
  long swc_file_max_num_files;
};

struct __sizewin_udata {
  struct __sizewin_conf sw_conf;
  rollingfile_udata_t *sw_rfudata;
  const char *sw_logdir;
  const char *sw_files_prefix;
  char **sw_filenames; 
  long sw_last_index;
  #define SW_LAST_FOPEN_FAILED 0x0001
  int sw_flags;
};

/***************************************************************************/

static int sizewin_init(log4c_rollingpolicy_t *this, rollingfile_udata_t* rfup);
static int sizewin_rollover(log4c_rollingpolicy_t *this, FILE **current_fpp );
static int sizewin_is_triggering_event(
			       log4c_rollingpolicy_t *this,
			       const log4c_logging_event_t* a_event,
			       long current_file_size);

static char **sizewin_make_filename_array(rollingpolicy_sizewin_udata_t *swup);
static int sizewin_get_last_index(rollingpolicy_sizewin_udata_t * swup);
static char* sizewin_get_filename_by_index(rollingpolicy_sizewin_udata_t * swup,
					   long i);
static int sizewin_open_zero_file(char *filename, FILE **fpp );

/*******************************************************************************
              Policy interface: init, is_triggering_event, rollover
*******************************************************************************/

static int sizewin_is_triggering_event(
			       log4c_rollingpolicy_t *this,
			       const log4c_logging_event_t* a_event,
			       long current_file_size){
  long len = 0;
  int decision = 0;
  rollingpolicy_sizewin_udata_t *swup = log4c_rollingpolicy_get_udata(this);
  /*rollingfile_udata_t *rfup = log4c_rollingpolicy_get_udata(this);  */

  sd_debug("sizewin_is_triggering_event[");

  /* find the len of this message
   xxx this should be provided by the logging_event class
  */
  len = strlen(a_event->evt_rendered_msg);
  sd_debug("fsize=%ld max=%ld len=%ld", current_file_size,
	  swup->sw_conf.swc_file_maxsize, len  );
  if ( swup->sw_conf.swc_file_maxsize > 0 &&
      len + current_file_size > swup->sw_conf.swc_file_maxsize ) {
    sd_debug("triggering event");
    decision = 1;
  } else {
    sd_debug("not triggering event");
  }
  
  sd_debug("]");  
  return(decision);
}

/*******************************************************************************/

static int sizewin_rollover(log4c_rollingpolicy_t *this, FILE ** current_fpp ){
  int rc = 0;
  rollingpolicy_sizewin_udata_t *swup = log4c_rollingpolicy_get_udata(this);
  int k = 0;
  int i = 0;
 
  sd_debug("sizewin_rollover[");
  /* Starting at the last_index work back renaming the files and
     leaving space for the .0 file.
     If the last index is negative then it means the file doesn't exist
     so we create the first file
  */

  if ( !swup || !swup->sw_logdir){
    sd_error("rollingpolicy '%s' not yet configured (logdir,prefix etc.)",
      log4c_rollingpolicy_get_name(this));
  } else {
  
   k = swup->sw_last_index;

   if ( k < 0 ) {
     sd_debug("creating first file");
     if (sizewin_open_zero_file(swup->sw_filenames[0], current_fpp)){
       swup->sw_flags |= SW_LAST_FOPEN_FAILED;
       sd_error("open zero file failed");
     } else{
       swup->sw_flags &= !SW_LAST_FOPEN_FAILED;
     }
     swup->sw_last_index = 0;
   } else {
     sd_debug("rolling up existing files");

     if ( k == swup->sw_conf.swc_file_max_num_files-1) {    
       if(unlink(swup->sw_filenames[k])){
          sd_error("unlink failed"); 
          rc = 1;
       } else {
         k = swup->sw_conf.swc_file_max_num_files-2;
       }
     } else {
       /* not yet reached the max num of files
	      * so there's still room to rotate the list up */    
     }

     /* If there's a current open fp, close it.*/
     if ( !(swup->sw_flags & SW_LAST_FOPEN_FAILED) && *current_fpp) {
       if(fclose(*current_fpp)){
         sd_error("failed to close current log file");
         rc = 1;
       }
     } else {
       if( (swup->sw_flags & SW_LAST_FOPEN_FAILED)){
           sd_debug("Not closing log file...last open failed");
       } else if (*current_fpp == 0) {
         sd_debug("No log file currentlty open...so not closing it");
       }else {
         sd_debug("Not closing current log file...not sure why");
       }
     }
     /* Now, rotate the list up if all seems ok, otherwise 
     * don't mess with teh files if something seems to have gone wrong
     */
     if ( !rc){
       sd_debug("rotate up , last index is %d", k); 
       i = k;
       while ( i >= 0 ) {
         sd_debug("Renaming %s to %s",
           swup->sw_filenames[i], swup->sw_filenames[i+1]);
         if(rename( swup->sw_filenames[i], swup->sw_filenames[i+1])){
           sd_error("rename failed"); 
           rc = 1;
           break;
         }
         i--;
       }
       if ( !rc){
         swup->sw_last_index = k + 1;
       }
     } else {
       sd_debug("not rotating up--some file access error");
     }
         
     /* Now open up the 0'th file for writing */
     if (sizewin_open_zero_file(swup->sw_filenames[0], current_fpp)){
       swup->sw_flags |= SW_LAST_FOPEN_FAILED;
       sd_error("open zero file failed");
       rc = 1;
     } else{
       swup->sw_flags &= !SW_LAST_FOPEN_FAILED;
       rc = 0;
     }

   }
   sd_debug("current file descriptor '%d'", fileno(*current_fpp));
  }
  sd_debug("]");
  return(rc);
}

/*******************************************************************************/

static int sizewin_init(log4c_rollingpolicy_t *this, rollingfile_udata_t *rfup){
  rollingpolicy_sizewin_udata_t *swup =  NULL;
  int i = 0;

  sd_debug("sizewin_init[");
  if (!this){
    goto sizewin_init_exit;
  }
  
  swup = log4c_rollingpolicy_get_udata(this);
  if ( swup == NULL ){
    swup = sizewin_make_udata();
    log4c_rollingpolicy_set_udata(this, swup);
  }
  
  /* initialize the filename array and last index */
  swup->sw_logdir = rollingfile_udata_get_logdir(rfup);
  swup->sw_files_prefix = rollingfile_udata_get_files_prefix(rfup);

  swup->sw_filenames = sizewin_make_filename_array(swup);
  for ( i = 0; i<swup->sw_conf.swc_file_max_num_files; i++){
    sd_debug("%s", swup->sw_filenames[i]);
  }
  swup->sw_last_index = sizewin_get_last_index(swup);  
  sd_debug("last index '%d'", swup->sw_last_index);
  
sizewin_init_exit:
  sd_debug("]"); 

  return(0);
}

/*******************************************************************************/

static int sizewin_fini(log4c_rollingpolicy_t *this){
  rollingpolicy_sizewin_udata_t *swup =  NULL;
  int i = 0;
  int rc = 0;

  sd_debug("sizewin_fini[ ");
  if (!this){
    goto sizewin_fini_exit;
  }
  
  swup = log4c_rollingpolicy_get_udata(this);
  if (!swup){
    goto sizewin_fini_exit;
  }
  
  for ( i = 0; i<swup->sw_conf.swc_file_max_num_files; i++){
    if ( swup->sw_filenames[i]){
      free(swup->sw_filenames[i]);
    }
  }
  free(swup->sw_filenames);

  /* logdir and files_prefix are just pointers into the rollingfile udata
  * so they are not ours to free--that will be done by the free call to
  * the rollingfile appender
  */
  sd_debug("freeing sizewin udata from rollingpolicy instance");
  free(swup);
  log4c_rollingpolicy_set_udata(this,NULL);
  
sizewin_fini_exit:
  sd_debug("]");
  
  return(rc);
}

/*******************************************************************************
                           sizewin specific conf functions
*******************************************************************************/

LOG4C_API rollingpolicy_sizewin_udata_t *sizewin_make_udata(void){
  rollingpolicy_sizewin_udata_t *swup = NULL;
  swup = (rollingpolicy_sizewin_udata_t *)sd_calloc(1, 
                              sizeof(rollingpolicy_sizewin_udata_t));
  sizewin_udata_set_file_maxsize(swup,
				  ROLLINGPOLICY_SIZE_DEFAULT_MAX_FILE_SIZE);
  sizewin_udata_set_max_num_files(swup,
				   ROLLINGPOLICY_SIZE_DEFAULT_MAX_NUM_FILES);

  return(swup);

}

/*******************************************************************************/

LOG4C_API int sizewin_udata_set_file_maxsize(rollingpolicy_sizewin_udata_t * swup,
					 long max_size){

  swup->sw_conf.swc_file_maxsize = max_size;

  return(0);
  
}

/****************************************************************************/

LOG4C_API int sizewin_udata_set_max_num_files(rollingpolicy_sizewin_udata_t *swup, 
					  long max_num){

  swup->sw_conf.swc_file_max_num_files = max_num;

  return(0);
}

/****************************************************************************/

LOG4C_API int sizewin_udata_set_rfudata(rollingpolicy_sizewin_udata_t *swup,
				     rollingfile_udata_t *rfup ){

  swup->sw_rfudata = rfup;

  return(0);
}

/*****************************************************************************
                       private functions
*****************************************************************************/


static char **sizewin_make_filename_array(rollingpolicy_sizewin_udata_t *swup){

  int i = 0;
  char **filenames = NULL;

  filenames = (char **)sd_calloc(swup->sw_conf.swc_file_max_num_files,
				 sizeof(char *));
  while ( i <  swup->sw_conf.swc_file_max_num_files){
    
    filenames[i] = sizewin_get_filename_by_index(swup,i);
    i++;
  }
  return(filenames);
}

/****************************************************************************/

static char* sizewin_get_filename_by_index(rollingpolicy_sizewin_udata_t* swup,
					   long i){
  char tmp[100];
  long filename_len = 0;
  char *s = NULL;
  
  sprintf(tmp, "%ld", i);
  filename_len = strlen(swup->sw_logdir) + 1 +
    strlen(swup->sw_files_prefix) + 1 +
    strlen(tmp) + 1 + 10; /* a margin */
  s = (char *)malloc(filename_len);      
  sprintf( s, "%s%s%s%s%s", swup->sw_logdir,
	   FILE_SEP, swup->sw_files_prefix, ".", tmp);	      
  return(s); 
}

/****************************************************************************/

static int sizewin_get_last_index(rollingpolicy_sizewin_udata_t *swup){

  /* Walk the filelist to find the last one that exists to
     initialize last_index
  */
  
  int i = 0;
  struct stat	info;
  
  while( i <swup->sw_conf.swc_file_max_num_files &&
    stat(swup->sw_filenames[i], &info) == 0  ) {
    i++;
  }

  if ( i == 0 ){
    if ( stat(swup->sw_filenames[i], &info) == 0 ) {
      return(0);
    } else {
      return(-1);
    }
  } else {
    return(i-1);
  }

}

/*******************************************************************************/

static int sizewin_open_zero_file(char *filename, FILE **fpp ){
  int rc = 0;
  sd_debug("sizewin_open_zero_file['%s'", filename);

  if ( (*fpp = fopen(filename, "w+")) == NULL){
   sd_error("failed to open zero file '%s'--defaulting to stderr--error='%s'",
     filename, strerror(errno));    
   *fpp = stderr;
    rc = 1;
  }  
    
  /* unbuffered mode at the filesystem level
   xxx make this configurable from the outside ?
  */
  setbuf(*fpp, NULL);    

  sd_debug("]");  
  return(rc);

}

/****************************************************************************/

const log4c_rollingpolicy_type_t log4c_rollingpolicy_type_sizewin = {
    "sizewin",
    sizewin_init,
    sizewin_is_triggering_event,
    sizewin_rollover,
    sizewin_fini
};

