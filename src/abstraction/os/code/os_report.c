/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifdef PIKEOS_POSIX
#include <lwip_config.h>
#endif

#include "os_version.h"
#include "os_gitrev.h"
#include "os_report.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_socket.h"
#include "os_library.h"
#include "os_config.h"
#include "os_mutex.h"
#include "os.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef INTEGRITY
#include "os_log_cfg.h"
#endif

#define OS_REPORTSERVICES_MAX   (10)
#define OS_REPORTPLUGINS_MAX    (10)
#define OS_MAX_DESCRIPTIONSIZE  (2048)

typedef struct {
    void (*os_reportService)(
        os_IReportService_s reportServiceContext,
        os_reportType reportType,
        const char *reportContext,
        const char *fileName,
        os_int32 lineNo,
        os_int32 code,
        const char *description,
        va_list args);
    os_IReportService_s os_reportContext;
} os_reportServiceType;

typedef struct os_reportPlugin_s {
    os_reportPlugin_initialize initialize_symbol;
    os_reportPlugin_report report_symbol;
    /** Pointer to the function for a typed event report method */
    os_reportPlugin_typedreport typedreport_symbol;
    os_reportPlugin_finalize finalize_symbol;
    os_reportPlugin_context plugin_context;
} *os_reportPlugin_t;

typedef struct os_reportPluginAdmin_s {
    unsigned int size;
    unsigned int length;
    os_reportPlugin_t * reportArray;
} *os_reportPluginAdmin;

static os_reportServiceType os_reportServices[OS_REPORTSERVICES_MAX] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 } };

static FILE* error_log = NULL;
static FILE* info_log = NULL;

static os_boolean doDefault = OS_TRUE;

static os_int32 os_reportServicesCount = 0;

static os_mutex reportMutex;

/**
 * Count any plugins registered with an XML string Report method
 * This is only used in an optimisation so it is never decremented.
 * The code that would need to decrement it isn't thread safe so it would create
 * a risk of a failure
 */
static os_uint32 xmlReportPluginsCount = 0;

os_reportPluginAdmin reportPluginAdmin = NULL;

/**
* Process global verbosity level for OS_REPORT output. os_reportType
* values >= this value will be written.
* This value defaults to OS_INFO, meaning that all types 'above' (i.e.
* other than) OS_DEBUG will be written and OS_DEBUG will not be.
*/
os_reportType os_reportVerbosity = OS_INFO;

/**
* Whether the logfiles should be opened as append
*/
static os_boolean doAppend = OS_TRUE;

/**
* Labels corresponding to os_reportType values.
* @see os_reportType
*/
const char *os_reportTypeText [] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "API_INFO",
    "ERROR",
    "CRITICAL ERROR",
    "FATAL ERROR",
    "REPAIRED",
    "NONE"
};

static const os_char os_env_logdir[] = "OSPL_LOGPATH";
static const os_char os_env_infofile[] = "OSPL_INFOFILE";
static const os_char os_env_errorfile[] = "OSPL_ERRORFILE";
static const os_char os_env_verbosity[] = "OSPL_VERBOSITY";
static const os_char os_env_append[] = "OSPL_LOGAPPEND";
#ifdef VXWORKS_RTP
static const os_char os_env_procname[] = "SPLICE_PROCNAME";
static const os_char os_default_logdir[] = "/tgtsvr";
#else
static const os_char os_default_logdir[] = ".";
#endif

static void
os_reportSetApiInfoRec (
    const char   *reportContext,
    const char   *sourceLine,
    const char   *callStack,
    os_int32      reportCode,
    const char   *description,
    va_list       args);

#if !defined(INTEGRITY)
/**
* Am not able to use os_sockError as lib ddsosnet is supposed
* to depend on ddsos, not vice versa... arguably we should not have
* network stuff here at all.
* @see os_sockError
*/
int socketErrorNo(void)
{
#if defined (WIN32) || defined (WINCE)
    return WSAGetLastError();
#else
    return errno;
#endif
}

