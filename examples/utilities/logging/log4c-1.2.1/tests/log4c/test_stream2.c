static const char version[] = "$Id$";

/*
 * test_category.c
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <log4c/appender.h>
#include <log4c/appender_type_stream2.h>

#include <log4c/layout.h>
#include <log4c/category.h>
#include <log4c/init.h>
#include <sd/test.h>
#include <sd/factory.h>
#include <stdio.h>
#include <stdlib.h>

static log4c_category_t* root = NULL;
static log4c_category_t* sub1 = NULL;
static log4c_category_t* sub1sub2 = NULL;

/******************************************************************************
The stream2 appender is used as follows:

1. Get an appender of type "stream2":
 stream2_appender = log4c_appender_get(<appender name>);
 log4c_appender_set_type(stream2_appender,
			    log4c_appender_type_get("stream2"));
  
2a.Go straight into logging with this appender by setting it as the appender
for a category:
 log4c_category_set_appender(root, stream2_appender);

 In this case the appender will  be configured automatically with default 
values: 
  . the filename is the same as the <appender name>
  .  the file is opened in "w+" mode
  . the default system buffer is used (cf; setbuf() )

OR

2b. Configure the appender using the stream2 setter functions and then
go into logging by setting the appender as the appender for a category:

If you set a file pointer yourself you are reposnsible for opening, 
setting any opttions and closing the file pointer.  In this case the name of
the log file will be the file name of myfp and not the name of the appender:

 log4c_stream2_set_fp(stream2_appender,myfp);
 log4c_category_set_appender(root, stream2_appender);

By default the stream2 file pointer is in buffered mode.
If you want to set the mode for the stream2 file pointer to unbuffered then
do it this way:

 log4c_stream2_set_flags(stream2_appender, LOG4C_STREAM2_UNBUFFERED);
 log4c_category_set_appender(root, stream2_appender);

I you are using your own file pointer then you can set the buffer directly
and you do not need to use this option.


*/


/******************************************************************************/
static void log4c_print(FILE* a_fp)
{   
    extern sd_factory_t* log4c_category_factory;
    extern sd_factory_t* log4c_appender_factory;
    extern sd_factory_t* log4c_layout_factory;

    sd_factory_print(log4c_category_factory, a_fp);	fprintf(a_fp, "\n");
    sd_factory_print(log4c_appender_factory, a_fp);	fprintf(a_fp, "\n");
    sd_factory_print(log4c_layout_factory, a_fp);	fprintf(a_fp, "\n");
}

/******************************************************************************/
#define foo(cat, level) \
{ \
    fprintf(sd_test_out(a_test), "\n# "#cat" "#level" (priority = %s)\n", \
	    log4c_priority_to_string(log4c_category_get_priority(cat))); \
    log4c_category_##level(cat, #cat" "#level); \
}

/******************************************************************************/

static int test0(sd_test_t* a_test, int argc, char* argv[])
{   
    log4c_print(sd_test_out(a_test));
    return 1;
}

/******************************************************************************/
/*
  This tests case 2b above (setting our own file pointer)
*/
static int test1(sd_test_t* a_test, int argc, char* argv[])
{
    log4c_appender_t* stream2_appender = NULL;

    log4c_category_set_priority(root, LOG4C_PRIORITY_TRACE);
  
    stream2_appender = 
	log4c_appender_get("stream2_appender");
    log4c_appender_set_type(stream2_appender, 
			    log4c_appender_type_get("stream2"));
    log4c_stream2_set_fp(stream2_appender, sd_test_out(a_test));
    log4c_category_set_appender(root, stream2_appender);
        
    foo(root, error);
    foo(root, warn);
 
    return 1;
}

/******************************************************************************/
/*
  This tests case 2b above (setting the fp mode to unbuffered) 
*/
static int test2(sd_test_t* a_test, int argc, char* argv[])
{
    log4c_appender_t* stream2_appender = NULL;
    int flags;

    log4c_category_set_priority(sub1, LOG4C_PRIORITY_TRACE);
  
    stream2_appender = 
	log4c_appender_get("stream2_appender");
    log4c_appender_set_type(stream2_appender, &log4c_appender_type_stream2);
    log4c_stream2_set_flags(stream2_appender,LOG4C_STREAM2_UNBUFFERED);
    flags = log4c_stream2_get_flags(stream2_appender);
    fprintf(sd_test_out(a_test), "stream2 flags '%d'\n",flags);
    log4c_category_set_appender(sub1, stream2_appender);
        
    foo(sub1, error);
    foo(sub1, warn);
 
    return 1;
}

/******************************************************************************//*
  This tests case 2a above (use stream2 default config values)
*/
static int test3(sd_test_t* a_test, int argc, char* argv[])
{
    log4c_appender_t* stream2_appender = NULL;
    char testlogname[1024];

    log4c_category_set_priority(root, LOG4C_PRIORITY_TRACE);
  
    sprintf(testlogname, "%s.log", sd_test_get_name(a_test));
    stream2_appender = 
	log4c_appender_get(testlogname);
    log4c_appender_set_type(stream2_appender,
			    log4c_appender_type_get("stream2"));
    log4c_category_set_appender(root, stream2_appender);
        
    foo(root, error);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    foo(root, warn);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    return 1;
}

/******************************************************************************//*
  This tests case tests opening and closing the stream2 
several times....to test for mem leaks or corruption
*/
static int test4(sd_test_t* a_test, int argc, char* argv[])
{
    log4c_appender_t* stream2_appender = NULL;
    char testlogname[1024];

    log4c_category_set_priority(root, LOG4C_PRIORITY_TRACE);
  
    sprintf(testlogname, "%s.1", sd_test_get_name(a_test));
    stream2_appender = 
	log4c_appender_get(testlogname);
    log4c_appender_set_type(stream2_appender,
			    log4c_appender_type_get("stream2"));
    log4c_category_set_appender(root, stream2_appender);
        
    foo(root, error);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    foo(root, warn);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);

    /* Now close it....*/
    log4c_appender_close(stream2_appender );
    
    /* Now open it....*/    
    foo(root, warn);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    foo(root, warn);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    
    /* Now close it....*/
    log4c_appender_close(stream2_appender );

    /* Now open it....*/
    foo(root, info);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);
    foo(root, warn);
    fprintf(sd_test_out(a_test),"# check %s for this message\n",
	    testlogname);		 
    /* Now close it....*/ 

    return 1;
}

/******************************************************************************/
int main(int argc, char* argv[])
{    
    int ret;
    sd_test_t* t = sd_test_new(argc, argv);
    
    if ( log4c_init() > 0 ) {
	fprintf(stderr, "Failed to init log4c...exiting\n");
	exit(1);
    }
    
    root = log4c_category_get("root");
    sub1 = log4c_category_get("sub1");
    sub1sub2 = log4c_category_get("sub1.sub2"); 
    
    sd_test_add(t, test0);    
    sd_test_add(t, test1);
    sd_test_add(t, test2);   
    sd_test_add(t, test3);  
    sd_test_add(t, test4);

    ret = sd_test_run(t, argc, argv);

    sd_test_delete(t);

    log4c_fini();

    return ! ret;
}
