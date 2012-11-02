/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_LOG_CFG_H
#define OS_LOG_CFG_H

#include <stdio.h>

extern FILE *os_open_info_file(void);
extern FILE *os_open_error_file(void);
extern int os_logprintf( FILE *log, const char *fmt, ... );

#endif