static FILE * open_socket (char *host, unsigned short port)
{
   FILE * file = NULL;
   struct sockaddr_in sa;
   os_socket sock;
   char* errorMessage;

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      errorMessage = os_reportErrnoToString(socketErrorNo());
      fprintf(stderr, "socket: %s\n", errorMessage);
      os_free(errorMessage);
      return NULL;
   }

   memset((char *)&sa, 0, sizeof(sa));
   sa.sin_family = AF_INET;
   sa.sin_port = htons(port);
   sa.sin_addr.s_addr = inet_addr (host);

   if (connect (sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
   {
      errorMessage = os_reportErrnoToString(socketErrorNo());
      fprintf(stderr, "connect: %s\n", errorMessage);
      os_free(errorMessage);
      return NULL;
   }
#ifdef WINCE
   file = _wfdopen ((int)sock, L"w");
#else
   file = fdopen ((int)sock, "w");
#endif

   return file;
}

static FILE *
os_open_file (char * file_name)
{
    FILE *logfile=NULL;
    char host[256];
    unsigned short port;
    os_char *dir, *file, *fmt, *str;
    os_int ret;
    os_size_t len;
    os_result res = os_resultSuccess;
#ifdef VXWORKS_RTP
    os_char serverfile[256];
    os_char *proc;
    os_char *ptr;
#endif

    /* OSPL-4002: Only OSPL_INFOFILE and OSPL_ERRORFILE can specify a host:port
                  combination. Since OSPL_LOGPATH and OSPL_INFOFILE/
                  OSPL_ERRORFILE are concatenated we need to strip the prefix
                  from file_name and then check if we should enter tcp mode.
                  This is considered a temporary workaround and will be removed
                  once the work specified in ticket OSPL-4091 is done */

    if (strcmp (file_name, "<stdout>") == 0) {
        logfile = stdout;
    } else if (strcmp (file_name, "<stderr>") == 0) {
        logfile = stderr;
    } else {
        dir = os_getenv (os_env_logdir);
        if (dir == NULL) {
            dir = (os_char *)os_default_logdir;
        }

        len = strlen (dir) + 2; /* '/' + '\0' */
        str = os_malloc (len);
        if (str != NULL) {
            (void)snprintf (str, len, "%s/", dir);
            dir = os_fileNormalize (str);
            os_free (str);
            if (dir == NULL) {
                res = os_resultFail;
            }
        } else {
            dir = NULL;
            res = os_resultFail;
        }

        if (res != os_resultFail) {
            file = file_name;
            len = strlen (dir);
            if (strncmp (dir, file_name, len) == 0) {
                file = file_name + len;
            }
            os_free (dir);

#ifdef VXWORKS_RTP
            /* FIXME: It isn't pretty, but we must remain bug compatible! */
            str = NULL;
            ptr = os_index (file, '%');
            if (ptr != NULL && *ptr == 's') {
                proc = os_getenv (os_env_procname);
                if (proc != NULL) {
                    len = (strlen (file)-1) + strlen (proc); /* -"%s" +'\0' */
                    str = os_malloc (len);
                    if (str != NULL) {
                        (void)snprintf (str, len, file, proc);
                        file = str;
                    } else {
                        res = os_resultFail;
                    }
                }
            }

            if (res == os_resultSuccess) {
                fmt = "%255[^:]:%hu:%255[^:]";
                ret = sscanf (file, fmt, host, &port, serverfile);
                if (ret != 3) {
                    fmt = "%255[^:]:%hu";
                    ret = sscanf (file, fmt, host, &port);
                }

                if (str != NULL) {
                    os_free (str);
                }
            }
#else
            fmt = "%255[^:]:%hu";
            ret = sscanf (file, fmt, host, &port);
#endif
            file = NULL;

            if (res == os_resultSuccess) {
                if (ret >= 2) {
                    logfile = open_socket (host, port);
                    if (logfile == NULL) {
                        res = os_resultFail;
                    }

#ifdef VXWORKS_RTP
                    if (ret == 3) {
                        fprintf (logfile, "FILENAME:%s\n", serverfile);
                    }
#endif
                } else {
                    logfile = fopen (file_name, "a");
                }
            }
        }
    }

    return logfile;
}

static void os_close_file (char * file_name, FILE *file)
{
  if (strcmp(file_name, "<stderr>") != 0 && strcmp(file_name, "<stdout>") != 0)
  {
    fclose(file);
  }
}

/**
* Read environment properties. In particular ones that can't be left until
* there is a requirement to log.
*/
void
os_reportInit(os_boolean forceReInit)
{
    static os_boolean doneOnce = OS_FALSE;
    char *envValue;
    os_mutexAttr attr;
    os_result osr;

    if (!doneOnce || forceReInit)
    {
        if (!doneOnce)
        {
           osr = os_mutexAttrInit(&attr);
           if(osr == os_resultSuccess)
           {
              attr.scopeAttr = OS_SCOPE_PRIVATE;
              osr = os_mutexInit(&reportMutex, &attr);
           }
           if(osr != os_resultSuccess)
           {
              OS_REPORT(OS_WARNING, "os_reportInit", 0,
                        "Unable to create report mutex");
           }
        }

        doneOnce = OS_TRUE;
        envValue = os_getenv(os_env_verbosity);
        if (envValue != NULL)
        {
            if (os_reportSetVerbosity(envValue) == os_resultFail)
            {
                OS_REPORT_3(OS_WARNING, "os_reportInit", 0,
                        "Cannot parse report verbosity %s value \"%s\","
                        " reporting verbosity remains %s", os_env_verbosity, envValue, os_reportTypeText[os_reportVerbosity]);
            }
        }

        if (os_procIsOpenSpliceDomainDaemon())
        {
            /** @todo dds2881 - Change default to OS_FALSE ? */
            doAppend = OS_TRUE;
        }

        envValue = os_getenv(os_env_append);
        if (envValue != NULL)
        {
            os_boolean shouldAppend;
            if (os_configIsTrue(envValue, &shouldAppend) == os_resultFail)
            {
                OS_REPORT_2(OS_WARNING, "os_reportInit", 0,
                        "Cannot parse report %s value \"%s\","
                        " reporting append mode unchanged", os_env_append, envValue);
            }
            else
            {
                os_reportSetDoAppend(shouldAppend);
            }
        }
    }
}

void os_reportExit()
{
  char * name;

  os_mutexDestroy(&reportMutex);

  if (error_log)
  {
    name = os_reportGetInfoFileName();
    os_close_file(name, error_log);
    os_free (name);
    error_log = NULL;
  }

  if (info_log)
  {
    name = os_reportGetErrorFileName();
    os_close_file(name, info_log);
    os_free (name);
    info_log = NULL;
  }
}

static int
os_createLogDir (const char *name)
{
    int result;
    os_result status;
    char dirName[OS_PATH_MAX];
    struct os_stat statBuf;
    unsigned long i;

    memset(dirName, 0, OS_PATH_MAX);
    result = 0;

    if(name)
    {
        result = 1;

        for(i=0; name[i] != '\0' && result; i++)
        {
            if((name[i] == OS_FILESEPCHAR) && (i != 0))
            {
                status = os_stat(dirName, &statBuf);

                if (status != os_resultSuccess)
                {
                    os_mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
                    status = os_stat(dirName, &statBuf);
                }
                if (status != os_resultSuccess || !OS_ISDIR (statBuf.stat_mode))
                {
#ifdef WIN32
                    if((strlen(dirName) == 2) && (dirName[1] == ':'))
                    {
                        /*This is a device like for instance: 'C:'*/
                    }
                    else
                    {
                        result = 0;
                    }
#else
                    result = 0;
#endif
                }
            }
            dirName[i] = name[i];
        }
        if(result)
        {
            if(dirName[i-1] != OS_FILESEPCHAR)
            {
                status = os_stat(dirName, &statBuf);

                if (status != os_resultSuccess)
                {
                    os_mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
                    status = os_stat(dirName, &statBuf);
                }

                if (status != os_resultSuccess || !OS_ISDIR (statBuf.stat_mode))
                {
#ifdef WIN32
                    if((strlen(dirName) == 2) && (dirName[1] == ':'))
                    {
                        /*This is a device like for instance: 'C:'. Check if it exists...*/
                        dirName[2] = OS_FILESEPCHAR;
                        status = os_stat(dirName, &statBuf);

                        if(status == os_resultFail)
                        {
                            result = 0;
                        }
                    }
                    else
                    {
                        result = 0;
                    }
#else
                    result = 0;
#endif
                }
            }
        }
    }
    else
    {
        result = 0;
    }

    /* os_access write access check fails for kernel mode vxworks 6.8 and 6.9
     * 	even for writeable directories.
     */
#if !( defined (VXWORKS_68) || defined (VXWORKS_69) ) || !defined (_WRS_KERNEL)
    if(result)
    {
        status = os_access(name, 2); /*Check whether dir is writable*/

        if(status != os_resultSuccess)
        {
#ifdef WIN32
            if((strlen(dirName) == 2) && (dirName[1] == ':'))
            {
                /*This is a device like for instance: 'C:'. Check if it exists...*/
                dirName[2] = OS_FILESEPCHAR;
                status = os_stat(dirName, &statBuf);

                if(status == os_resultFail)
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }
#else
            result = 0;
#endif
        }
    }
#endif
    return result;
}


/**
* Return either a log file path string or a pseudo file name/path value
* like <stdout> or <stderr>.
* The result of os_report_file_path must be freed with os_free
* @param override_variable An environment variable name that may hold a filename
* or pseudo filename. If this var is set, and is not a pseudo filename,
* the value of this var will be added to the value of env variable
* OSPL_LOGPATH (if set or './' if not) to create the log file path.
* @param default_file If override_variable is not defined in the environment
* this is the filename used.
*/
static char *
os_report_file_path(char * default_file, char * override_variable)
{
    char *file_dir;
    char file_path[2048];
    char host[256];
    char server_file[256];
    unsigned short port;
    int len;
    char *file_name = NULL;

    file_dir = os_getenv(os_env_logdir);
    if (! os_createLogDir(file_dir))
    {
        file_dir = (os_char *)os_default_logdir;
    }
    if (override_variable != NULL)
    {
        file_name = os_getenv(override_variable);
#if defined VXWORKS_RTP
	{
	  /** &todo dds3335/dds3299 Allow substitution of %s for the process name */
	    char *ptok;
	    char *splice_procname;
	    if ( file_name != NULL
		 && ((ptok = os_index( file_name, '%' ) ) != NULL)
		 && ptok[1] == 's'
		 && ( splice_procname = os_getenv( os_env_procname ) ) != NULL )
	    {
		char * new_name = os_malloc( strlen( file_name )
					  + strlen( splice_procname )
					  -1 );
		sprintf( new_name, file_name, splice_procname );
		if (sscanf (new_name, "%255[^:]:%hu", host, &port) != 2
		    && sscanf (new_name, "%255[^:]:%hu:%255[^:]", host, &port, server_file) != 3)
		{
		   len = snprintf(file_path, sizeof(file_path), "%s/%s", file_dir, new_name);
		   /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
		   if ( len < (int)sizeof(file_path) && len > -1 )
		   {
		     os_free( new_name );
		     return os_fileNormalize(file_path);
		   }
		   else
		   {
		     return( new_name );
		   }
		}
		else
		{
		   return( new_name );
		}
	    }
	}
#endif
    }
    if (file_name == NULL)
    {
        file_name = default_file;
    }
    if (strcmp(file_name, "<stderr>") != 0 && strcmp(file_name, "<stdout>") != 0)
    {
        if (sscanf (file_name, "%255[^:]:%hu", host, &port) != 2
	    && sscanf (file_name, "%255[^:]:%hu:%255[^:]", host, &port, server_file) != 3)
        {
            len = snprintf(file_path, sizeof(file_path), "%s/%s", file_dir, file_name);
            /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
            if ( len < (int)sizeof(file_path) && len > -1 )
            {
               return os_fileNormalize(file_path);
            }
        }
    }

    return os_strdup (file_name);
}

/**
* Get the destination for logging error reports. Env property OSPL_INFOFILE and
* OSPL_LOGPATH controls this value.
* If OSPL_INFOFILE is not set & this process is an OpenSplice service default
* to logging to a file named ospl-info.log, otherwise
* use standard out.
* @see os_report_file_path
*/
char *
os_reportGetInfoFileName()
{
    char* file_name;
    char procIdentity[256];
    static os_boolean doneOnce = OS_FALSE;

    os_reportInit(OS_FALSE);

    file_name = os_report_file_path ("ospl-info.log", (os_char *)os_env_infofile);
    /* @todo dds2881 - Uncomment below & remove above to enable application default error logging to stderr */
    /* file_name = os_report_file_path (os_procIsOpenSpliceService() ? "ospl-info.log" : "<stdout>", os_env_infofile);*/

    os_procFigureIdentity(procIdentity, sizeof (procIdentity)-1);
    procIdentity[sizeof (procIdentity)-1] = '\0';
    if (!doneOnce)
    {
        doneOnce = OS_TRUE;
        if (!doAppend)
        {
            os_remove (file_name);
        }
    }

    return file_name;
}

/**
* Get the destination for logging error reports. Env property OSPL_ERRORFILE and
* OSPL_LOGPATH controls this value.
* If OSPL_ERRORFILE is not set & this process is an OpenSplice service default
* to logging to a file named ospl-error.log, otherwise
* use standard error.
* @see os_report_file_path
*/
char *
os_reportGetErrorFileName()
{
    char* file_name;
    static os_boolean doneOnce = OS_FALSE;

    file_name = os_report_file_path ("ospl-error.log", (os_char *)os_env_errorfile);
    /* @todo dds2881 - Uncomment below & remove above to enable application default error logging to stderr */
    /* file_name = os_report_file_path (os_procIsOpenSpliceService() ? "ospl-error.log" : "<stderr>", os_env_errorfile); */

    if (!doneOnce)
    {
        doneOnce = OS_TRUE;
        if (!doAppend)
        {
            os_remove (file_name);
        }
    }

    return file_name;
}

static FILE *
os_open_info_file (void)
{
    char * name;
    FILE * file;

    name = os_reportGetInfoFileName();
    file = os_open_file(name);
    os_free (name);
    return file;
}

static FILE *
os_open_error_file (void)
{
    char * name;
    FILE * file;

    name = os_reportGetErrorFileName();
    file = os_open_file(name);
    os_free (name);
    return file;
}

os_reportPluginAdmin
os_reportPluginAdminNew(
    unsigned int size)
{
    os_reportPluginAdmin admin = NULL;
    unsigned int i;

    admin = (os_reportPluginAdmin) os_malloc(sizeof(struct os_reportPluginAdmin_s));
    admin->reportArray = (os_reportPlugin_t*) os_malloc(sizeof(os_reportPlugin_t) * size);

    for (i = 0; i < size; i++) {
      admin->reportArray[i] = NULL;
    }

    admin->size = size;
    admin->length =0;

    return admin;
}

void
os_reportDisplayLogLocations()
{
    char * infoFileName;
    char * errorFileName;

    infoFileName = os_reportGetInfoFileName();
    errorFileName = os_reportGetErrorFileName();
    printf ("\nInfo  log : %s\n", infoFileName);
    printf ("Error log : %s\n", errorFileName);
    os_free (infoFileName);
    os_free (errorFileName);
}

#endif

static void
os_defaultReport(
    os_reportEventV1 event)
{
    os_time ostime;
    char node[64];
    char date_time[128];
    FILE *log;
    time_t tt;


    switch (event->reportType) {
    case OS_DEBUG:
    case OS_INFO:
    case OS_WARNING:
      /* Check info_file is NULL here to keep user loggging */
      /* plugin simple in integrity */
      if ( info_log == NULL )
      {
        info_log = os_open_info_file();
      }
      log = info_log;
      break;
    case OS_API_INFO:
    case OS_ERROR:
    case OS_CRITICAL:
    case OS_FATAL:
    case OS_REPAIRED:
    default:
      /* Check error_file is NULL here to keep user loggging */
      /* plugin simple in integrity */
      if ( error_log == NULL )
      {
        error_log = os_open_error_file();
      }
      log = error_log;
      break;
    }

    ostime = os_timeGet();
    tt = ostime.tv_sec;
    if (strftime(date_time, sizeof(date_time), "%a %b %d %H:%M:%S %Z %Y", localtime(&tt)) == 0) {
        date_time[0] = '\0';
    }

    os_gethostname(node, sizeof(node)-1);
    node[sizeof(node)-1] = '\0';

    os_mutexLock(&reportMutex);
#ifdef INTEGRITY
    os_logprintf( log,
#else
    fprintf(log,
#endif
            "========================================================================================\n"
            "Report      : %s\n"
            "Date        : %s\n"
            "Description : %s\n"
            "Node        : %s\n"
            "Process     : %s\n"
            "Thread      : %s\n"
            "Internals   : %s/%s/%s/%s/%s/%d/%d/%d.%09d\n",
            os_reportTypeText[event->reportType],
            date_time,
            event->description,
            node,
            event->processDesc,
            event->threadDesc,
            OSPL_VERSION_STR,
            OSPL_INNER_REV_STR,
            OSPL_OUTER_REV_STR,
            event->reportContext,
            event->fileName,
            event->lineNo,
            event->code,
            ostime.tv_sec,
            ostime.tv_nsec);
#ifndef INTEGRITY
    fflush (log);
#endif
    os_mutexUnlock(&reportMutex);
}

void
os_report(
    os_reportType reportType,
    const char   *reportContext,
    const char   *fileName,
    os_int32      lineNo,
    os_int32      reportCode,
    const char   *description,
    ...)
{
    va_list args;
    unsigned int i;
    const char *file_name = fileName;
    const char *ptr;
    char xml_description[1024];
    char extended_description[1024];
    char threadIdentity[64];
    char procIdentity[256];
    struct os_reportEventV1_s eventDetails = { OS_REPORT_EVENT_V1, /* version */
                                               0, /* reportType */
                                               NULL, /* reportContext */
                                               NULL, /* fileName */
                                               0, /* lineNo */
                                               0, /* code */
                                               NULL, /*description */
                                               NULL, /* threadDesc */
                                               NULL /*processDesc */
                                              };

#ifdef WIN32
    char file_separator = '\\';
#else
    char file_separator = '/';
#endif

    eventDetails.description = (char*) &extended_description;
    eventDetails.threadDesc = (char*) &threadIdentity;
    eventDetails.processDesc = (char*) &procIdentity;

    if (reportType < os_reportVerbosity)
    {
        /* This level / type of report is below the process output suppression threshold. */
        return;
    }

    va_start(args, description);
    os_vsnprintf(extended_description, sizeof(extended_description)-1, description, args);
    va_end(args);
    extended_description[sizeof(extended_description)-1] = '\0';
    os_threadFigureIdentity(threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity[sizeof (threadIdentity)-1] = '\0';
    os_procFigureIdentity(procIdentity, sizeof (procIdentity)-1);
    procIdentity[sizeof (procIdentity)-1] = '\0';
    eventDetails.reportType = reportType;
    eventDetails.reportContext = reportContext;
    eventDetails.lineNo = lineNo;
    eventDetails.code = reportCode;

    for (ptr=fileName; *ptr!=0; ptr++) {
       if (*ptr == file_separator) {
          file_name = (char *)((os_address)ptr+1);
       }
    }

    eventDetails.fileName = file_name;

    if (os_reportServicesCount != 0) {
       for (i = 0; i < OS_REPORTSERVICES_MAX; i++) {
          if (os_reportServices[i].os_reportContext != 0) {
             /* Must call va_start and va_end before and after each call to the plugged in services */
             va_start(args, description);
             os_reportServices[i].os_reportService(os_reportServices[i].os_reportContext,
                                                   reportType, reportContext, file_name, lineNo, reportCode,
                                                   description, args);
             va_end(args);
          }
       }
    }

    if (reportPluginAdmin != NULL){
        if (xmlReportPluginsCount > 0)
        {
            /* print format the XML string only if at least one plugin
             * has been registered with an XML Report method */
            os_sprintf (xml_description,
                        "<%s>\n"
                        "<DESCRIPTION>%s</DESCRIPTION>\n"
                        "<CONTEXT>%s</CONTEXT>\n"
                        "<FILE>%s</FILE>\n"
                        "<LINE>%d</LINE>\n"
                        "<CODE>%d</CODE>\n"
                        "</%s>\n",
                        os_reportTypeText[reportType],
                        extended_description,
                        reportContext,
                        file_name,
                        lineNo,
                        reportCode,
                        os_reportTypeText[reportType]);
            xml_description[sizeof(xml_description)-1] = '\0';
        }
        for (i = 0; i < reportPluginAdmin->size; i++){
            if (reportPluginAdmin->reportArray[i] != NULL) {
                if (reportPluginAdmin->reportArray[i]->report_symbol != NULL) {
                    reportPluginAdmin->reportArray[i]->report_symbol
                    (reportPluginAdmin->reportArray[i]->plugin_context, xml_description);
                }

                if (reportPluginAdmin->reportArray[i]->typedreport_symbol != NULL) {
                    reportPluginAdmin->reportArray[i]->typedreport_symbol
                    (reportPluginAdmin->reportArray[i]->plugin_context, (os_reportEvent) &eventDetails);
                }
            }
        }
    }

    if (doDefault)
    {
       os_defaultReport(&eventDetails);
    }

    va_start(args, description);
    if (reportType == OS_API_INFO) {
       char sourceLine[512];

       snprintf(sourceLine, sizeof(sourceLine), "%s::%d", file_name, lineNo);
       sourceLine[sizeof(sourceLine)-1] = '\0';
       os_reportSetApiInfoRec(reportContext, sourceLine, NULL, reportCode,
                              description, args);
    }
    va_end(args);
 }

void
os_reportSetApiInfoContext(
    const char   *reportContext)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report) {
        if (report->reportContext) {
            os_free(report->reportContext);
            report->reportContext = NULL;
        }
        if (reportContext) {
            report->reportContext = os_strdup(reportContext);
        }
    }
}

void
os_reportSetApiInfoLine(
    const char   *sourceLine)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report) {
        if (report->sourceLine) {
            os_free(report->sourceLine);
            report->sourceLine = NULL;
        }
        if (sourceLine) {
            report->sourceLine = os_strdup(sourceLine);
        }
    }
}

