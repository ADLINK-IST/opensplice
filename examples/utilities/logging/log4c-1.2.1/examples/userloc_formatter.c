/******************************************************************************
 *
 * Part of the log4c examples.
 *
 * Along with example_appenders.c this file is used to create a small
 * library of custom appenders and formatters.
 *
 * This library is excercised using application_3 and a sample log4crc
 * config file.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <log4c.h>
#include "application_3.h"

/**********************************************************************
 *
 * Formatted to look for extended user location info
 *
 **********************************************************************/
static const char* userloc_format(
    const log4c_layout_t*       a_layout,
    const log4c_logging_event_t*a_event)
{
    static char buffer[4096];
    user_locinfo_t* uloc = NULL;

    sd_debug("Formatter s13_userloc checking location info for userdata %X",a_event->evt_loc->loc_data);
    if (a_event->evt_loc->loc_data != NULL)
    {
	sd_debug("Formatter s13_userloc getting a valid user location info pointer");
        uloc = (user_locinfo_t*) a_event->evt_loc->loc_data;
        sprintf(buffer, "[%s][HOST:%s][PID:%i][FILE:%s][LINE:%i][MSG:%s]",
		a_event->evt_category,  
		uloc->hostname, uloc->pid, a_event->evt_loc->loc_file,
		a_event->evt_loc->loc_line,a_event->evt_msg);

    }
    else
    {
        sprintf(buffer, "[%s]::[FILE:%s][LINE:%i][MSG::%s]", 
		a_event->evt_category,  
		a_event->evt_loc->loc_file,
		a_event->evt_loc->loc_line,a_event->evt_msg);
    }
    return buffer;
}

const log4c_layout_type_t log4c_layout_type_userloc  = {
   "s13_userloc",
   userloc_format,
};

/*****************************/
/*
 * Here provide an init routine for this lib 
 *
******************************/
static const log4c_layout_type_t * const layout_types[] = {
    &log4c_layout_type_userloc,
};
static int nlayout_types =
	(int)(sizeof(layout_types) / sizeof(layout_types[0]));


int init_userloc_formatters(){

  int rc = 0; int i = 0;
	
  for (i = 0; i < nlayout_types; i++) 
     log4c_layout_type_set(layout_types[i]);

  return(rc);

}


