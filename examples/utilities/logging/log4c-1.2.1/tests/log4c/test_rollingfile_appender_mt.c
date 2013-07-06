/*
 * test_rollingfile_appender.c
 *
 * This file  demonstrates how to programatically use the rolling
 * file appender and tests it in a multi-threaded environment.  See
 * also the API documentation for the appender_type_rollingfile.h file.
 *
 * It is possible to be more terse here if you accept the default values, 
 * but we spell it out explicitly. 
 *
 * See the COPYING file for the terms of usage and distribution.
 */
 
 #ifdef HAVE_CONFIG_H
 #include "config.h"
 #endif
 
#include <stdio.h> 
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>                    
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <log4c.h>
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy_type_sizewin.h>

/* 
 * Include sd_xplatform to pick up handy xplatform macros.
 * This file is not part of the distribution of log4c so an application using the 
 * distribution of log4c must handle it's own x-platform issues: eg. threading,
 * the slight differences between the c functions etc.
*/
#include <sd/sd_xplatform.h>

/*********************** Parameters **********************************
 *
 * could be taken from the command line to facilitate testing
 *
*/

/*
 * rolling file specific params
*/
char *param_log_dir = ROLLINGFILE_DEFAULT_LOG_DIR;
char* param_log_prefix = ROLLINGFILE_DEFAULT_LOG_PREFIX"rfmt";
long param_max_file_size = 10*1024; /* 0 means 'no_limit' */
long param_max_num_files = 6;

/*
 * Other Logging params
*/
char *param_layout_to_use = "basic"; /* could also be "dated" */
#define PARAM_NUM_THREADS 10
int param_loops_per_thread = 100;

/*******************************************************************
 *
 * Globals
 *
 */
log4c_category_t* root = NULL;
log4c_appender_t* file_appender = NULL;
log4c_layout_t* basic_layout = NULL;
/******************************************************************************
*
* Local functions
*/
 
typedef XP_UINT64 usec_t;
static usec_t now(void)
{
#ifdef _WIN32
 FILETIME tv;
 ULARGE_INTEGER   li;
#else
 struct timeval tv;
#endif
  
   SD_GETTIMEOFDAY(&tv, NULL);

#ifdef _WIN32
	memcpy(&li, &tv, sizeof(FILETIME));
	li.QuadPart /= 10;                /* In microseconds */
	/* printf("timestampstamp usec %I64u\n", li.QuadPart);*/
	return li.QuadPart;
#else
  return (usec_t) (tv.tv_sec * 1000000 + tv.tv_usec);
#endif

}

/******************************************************************************/

/*
 * Init log4c and configure a rolling file appender
 *
*/
static void init_log4c_with_rollingfile_appender(){
  int rc = 2;
  rollingfile_udata_t *rfup = NULL;
  log4c_rollingpolicy_t *policyp = NULL;
  rollingpolicy_sizewin_udata_t *sizewin_confp = NULL;

  printf("using the rolling file appender with multiple logging threads"
          "to write test log files\n"
          "to log directory '%s', log file prefix is '%s'"
          ", max file size is '%ld'\n"
          "max num files is '%ld'\n",
          param_log_dir, param_log_prefix, param_max_file_size,
          param_max_num_files);
  
  if ( (rc = log4c_init()) == 0 ) {
    printf("log4c init success\n");
  } else {
    printf("log4c init failed--error %d\n", rc);
    return;
  }

  /*
   * Get a reference to the root category
  */
  root = log4c_category_get("root");
  log4c_category_set_priority(root,
			      LOG4C_PRIORITY_WARN);

  /* 
   * Get a file appender and set the type to rollingfile
  */
  file_appender = log4c_appender_get("aname");
  log4c_appender_set_type(file_appender, 
    log4c_appender_type_get("rollingfile"));
  
  /*
   * Make a rolling file conf object and set the basic parameters 
  */
  rfup = rollingfile_make_udata();              
  rollingfile_udata_set_logdir(rfup, param_log_dir);
  rollingfile_udata_set_files_prefix(rfup, param_log_prefix);

  /*
   * Get a new rollingpolicy
   * type defaults to "sizewin" but set the type explicitly here
   * to show how to do it.
  */
  policyp = log4c_rollingpolicy_get("a_policy_name");
  log4c_rollingpolicy_set_type(policyp,
              log4c_rollingpolicy_type_get("sizewin"));

  /*
   * Get a new sizewin policy type and configure it.
   * Then attach it to the policy object.
  */
  sizewin_confp = sizewin_make_udata();
  sizewin_udata_set_file_maxsize(sizewin_confp, param_max_file_size);
  sizewin_udata_set_max_num_files(sizewin_confp, param_max_num_files);
  log4c_rollingpolicy_set_udata(policyp,sizewin_confp);

  /*
   * Configure our rolling policy, then use that to configure
   * our rolling file appender.
  */
  
  rollingfile_udata_set_policy(rfup, policyp);
  log4c_appender_set_udata(file_appender, rfup);
 
  /*
   * Open the rollingfile appender.  This will also trigger
   * the initialization of the rollingpolicy.
   * Log4c is not thread safe with respect to creation/initialization
   * of categories and appenders, so we must call the open
   * once, before all the threads kick in logging with this appender.
   * Calling open as a side effect of the first call to append does
   * not work in a MT environment.
   *
   * For info, could also call init directly on the rollingpolicy here:
   *   log4c_rollingpolicy_init(policyp, rfup);
   * 
   *
  */
  log4c_appender_open(file_appender);

  /*
   * Configure a layout for the rolling file appender 
   * (could also have chosen "basic" here, for example)
  */
  log4c_appender_set_layout(file_appender, 
                            log4c_layout_get(param_layout_to_use) );

  /*
   * Configure the root category with our rolling file appender...
   * and we can then start logging to it.
   *
  */
  log4c_category_set_appender(root,file_appender );
  
  log4c_dump_all_instances(stderr);


}

