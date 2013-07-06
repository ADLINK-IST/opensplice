/*
* This is one of log4c example programs.
*
* Notice how no relationships between the category and a certain
* priority, appender, or formatter are coded here.  These are all left
* to the log4crc config file so they can be chaned without recompiling
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
  int i = 0;
  
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
  
  /* You could put your category class into a file with a wrapper macro
  * to make calling it easier and more consistent
  */
  
  log4c_init();
  mycat = log4c_category_get("six13log.log.app.application1");
  
  gettimeofday(&start_time, NULL);
  gettimeofday(&now_time, NULL);
  i = 0;
  while ( (now_time.tv_sec - start_time.tv_sec) < looptime)
  {
    log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, "Debugging app 1 - loop %d", i);	      
    log4c_category_log(mycat, LOG4C_PRIORITY_ERROR,
      "some error from app1 at line %d in file %s - loop %d",
      __LINE__, __FILE__, i);
    
    sleep(3);
    
    gettimeofday(&now_time, NULL);
    i++;
  }
  
  /* Explicitly call the log4c cleanup routine */
  if ( log4c_fini()){
    printf("log4c_fini() failed");
  }
  
  return 0;
}

