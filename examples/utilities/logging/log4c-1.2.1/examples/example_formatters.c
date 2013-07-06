/******************************************************************************
 *
 * Part of the log4c examples.
 *
 * Along with example_appenders.c this file is used to create a small
 * library of custom appenders and formatters.
 *
 * This library is excercised using application_2 and a sample log4crc
 * config file.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <log4c.h>

/* Defined in example_appenders.c */
extern int init_example_appenders(void);

/**********************************************************************
 *
 * Formatted to put [category] out at the front of the message
 *
 **********************************************************************/
static const char* cat_format(
    const log4c_layout_t*       a_layout,
    const log4c_logging_event_t*a_event)
{
    static char buffer[4096];

    /*
     * For this formatter we put the category up front in the log message
     */
    sprintf(buffer, "[%s][LINE:%d][FILE:%s] %s", a_event->evt_category,
    	a_event->evt_loc->loc_line, a_event->evt_loc->loc_file, a_event->evt_msg);

    return buffer;
}

const log4c_layout_type_t log4c_layout_type_cat  = {
   "s13_cat",
   cat_format,
};


static const char* none_format(
    const log4c_layout_t*       a_layout,
    const log4c_logging_event_t*a_event)
{
    static char buffer[4096];
    return buffer;
}

const log4c_layout_type_t log4c_layout_type_none  = {
  "s13_none",
  none_format,
};


/**********************************************************************/
/*
 * Formatted to mock up an xml format.
 *
 **********************************************************************/
static const char* xml_format(
    const log4c_layout_t*       a_layout,
    const log4c_logging_event_t*a_event)
{
    static char buffer[4096];

    /*
     * For this formatter we put the category up front in the log message
     */
    sprintf(buffer, "<logmessage><category>%s</category><message>%s</message></logmessage>", a_event->evt_category,  a_event->evt_msg);

    return buffer;
}

const log4c_layout_type_t log4c_layout_type_xml = {
    "s13_xml",
     xml_format,
};




/*****************************/
/*
 * Here provide an init routine for this lib 
 *
******************************/
static const log4c_layout_type_t * const layout_types[] = {
    &log4c_layout_type_xml,
    &log4c_layout_type_none,
    &log4c_layout_type_cat    
};
static int nlayout_types =
	(int)(sizeof(layout_types) / sizeof(layout_types[0]));


int init_example_formatters(){

  int rc = 0; int i = 0;
	
  for (i = 0; i < nlayout_types; i++) 
     log4c_layout_type_set(layout_types[i]);

  return(rc);

}


int init_examples_lib() {

	init_example_formatters();
	init_example_appenders();

  return(0);
}


