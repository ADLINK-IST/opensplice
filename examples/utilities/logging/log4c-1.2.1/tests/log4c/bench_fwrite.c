#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <log4c/defs.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

extern char *optarg;
extern int optind, opterr, optopt;

/******************************************************************************/

#define MAX_NUM_THREADS 100
#define MSG_SIZE 128
#define NUM_MSGS 16
#define FILENAME "/var/opt/bench_fwrite.out"


#ifdef OLD_VARIADIC_MACRO
#define bench_log(args...) fprintf(stderr, args)
#else
#define bench_log(...) fprintf(stderr, __VA_ARGS__)
#endif /* OLD_VARIADIC_MACRO */


#define USAGE  "This program is a log4c developer tool used to compare fwrite() performance against fprintf()\n\n" \
		       "bench_fwrite [-f] [-n<num msgs] [-w] [-b<buffer size>]\n" \
               "             [-t<num threads>] [-m<buffer mode> [-s<msg size>]\n" \
             "  -f use a file rather than stdout--goes to /var/opt/bench_fwrite.out\n" \
             "  -w use fwrite rather than fprintf\n" \
             "  -b<buffer size> if not specified defaults to using system buffer.\n" \
             "                  if specified as zero we do setbuf NULL\n" \
             "                  if non zero we do setvbuf to that value rounded up\n" \
             "                  to the next highest BUFSIZ\n" \
             "  -m if 1 then use IOFBF mode, otherwise use IOLBF\n" \
             "  -p Add an sprintf() of the test string--this is usually required to format the \n" \
             "     buffer passed to fwrite, so it's a more realistic comparison to use this\n" \
	  		 "\n" \
			 " eg. 'bench_fwrite -s256 -f -n1000000'\n" \
			 "measures the time to write a million strings of length 256 to a file using fprintf() with the default system buffer\n" \
 	  		 "while 'bench_fwrite -s256 -f -n1000000 -w'\n" \
			 "does the same thing but using fwrite()\n"

		

/******************************************************************************/

char *g_msgbuf = NULL;
const char *g_outfilename = FILENAME;
long g_num_msgs = NUM_MSGS;
int g_usefile = 0;

int g_usefwrite = 0;
FILE *g_fp = NULL;
int g_usebuffer = 1;
int g_usemybuffer = 0;
size_t g_mybufsize = 0;
size_t g_mybufadjustedsize = 0;
char *g_myfilebuffer = NULL;
int g_numthreads = 1;
int g_bufmode = 1; /* 1==full , ow. line*/
long g_msgsize = MSG_SIZE;
char g_tmpbuf[10*1024];
int g_add_sprintf_to_fwrite = 0;

/******************************************************************************/

static char *make_msgbuf(long msgsize);
static void bench_init( int start /* non-zero for start, 0 for exit */);
static void *thread_work(void *arg);
unsigned long long gettimestamp_milis(void);

/******************************************************************************/

static void bench_init(int start) {

  if (g_usefile) {
    if ( start) {
      bench_log("  Writing to file '%s'\n", g_outfilename);
      g_fp = fopen( g_outfilename, "w+");

      if ( g_fp == NULL){
	bench_log("Failed to open '%s' for writing\n", g_outfilename);
	exit(1);
      }
      if ( g_usebuffer) {
	if ( g_usemybuffer) {
	  bench_log("  Using my buffer of adjusted size %d, mode '%s'\n", 
		    g_mybufadjustedsize, (g_bufmode == 1 ? "_IOFBF":"_IOLBF") );
	  g_myfilebuffer = (char *)malloc(g_mybufadjustedsize);
	  setvbuf(g_fp, g_myfilebuffer, (g_bufmode == 1 ? _IOFBF:_IOLBF),
		  g_mybufadjustedsize ); 
	} else {
	  bench_log("  Using system buffer of size %d\n", BUFSIZ);
	}
      } else {
	bench_log("  Unbuffered output\n");
	setbuf(g_fp, NULL); /* unbuffered */
      }
    } else {
      bench_log("  Closing file '%s'\n", g_outfilename);
      fclose(g_fp);
    }
  } else {
    if ( start ) {
      bench_log( "  Writing to 'stdout'\n");
      g_fp = stdout;
    }
  }

  if ( g_usefwrite ) {
    if ( start ){
       bench_log( "  Using fwrite\n");
       if (g_add_sprintf_to_fwrite){
	 bench_log( "  Adding sprintf of msg to help compare with fprintf\n");
       }else{
	 bench_log( "  fwite'ing directly from a buffer--no printf required\n");
       }
    }
  } else {
     if ( start ){
       bench_log( "  Using fprintf\n");
    }
  }
}

