
/*
 * test_rollingfile_appender.c
 *
 * This file  demonstrates how to programatically use the rolling
 * file appender.  See also the API documentation for the
 * appender_type_rollingfile.h file.
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
#include <log4c.h>
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rollingpolicy_type_sizewin.h>

/*********************** Parameters **********************************
 *
 * params could be taken from the command line to facillitate testing
 *
*/

/*
 * rolling file specific params
*/
#ifdef _WIN32
#define FILE_SEP "\\"
#else
#define FILE_SEP "/"
#endif
char *param_log_dir = ROLLINGFILE_DEFAULT_LOG_DIR;
char* param_log_prefix = ROLLINGFILE_DEFAULT_LOG_PREFIX"rf";
long param_max_file_size = 1666;
long param_max_num_files = 6;

/*
 * Other Logging params
 * xxx Problem with dated layout on windows: assert failure
 * in gmtime call.
*/
char *param_layout_to_use = "dated"; /* could also be "basic" */

/*******************************************************************
 *
 * Globals
 *
 */
log4c_category_t* root = NULL;
log4c_appender_t* file_appender = NULL;
/******************************************************************************/

/*
 * Init log4c and configure a rolling file appender
 *
*/
static void init_log4c_with_rollingfile_appender(){
  int rc = 2;
  rollingfile_udata_t *rfup = NULL;
  log4c_rollingpolicy_t *policyp = NULL;
  rollingpolicy_sizewin_udata_t *sizewin_udatap = NULL;

  printf("using the rolling file appender "
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
   * Make a rolling file udata object and set the basic parameters 
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
  sizewin_udatap = sizewin_make_udata();
  sizewin_udata_set_file_maxsize(sizewin_udatap, param_max_file_size);
  sizewin_udata_set_max_num_files(sizewin_udatap, param_max_num_files);
  log4c_rollingpolicy_set_udata(policyp,sizewin_udatap);

  /*
   * Now set that policy in our rolling file appender udata.
  */
  
  rollingfile_udata_set_policy(rfup, policyp);
  log4c_appender_set_udata(file_appender, rfup);
 
  /*
  * Allow the rolling policy to initialize itself:
  * it needs to know the rollingfile udata it is associated with--it
  * picks up some parameters in there
  */
  log4c_rollingpolicy_init(policyp, rfup);

  /*
   * Configure a layout for the rolling file appender 
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

static int test0(int argc, char* argv[])
{    
  int i = 0;
  init_log4c_with_rollingfile_appender();
  
  for (i = 0; i<200; i++){ 

    log4c_category_fatal(root,
		       "%d%d%d%d%d%d%d%d%d%d%d%d%d\n",i,i,i,i,i,i,i,i,i,i,i,i,i);

  }

  /* Explicitly call the log4c cleanup routine */
  if ( log4c_fini()){
    printf("log4c_fini() failed");
  }
  
  return 1;
}

/******************************************************************************/
int main(int argc, char* argv[])
{    
   
  test0(argc,argv);

    return(0);
}
