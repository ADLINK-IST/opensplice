#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <log4c/defs.h>
#include <log4c/appender.h>
#include <log4c/category.h>
#include <log4c/init.h>
#include <log4c/appender_type_stream2.h>
#include <sd/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sd/sd_xplatform.h>

/******************************************************************************/

typedef XP_UINT64 usec_t;
static usec_t my_utime(void)
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
#define MSG_SIZE 128
#define NUM_MSGS 16

#ifdef _WIN32
#define display_time(name,start,stop,elapsed, avg) \
fprintf(stderr,"%s: (start %I64u stop %I64u) elapsed %I64u us - average %I64u us\n\n", \
	name, start, stop, elapsed, avg)
#else
#define display_time(name,start,stop,elapsed, avg) \
fprintf(stderr,"%s: (start %llu stop %llu) elapsed %llu us - average %llu us\n\n", \
	name,start, stop, elapsed, avg)
#endif

#define USAGE  "Usage: bench [-n] [-h] <num msgs> [<msg size>]\n\n" \
"This program runs a sequence of timed calls to log4c logging calls.\n" \
"This allows, for example, the performance of different appenders\n" \
"or the performance of a given appender with different options to be\n" \
"compared.\n\n" \
"The results are sent to stderr so you can collect them in a file by doing\n" \
"something like this: './bench 128 2> results.txt'\n\n" \
"For covenience, we allow the appenders that write to the screen to be\n" \
"turned off using the -n option.  By default they are run.\n\n" \
"The default msg size is 128.\n\n" \
"-n  do not run the timed tests that write to the screen\n" \
"-h  display this help message\n"

/******************************************************************************/
int g_doall = 0;
int g_noscreen_appenders = 0;
long g_num_msgs = NUM_MSGS;
long g_msgsize = MSG_SIZE;

static log4c_category_t* mmap = NULL;
static log4c_category_t* stream = NULL;
static log4c_category_t* catstream2 = NULL;
static log4c_category_t* catstream_file = NULL;

/******************************************************************************/
void getopts(int argc, char **argv){

  char c;

  if ( argc == 1) {
      fprintf(stderr,USAGE);
      exit(1);
  }  

   while ((c = SD_GETOPT(argc, argv, "nh")) != -1) {
    switch(c) {   
    case 'n':
      g_noscreen_appenders = 1;
      break;
    case 'h': 
      fprintf(stderr, USAGE);
      exit(1);
      break;
    }
   }    
   
   /* Pick up the number of msgs and the size */
   if ( SD_OPTIND < argc ){
       g_num_msgs = atol(argv[SD_OPTIND]);
       if ( SD_OPTIND+1 < argc ){
	    g_msgsize =  atol(argv[SD_OPTIND+1]);
       }
   }

   fprintf(stderr, "  Writing %ld message(s) of length %ld\n",
	  g_num_msgs,g_msgsize);
   if ( g_noscreen_appenders){
     fprintf(stderr, "  Not running tests that log to the screen\n\n");
   }

}

