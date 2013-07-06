#include <stdio.h>

#include "log4c.h"

int main(int argc, char** argv){
  int rc = 0;
  log4c_category_t* mycat = NULL;
  
  if (log4c_init()){
    printf("log4c_init() failed");
    rc = 1;  
  }else{
      mycat = log4c_category_get("log4c.examples.helloworld");

      log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "Hello World!");
    
    /* Explicitly call the log4c cleanup routine */
    if ( log4c_fini()){
      printf("log4c_fini() failed");
    }
  }
  return 0;
}
