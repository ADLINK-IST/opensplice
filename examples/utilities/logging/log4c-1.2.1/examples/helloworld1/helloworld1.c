#include <stdio.h>

#define MYLOG_CATEGORY_NAME "log4c.examples.helloworld"
#include "mylog.h"

int main(int argc, char** argv){
  int rc = 0;
  
  if (mylog_init()){
    printf("mylog_init() failed");
    rc = 1;  
  }else{

    MYLOGMSG(LOG4C_PRIORITY_ERROR, "Hello World!");
    
    /* Explicitly call the log4c cleanup routine */
    if ( mylog_fini()){
      printf("mylog_fini() failed");
    }
  }
  return rc;
}
