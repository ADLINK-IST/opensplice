#ifndef OS_LOG_CFG_H
#define OS_LOG_CFG_H

#include <stdio.h>

extern FILE *os_open_info_file(void);
extern FILE *os_open_error_file(void);
extern int os_logprintf( FILE *log, const char *fmt, ... );

#endif