void
os_reportSetApiInfoStack(
    const char   *callStack)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report) {
        if (report->callStack) {
            os_free(report->callStack);
            report->callStack = NULL;
        }
        if (callStack) {
            report->callStack = os_strdup(callStack);
        }
    }
}

void
os_reportSetApiInfoCode(
    os_int32 reportCode)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report) {
        report->reportCode = reportCode;
    }
}

void
os_reportSetApiInfoDescription(
    const char   *description,
    ...)
{
    os_reportInfo *report;
    va_list args;
    char * descriptionCopy = NULL; /* since description may point to report->description*/

    if (description) {
        descriptionCopy = os_strdup(description);
    }

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report) {
        if (report->description) {
            os_free(report->description);
            report->description = NULL;
        }
        if (descriptionCopy) {
            report->description = (char*) os_malloc(OS_MAX_DESCRIPTIONSIZE);
            if (report->description) {
                va_start(args, description);
                os_vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
                va_end(args);
            }
        }
    }
    os_free(descriptionCopy);
}

static void
os_reportSetApiInfoRec(
    const char   *reportContext,
    const char   *sourceLine,
    const char   *callStack,
    os_int32      reportCode,
    const char   *description,
    va_list       args)
{
    os_reportInfo *report;
    char * descriptionCopy = NULL; /* since description may point to report->description*/

    if (description) {
        descriptionCopy = os_strdup(description);
    }

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report == NULL) {
        report = (os_reportInfo *)os_threadMemMalloc(OS_THREAD_API_INFO, sizeof(os_reportInfo));
        if (report) memset(report, 0, sizeof(os_reportInfo));
    }
    if (report) {
        if (report->reportContext) {
            os_free(report->reportContext);
            report->reportContext = NULL;
        }
        if (reportContext) {
            report->reportContext = os_strdup(reportContext);
        }
        if (report->sourceLine) {
            os_free(report->sourceLine);
            report->sourceLine = NULL;
        }
        if (sourceLine) {
            report->sourceLine = os_strdup(sourceLine);
        }
        if (report->callStack) {
            os_free(report->callStack);
            report->callStack = NULL;
        }
        if (callStack) {
            report->callStack = os_strdup(callStack);
        }
        report->reportCode = reportCode;
        if (report->description) {
            os_free(report->description);
            report->description = NULL;
        }
        if (descriptionCopy) {
            report->description = (char*) os_malloc(OS_MAX_DESCRIPTIONSIZE);
            if (report->description) {
                os_vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
            }
        }
    }
    os_free(descriptionCopy);
}