int main(int argc, char* argv[]){
    int flags = 0; 
    char* buffer  = NULL;
	int i;
	usec_t start;
	usec_t stop;
	char *test_name;

	/* declare and get a reference to some unconfigured appenders */
    log4c_appender_t* mmap_appender = log4c_appender_get("bench.mmap");
    log4c_appender_t* stream_appender = log4c_appender_get("stdout");
    log4c_appender_t* stream2_appender = log4c_appender_get("bench.stream2");
    log4c_appender_t* streamfile_appender = log4c_appender_get("bench.stream");

	/*
	 * Obligatory log4c init call
	 */
    log4c_init();

	/*
	 * Get some categories 
	*/
    mmap = log4c_category_get("mmap");
    stream = log4c_category_get("stream");
  
    catstream2 = log4c_category_get("stream2");
    catstream_file = log4c_category_get("stream_file");
 
	/*
	 * Read command line options
	*/
    getopts(argc, argv);

	/*
	 * Configure the categories and appenders
	*/
    log4c_appender_set_type(mmap_appender, log4c_appender_type_get("mmap"));
    log4c_category_set_appender(mmap, mmap_appender);
    log4c_category_set_appender(stream, stream_appender);

    log4c_appender_set_type(stream2_appender,
			    log4c_appender_type_get("stream2"));
    log4c_category_set_appender(catstream2, stream2_appender);
    
     log4c_category_set_appender(catstream_file, streamfile_appender);
    
    log4c_category_set_priority(log4c_category_get("root"),
				LOG4C_PRIORITY_ERROR);

	/*
	 * Get a buffer for the message
	*/
    buffer = (char*) malloc(g_msgsize * sizeof(char));    
    memset(buffer, 'X', g_msgsize);
    buffer[g_msgsize - 1] = '\0';       
   
	/*
	 * Calibration: do a couple of 1 second sleeps to make sure
	 * the timing routines are in working order.
	*/
	start = my_utime();
	for (i = 0; i < 2; i++){ sleep(1);}
	stop = my_utime();
	display_time("calibration 2 x 1 second sleep calls", start, stop, (stop-start), (stop-start)/2);

    /* logs that write to the screen, if required */
    if ( !g_noscreen_appenders){

		/*
		 * fprintf writing to the screen
		*/
		start = my_utime();
		for (i = 0; i < g_num_msgs; i++){ fprintf(stdout, "%s\n", buffer);}	
		stop = my_utime();
		display_time("fprintf", start, stop, (stop-start), (stop-start)/g_num_msgs);
	 
		/*
		 * log4c writing to the screen
		*/
		start = my_utime();
		for (i = 0; i < g_num_msgs; i++){
			log4c_category_error(stream, "%s", buffer);
		}	
		stop = my_utime();
		display_time("fprintf", start, stop, (stop-start),
			(stop-start)/g_num_msgs);
    }
    /* Logs that write to files */
    
	/*
	 * mmap appender writing to bench.mmap
	 *
	 * On windows as this type is not implemented it makes pretty much
	 * null calls to log4c--so it does nothing but is safe.
	*/
	start = my_utime();
	for (i = 0; i < g_num_msgs; i++){ log4c_category_error(mmap, "%s", buffer);}
	stop = my_utime();
	display_time("mmap", start, stop, (stop-start), 
		(stop-start)/g_num_msgs);

	/*
	 * stream appender writing to bench.stream
	*/
	start = my_utime();
	for (i = 0; i < g_num_msgs; i++){ 
		log4c_category_error(catstream_file, "%s", buffer);
	}
	stop = my_utime();
	display_time("stream_file", start, stop, (stop-start), 
		(stop-start)/g_num_msgs);
    
    /* 
	 * stream2 appender writing to bench.stream2
	 * in buffered mode 
     */
    log4c_appender_open(stream2_appender);
    flags = log4c_stream2_get_flags(stream2_appender);  
    
	start = my_utime();
	for (i = 0; i < g_num_msgs; i++){ 
		log4c_category_error(catstream2, "%s", buffer);
	}
	stop = my_utime();
	test_name = (flags & LOG4C_STREAM2_UNBUFFERED ?
					"stream2 unbuffered -":"stream2 buffered -");
	display_time(test_name,start, stop, (stop-start),(stop-start)/g_num_msgs);   
    log4c_appender_close(stream2_appender);    

	/* 
	 * stream2 appender writing to bench.stream2
	 * in unbuffered mode -- for comparison with buffered mode
	 * test above
    */
    log4c_appender_open(stream2_appender);
    log4c_stream2_set_flags(stream2_appender, LOG4C_STREAM2_UNBUFFERED);
    flags = log4c_stream2_get_flags(stream2_appender);    
	start = my_utime();
	for (i = 0; i < g_num_msgs; i++){ 
		log4c_category_error(catstream2, "%s", buffer);
	}
	stop = my_utime();
	display_time((flags & LOG4C_STREAM2_UNBUFFERED ? "stream2 unbuffered -":"stream2 buffered -"),
		start, stop, (stop-start),(stop-start)/g_num_msgs);      
	log4c_appender_close(stream2_appender); 


	/*
	 * Obligatory log4c cleanup call
	*/
    log4c_fini();

    return 0;
}