/*
 * thread function to log messages
 *
*/
#ifdef _WIN32
unsigned int thread_log_messages(void *arg) 
#else
void * thread_log_messages(void *arg) 
#endif
{ 

  int loop;
  usec_t stop_time;
  usec_t start_time;
  unsigned long elapsed;
  int tid,j;   
  char buf[1024];

  tid= *((int *)arg);
 
  loop = 0 ;
  while (loop < param_loops_per_thread) {

    /* Now start the transaction measurement. */ 
	 start_time = now();

    /* Do a bit of work, fabricate a string containing the thread id */
    buf[0] = '\0';
    j = 0;
    while( j < 128) {
      snprintf(&buf[strlen(buf)], 1024-strlen(buf), "%d", tid);
      j++;
    }
    sleep(1);
    
    stop_time = now();
    elapsed = (unsigned long)(((usec_t)stop_time - (usec_t)start_time)/1000);
    /*printf("Hello world--thread(%d)! (loop %d of %d) sleeping for 1 second"
      "--measured time %lld--%s\n", tid, loop, param_loops_per_thread,
        stop_time-start_time,buf);*/
    log4c_category_fatal(root,
      "Hello world--thread(%d)! (loop %d of %d) sleeping for 1 second"
      "--measured tme %ld ms--%s\n", tid, loop, param_loops_per_thread,
        elapsed,buf);
    
    loop++;
  } 
#ifndef _WIN32
  return(NULL);
#else
	return(0);
#endif
}


int main(int argc, char *argv[]) 
{                                                                     
  
  int rc = 0;
  pthread_t tid[PARAM_NUM_THREADS];
  int i;                                    

  init_log4c_with_rollingfile_appender();

  
  /* Simple start/stop example. */  
  printf("Launching %d threads to log messages\n", PARAM_NUM_THREADS);
  for ( i = 0; i < PARAM_NUM_THREADS; i++){
	  int * this_thread_arg =
    NULL;
		  
		  if ((this_thread_arg = (int *)malloc(sizeof(int))) == NULL){
			  exit(1);
		  }	
		*this_thread_arg = i;
		rc = pthread_create(&tid[i], NULL, thread_log_messages,
								 (void *)this_thread_arg);
		if ( tid[i] == 0 ) {
								printf("Error creating thead %d...exiting\n", i);
								exit(1);
		}
  }

  for ( i = 0; i < PARAM_NUM_THREADS; i++){
    pthread_join(tid[i], NULL);
  }
  
  /* Explicitly call the log4c cleanup routine */
  if ( log4c_fini()){
    printf("log4c_fini() failed");
  }

  return rc; 
} 