void
os_reportSetApiInfo(
    const char   *reportContext,
    const char   *sourceLine,
    const char   *callStack,
    os_int32      reportCode,
    const char   *description,
    ...)
{
    va_list args;

    va_start(args, description);
    os_reportSetApiInfoRec(reportContext, sourceLine, callStack, reportCode, description, args);
    va_end(args);
}

os_reportInfo *
os_reportGetApiInfo(void)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);

    return report;
}

void
os_reportClearApiInfo(void)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);

    if (report->reportContext) {
        os_free(report->reportContext);
        report->reportContext = NULL;
    }
    if (report->sourceLine) {
        os_free(report->sourceLine);
        report->sourceLine = NULL;
    }
    if (report->callStack) {
        os_free(report->callStack);
        report->callStack = NULL;
    }
    if (report->description) {
        os_free(report->description);
        report->description = NULL;
    }
    os_threadMemFree(OS_THREAD_API_INFO);
}

os_int32
os_registerReportService(
    os_IReportService_s reportServiceContext,
    void (*reportService)(
        os_IReportService_s reportServiceContext,
        os_reportType reportType,
        const char *reportContext,
        const char *fileName,
        os_int32 lineNo,
        os_int32 code,
        const char *description,
        va_list args)
    )
{
    os_int32 i;

    for (i = 0; i < OS_REPORTSERVICES_MAX; i++) {
        if (os_reportServices[i].os_reportContext == 0) {
            os_reportServices[i].os_reportContext = reportServiceContext;
            os_reportServices[i].os_reportService = reportService;
            os_reportServicesCount++;
            return os_reportServicesCount;
        }
    }
    return -1;
}

