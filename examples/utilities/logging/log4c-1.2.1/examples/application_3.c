/*
* This is one of the log4c example programs.
*                                   
* In this example we link against a shared library that has
* additional formatters and appenders where the formatter is
* augmented to expect the extra user location info that we are sending
* in the void* argument the location_info struct
* 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#include <windows.h>
#include <winsock.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "log4c.h"
#include "application_3.h"

extern int init_examples_lib(void);
extern int init_userloc_formatters(void);

#ifdef _WIN32
int gettimeofday(struct timeval* tp, void* tzp) {
  DWORD t;
  t = timeGetTime();
  tp->tv_sec = t / 1000;
  tp->tv_usec = t % 1000;
  /* 0 indicates that the call succeeded. */
  return 0;
}

int sleep(DWORD t){
  
	Sleep(1000*t);
	return(0);
}
#endif /* _WIN32 */


int main(int argc, char** argv)
{
  struct timeval start_time;
  struct timeval now_time;
  user_locinfo_t userloc;
  int looptime = 0;
  log4c_category_t* mycat = NULL;

  /* setup for the extra user location info */
  char hostname[256];
  int  pid = getpid();
  gethostname(hostname,255);
  hostname[255] = '\0';
  userloc.hostname = hostname;
  userloc.pid = pid;

  if (argc < 2)
  {
    printf("usage: %s loop_time_in_seconds\n",argv[0]);
    exit (1);
  }
  if (sscanf(argv[1],"%d",&looptime) != 1)
  {
    printf("could not convert %s to number of seconds to loop\n",argv[1]);
    exit(1);
  }
  
  /*
  * Here, if using explicit initialization (as opposed to implicit via the
    * init phase of the library) it's important to initialize the custom appenders
  * and layouts before calling log4c_init().
  * This is because when log4c_init() parses the config file it looks for the
  * types mentioned in the file to set up the relations between categories, 
  * appenders and layouts.  If it does not find a coresponding type in the 
  * internal hash tables, it creates one with that type name, but the function
  * table is not set up--so that at log time nothing happens. 
  *  
  */
  
  init_examples_lib();
  init_userloc_formatters();

  log4c_init();

  /*
  * Here we add our own userdefined location info, and then pick that up in our formatter
  */
  mycat = log4c_category_get("six13log.log.app.application3");
  
  gettimeofday(&start_time, NULL);
  gettimeofday(&now_time, NULL);
  
  while ( (now_time.tv_sec - start_time.tv_sec) < looptime)
  {
    /* LINE and FILE are bad */
    log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, "Debugging app 3, direct log call");  

    /* using the new API directly */
    const log4c_location_info_t locinfo2 = LOG4C_LOCATION_INFO_INITIALIZER(&userloc);
    log4c_category_log_locinfo(mycat, &locinfo2,
    	LOG4C_PRIORITY_DEBUG, "Debugging application number THREE with extra user location");
    const log4c_location_info_t locinfo3 = LOG4C_LOCATION_INFO_INITIALIZER(NULL);
    log4c_category_log_locinfo(mycat, &locinfo3,
    	LOG4C_PRIORITY_ERROR,
      	"some error from app at line %d in file %s with NULL for extra user location info",
      	__LINE__, __FILE__);

    /* using the new API with the define wrapper */
    log4c_category_log_userinfo(mycat, &userloc,  LOG4C_PRIORITY_DEBUG, "Debug app3 wrapper define");

    sleep(3);
    
    gettimeofday(&now_time, NULL);
  }
  
  /* Explicitly call the log4c cleanup routine */
  if ( log4c_fini()){
    printf("log4c_fini() failed");
  }
  
  return 0;
}
