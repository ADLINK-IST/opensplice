static const char version[] = "$Id$";

/*
 * sd_xplatform.c
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <stdio.h>
#include <string.h>
#include "log4c/defs.h"

#include "sd_xplatform.h"

/****************** getopt *******************************/

#define	EOF	(-1)
 
 int sd_opterr = 1;
 int sd_optind = 1;
 int sd_optopt = 0;
 char *sd_optarg = NULL;
 int _sp = 1;
 
#define warn(a,b,c)fprintf(stderr,a,b,c)
 
 void
 getopt_reset(void)
 {
 	sd_opterr = 1;
 	sd_optind = 1;
 	sd_optopt = 0;
 	sd_optarg = NULL;
 	_sp = 1;
 }
 
 int
 sd_getopt(int argc, char *const *argv, const char *opts)
 {
 	char c;
 	char *cp;
 
 	if (_sp == 1) {
 		if (sd_optind >= argc || argv[sd_optind][0] != '-' ||
 		    argv[sd_optind] == NULL || argv[sd_optind][1] == '\0')
 			return (EOF);
 		else if (strcmp(argv[sd_optind], "--") == 0) {
 			sd_optind++;
 			return (EOF);
 		}
 	}
 	sd_optopt = c = (unsigned char)argv[sd_optind][_sp];
 	if (c == ':' || (cp = strchr(opts, c)) == NULL) {
 		if (opts[0] != ':')
 			warn("%s: illegal option -- %c\n", argv[0], c);
 		if (argv[sd_optind][++_sp] == '\0') {
 			sd_optind++;
 			_sp = 1;
 		}
 		return ('?');
 	}
 
 	if (*(cp + 1) == ':') {
 		if (argv[sd_optind][_sp+1] != '\0')
 			sd_optarg = &argv[sd_optind++][_sp+1];
 		else if (++sd_optind >= argc) {
 			if (opts[0] != ':') {
 				warn("%s: option requires an argument"
 				    " -- %c\n", argv[0], c);
 			}
 			_sp = 1;
 			sd_optarg = NULL;
 			return (opts[0] == ':' ? ':' : '?');
 		} else
 			sd_optarg = argv[sd_optind++];
 		_sp = 1;
 	} else {
 		if (argv[sd_optind][++_sp] == '\0') {
 			_sp = 1;
 			sd_optind++;
 		}
 		sd_optarg = NULL;
 	}
 	return (c);
 }
 
/*****************************  gettimeofday *******************/


#ifdef _WIN32

#if 0 /* also in winsock[2].h */
#define _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;
    long tv_usec;
    long tv_usec;
};
#endif /* _TIMEVAL_DEFINED */

int sd_gettimeofday(LPFILETIME lpft, void* tzp) {

    if (lpft) {
        GetSystemTimeAsFileTime(lpft);
    }
    /* 0 indicates that the call succeeded. */
    return 0;
}
#endif /* _WIN32 */

/*
 * Placeholder for WIN32 version to get last changetime of a file
 */
#ifdef WIN32
int sd_stat_ctime(const char* path, time_t* time)
{   
    HANDLE file_handle;
    FILETIME last_changed;
    ULARGE_INTEGER integer_time;
    int result = -1;
    
    file_handle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        if (GetFileTime(file_handle, NULL, NULL, &last_changed))
        {
            integer_time.LowPart = last_changed.dwLowDateTime;
            integer_time.HighPart = last_changed.dwHighDateTime;
            /* time_t was 32 bits on old VCs so still better convert. Math is pretty cheap. */
            *time = (integer_time.QuadPart - 116444736000000000) / 10000000;
            result = 0;
        }
        CloseHandle(file_handle);
    }
    return result; 
}
#else
int sd_stat_ctime(const char* path, time_t* time)
{
	struct stat astat;
	int statret=stat(path,&astat);
	if (0 != statret)
	{
		return statret;
	}
	*time=astat.st_ctime;
	return statret;
}
#endif