os_int32
os_unregisterReportService (
    os_IReportService_s reportServiceContext)
{
    os_int32 i;

    for (i = 0; i < OS_REPORTSERVICES_MAX; i++) {
        if (os_reportServices[i].os_reportContext == reportServiceContext) {
            os_reportServices[i].os_reportContext = 0;
            os_reportServices[i].os_reportService = 0;
            os_reportServicesCount--;
            return os_reportServicesCount;
        }
    }
    return -1;
}

 os_int32
 os_reportRegisterPlugin(
    const char *library_file_name,
    const char *initialize_method_name,
    const char *argument,
    const char *report_method_name,
    const char *typedreport_method_name,
    const char *finalize_method_name,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin)
 {
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_library libraryHandle;
    os_libraryAttr attr;
    os_result osr;
    os_boolean error = OS_FALSE;
    os_int32 initResult;

    os_reportPlugin_initialize initFunction;
    os_reportPlugin_finalize finalizeFunction;
    os_reportPlugin_report reportFunction = NULL;
    os_reportPlugin_typedreport typedReportFunction = NULL;

    osr = os_libraryAttrInit(&attr);
    libraryHandle = NULL;

    if (library_file_name != NULL )
    {
       libraryHandle = os_libraryOpen (library_file_name, &attr);
    }

    if (libraryHandle == NULL)
    {
       OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Unable to load library: %s", library_file_name);

       error = OS_TRUE;
    }

    if (!error && typedreport_method_name == NULL && report_method_name == NULL)
    {
       OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "At least one of TypedReport or Report symbole must be defined");

       error = OS_TRUE;
    }

    if (!error)
    {
        initFunction =  (os_reportPlugin_initialize)os_fptr(os_libraryGetSymbol (libraryHandle, initialize_method_name));

        if (initFunction == NULL)
        {
           OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Unable to resolve report intialize function: %s", initialize_method_name);

           error = OS_TRUE;
        }
    }

    if (!error)
    {
        finalizeFunction = (os_reportPlugin_finalize)os_fptr(os_libraryGetSymbol (libraryHandle, finalize_method_name));

        if (finalizeFunction == NULL)
        {
            OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Unable to resolve report finalize function: %s", finalize_method_name);

            error = OS_TRUE;
        }
    }

    if (!error )
    {
        if (report_method_name != NULL)
        {
            reportFunction = (os_reportPlugin_report)os_fptr(os_libraryGetSymbol (libraryHandle, report_method_name));

            if (reportFunction == NULL)
            {
                OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                            "Unable to resolve report Report function: %s", report_method_name);

                error = OS_TRUE;
            }
            else
            {
                ++xmlReportPluginsCount;
            }
        }

        if (typedreport_method_name != NULL)
        {
            typedReportFunction = (os_reportPlugin_typedreport)os_fptr(os_libraryGetSymbol (libraryHandle, typedreport_method_name));

            if (typedReportFunction == NULL)
            {
                OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                            "Unable to resolve report TypedReport function: %s", typedreport_method_name);

                error = OS_TRUE;
            }
        }
    }

    if (!error)
    {
        initResult = os_reportInitPlugin(argument,
                                       initFunction,
                                       finalizeFunction,
                                       reportFunction,
                                       typedReportFunction,
                                       suppressDefaultLogs,
                                       plugin);
        if (initResult)
        {
            OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                            "Plug-in initialization method failed : %s", initialize_method_name);
        }
        else
        {
            return 0;
        }
    }

    OS_REPORT_1 (OS_WARNING, "os_reportRegisterPlugin", 0,
                 "Failed to register report plugin : %s", library_file_name);

    return -1;
