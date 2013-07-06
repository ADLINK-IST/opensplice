/*
* This is one of the log4c example programs.
*                                   
* In this example we link against a shared library that has
* additional formatters and appenders.  This shows how easy it is to
* add in more appenders and formatters to the log4c framework.
*
* With gcc this file is in fact exactly the same as application_1.c--
* only at link time it is linked with a new library.  
* With other compilers using explicit initialization, this program needs
* to explicitly tell the custom appender/formatter lib to initialize
* itself and it's appenders and formatters.
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

extern int init_examples_lib(void);

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
  int looptime = 0;
  log4c_category_t* mycat = NULL;
  
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
  
  log4c_init();
  
  /*
  * You could choose to wrap the log4c_category_log with some macro
  * that then calls an accessor to get your pre-created category
  * mycat and then logs to it.  But for now we just focus on the fact
  * that we are using log4crc to have this application use the new
  * formatters and appenders we wrote as examples
  */
  mycat = log4c_category_get("six13log.log.app.application2");
  
  gettimeofday(&start_time, NULL);
  gettimeofday(&now_time, NULL);
  
  while ( (now_time.tv_sec - start_time.tv_sec) < looptime)
  {
    log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, "Debugging app 2");	      
    log4c_category_log(mycat, LOG4C_PRIORITY_ERROR,
      "some error from app2 at line %d in file %s",
      __LINE__, __FILE__);
    sleep(3);
    
    gettimeofday(&now_time, NULL);
  }
  
  /* Explicitly call the log4c cleanup routine */
  if ( log4c_fini()){
    printf("log4c_fini() failed");
  }
  
  return 0;
}