/******************************************************************************/

static char *make_msgbuf(long msgsize){
  int i = 0;
  char *s = (char *)calloc(msgsize, sizeof(char));
 
  i = 0;
  while(i < msgsize-1) {
    s[i] = 'm';
    i++;
  }

  return(s);
}

/******************************************************************************/

static inline void bench_dowrite(FILE *fp, char *msgbuf, size_t buflen){
 
   if ( !g_usefwrite ) {
     /* Don't need the len, it's a string */
     fprintf(fp, "%s\n", msgbuf);
   } else {    
     /* use fwrite */
     /* bench_log("wrting using fwrite\n");*/
     /* sprintf here to try to make it comparable
	to fprintf
     */
     if (g_add_sprintf_to_fwrite){    
       sprintf(g_tmpbuf, "%s\n", msgbuf);
     }
     fwrite(msgbuf, sizeof(char), buflen, fp);
   }
}
/******************************************************************************/

void getopts(int argc, char **argv){

  char c;

	if ( argc == 1) {
	   bench_log(USAGE);
	   exit(1);
	}

   while ((c = getopt(argc, argv, "fn:wb:t:m:s:hp")) != -1) {
    switch(c) {
    case 'f':
      g_usefile = 1;
      //printf("appender is '%s'\n",appender );
      break;
    case 'p':
      g_add_sprintf_to_fwrite = 1;
      //printf("appender is '%s'\n",appender );
      break;
    case 'm':
      g_bufmode = atoi(optarg);
      //printf("appender is '%s'\n",appender );
      break;
    case 'n':
      g_num_msgs = atol(optarg);
      // printf("priority is '%s'\n",priority );
      break;  
    case 't':
      g_numthreads = atoi(optarg);
      if ( g_numthreads <= 0 ) {
	g_numthreads = 1;
      } else if ( g_numthreads > MAX_NUM_THREADS){
	g_numthreads = MAX_NUM_THREADS;
      }
      // printf("priority is '%s'\n",priority );
      break;
    case 'w':
      g_usefwrite = 1;
      // printf("priority is '%s'\n",priority );
      break; 
    case 's':
      g_msgsize = atol(optarg);
      // printf("priority is '%s'\n",priority );
      break;
    case 'b':      
      g_mybufsize = atol(optarg);
      if ( g_mybufsize == 0 ) {
	g_usebuffer = 0; /* turn off buffering completely */
      } else {
	g_usemybuffer = 1;	
	g_mybufadjustedsize = ((BUFSIZ + g_mybufsize)/BUFSIZ)*BUFSIZ;
      }
      // printf("priority is '%s'\n",priority );
      break;
    case 'h': 
      bench_log(USAGE);
      exit(1);
      break;
    }
   }  
    
   bench_log("  Writing %ld message(s) of length %ld\n",
	  g_num_msgs,g_msgsize);

}

static void *thread_work(void *arg){
    /* int tid= *((int *)arg); */
  int msgnum = 0;

  /* bench_log("In thread %d\n", tid); */
  
  while( msgnum < g_num_msgs){

    bench_dowrite(g_fp, g_msgbuf, MSG_SIZE);
   
    msgnum++;
  }
  return(NULL);
}

unsigned long long gettimestamp_milis(){
   struct timeval time;
   unsigned long long ticks;

   (void)gettimeofday(&time, NULL);   
   ticks = (unsigned long long)time.tv_sec * 1000;  /* that's in milliseconds */
   ticks += (unsigned long long)time.tv_usec / 1000;     /* so's that */
   return ticks;
}  


/******************************************************************************/

int main(int argc, char **argv){
  int i = 0;
  unsigned long long start, end, delta;
  pthread_t tid[MAX_NUM_THREADS];
  
  bench_log("Welcome to %s:\n\n", argv[0]);

  getopts(argc, argv);
  bench_init(1 /* start */);
  g_msgbuf = make_msgbuf(g_msgsize);

  if ( g_msgsize < 5 ) {
    bench_log("  Ridiculously short message %ld--exiting\n", 
	   g_msgsize);
    exit(1);
  }
 
  bench_log("  Starting %d thread(s)\n", g_numthreads);
  start = gettimestamp_milis();

  for ( i = 0; i < g_numthreads; i++)
    pthread_create(&tid[i], NULL, thread_work,
		   (void *)&i);
  for ( i = 0; i < g_numthreads; i++)
    pthread_join(tid[i], NULL);

  end = gettimestamp_milis();
  delta = (end - start);
  bench_log( "\n  Time = %lld milisec\n", delta);
  
  bench_init(0);
  
  return(0);
}