#else
   if (library_file_name != NULL ) {
       OS_REPORT_1(OS_ERROR, "os_reportRegisterPlugin", 0, "Unable to register report plugin: %s because \
                   product was not built with INCLUDE_PLUGGABLE_REPORTING enabled", library_file_name);
   } else {
       OS_REPORT(OS_ERROR, "os_reportRegisterPlugin", 0, "Unable to register report plugin because \
                 product was not built with INCLUDE_PLUGGABLE_REPORTING enabled");
   }
   (void)library_file_name;
   (void)initialize_method_name;
   (void)argument;
   (void)report_method_name;
   (void)finalize_method_name;
   (void)suppressDefaultLogs;
   (void)plugin;
   return -1;
#endif
 }

 os_int32
 os_reportInitPlugin(
    const char *argument,
    os_reportPlugin_initialize initFunction,
    os_reportPlugin_finalize finalizeFunction,
    os_reportPlugin_report reportFunction,
    os_reportPlugin_typedreport typedReportFunction,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin)
 {
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_reportPlugin_context context;
    os_reportPlugin_t rplugin;
    os_result osr;

    if (reportPluginAdmin == NULL)
    {
        reportPluginAdmin = os_reportPluginAdminNew (OS_REPORTPLUGINS_MAX);
    }

    if (reportPluginAdmin->length < reportPluginAdmin->size)
    {
        osr = initFunction (argument, &context);

        if (osr != 0)
        {
            OS_REPORT_1 (OS_ERROR, "os_reportInitPlugin", 0,
                            "Initialize report plugin failed : Return code %d\n", osr);
            return -1;
        }

        reportPluginAdmin->reportArray[reportPluginAdmin->length] = os_malloc(sizeof(struct os_reportPlugin_s));

        rplugin = reportPluginAdmin->reportArray[reportPluginAdmin->length++];

        rplugin->initialize_symbol = initFunction;
        rplugin->report_symbol = reportFunction;
        rplugin->typedreport_symbol = typedReportFunction;
        rplugin->finalize_symbol = finalizeFunction;
        rplugin->plugin_context = context;

        *plugin = rplugin;

        if (suppressDefaultLogs)
        {
            doDefault = OS_FALSE;
        }
        return 0;
    }
    else
    {
         OS_REPORT_1 (OS_ERROR, "os_reportInitPlugin", 0,
                            "Initialize report plugin failed. Max plug-ins (%d) exceeded.\n", reportPluginAdmin->size);
    }
    return -1;
#else
    (void)argument;
    (void)initFunction;
    (void)finalizeFunction;
    (void)reportFunction;
    (void)typedReportFunction;
    (void)suppressDefaultLogs;
    (void)plugin;
    return -1;
#endif
 }

 os_int32
 os_reportUnregisterPlugin(
    os_reportPlugin plugin)
 {
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_reportPlugin_t rplugin;
    os_result osr;

    if (reportPluginAdmin != NULL){
       rplugin = (os_reportPlugin_t)plugin;

       /* Verify if the plugin is a valid address within the bounds of the reportArray.
        */
       if ((rplugin >= reportPluginAdmin->reportArray[0]) &&
           (rplugin <= reportPluginAdmin->reportArray[reportPluginAdmin->length - 1]))
       {
          os_reportPlugin_finalize finalize_symbol = rplugin->finalize_symbol;
          os_reportPlugin_context plugin_context = rplugin->plugin_context;
          /* Note that this isn't thread safe!
           * Problems minimized by first setting report_symbol to NULL;
           */
          rplugin->report_symbol = NULL;
          rplugin->typedreport_symbol = NULL;
          rplugin->initialize_symbol = NULL;
          rplugin->finalize_symbol = NULL;
          rplugin->plugin_context = NULL;
          if (finalize_symbol) {
             osr = finalize_symbol(plugin_context);
             if (osr != 0){
                OS_REPORT_1 (OS_ERROR,
                             "os_reportUnregisterPlugin", 0,
                             "Finalize report plugin failed : Return code %d\n",
                             osr);
                return -1;
             }
          }
          return 0;
       } else {
          OS_REPORT (OS_WARNING,
                     "os_reportUnregisterPlugin", 0,
                     "Finalize report plugin failed");
       }
    } else {
       OS_REPORT (OS_WARNING,
                  "os_reportUnregisterPlugin", 0,
                  "Finalize report plugin failed");
    }
    return -1;
#else
    (void)plugin;
    return -1;
#endif
 }

/**
* Convert the specified socket error to a string.
* @return Heap allocated string representation. Caller owns and must os_free
* @see os_sockError
* @param errNo The error number
*/
char*
os_reportErrnoToString(int errNo)
{
    char* result;
#if defined (WIN32) || defined (WINCE)
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS |
                    FORMAT_MESSAGE_MAX_WIDTH_MASK,
                    NULL,
                    errNo,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0, NULL);
    result = os_strdup(lpMsgBuf);
    LocalFree(lpMsgBuf);
#else
    /* @todo This isn't threadsafe. Should replace w/ strerror_r */
    result = os_strdup(strerror(errNo));
#endif
    return result;
}

/**
* Overrides the current minimum output level to be reported from
* this process.
* @param newVerbosity String holding either an integer value corresponding
* to an acceptable (in range) log verbosity or a string verbosity 'name'
* like 'ERROR' or 'warning' or 'DEBUG' or somesuch.
* @return os_resultFail if the string contains neither of the above;
* os_resultSuccess otherwise.
*/
os_result
os_reportSetVerbosity(
    const char* newVerbosity)
{
    long verbosityInt;
    os_result result;

    result = os_resultFail;
    verbosityInt = strtol(newVerbosity, NULL, 0);

    os_reportInit(OS_FALSE);
    if (verbosityInt == 0
        && strcmp("0", newVerbosity)
        )
    {
        /* Conversion from int failed. See if it's one of the string forms. */
        while (verbosityInt < (long) (sizeof(os_reportTypeText) / sizeof(os_reportTypeText[0])))
        {
            if (os_strcasecmp(newVerbosity, os_reportTypeText[verbosityInt]) == 0)
            {
                break;
            }
            ++verbosityInt;
        }
    }

    if (verbosityInt >= 0 && verbosityInt < (long) (sizeof(os_reportTypeText) / sizeof(os_reportTypeText[0])))
    {
        os_reportVerbosity = verbosityInt;
        result = os_resultSuccess;
    }

    return result;
}

/**
* Sets whether this process should delete any pre-existing log fuile or not prior
* to opening a file for writing.
* @param shouldAppend If true then the file should not be deleted.
*/
void
os_reportSetDoAppend(
    os_boolean shouldAppend)
{
    os_reportInit(OS_FALSE);

    doAppend = shouldAppend;
}
