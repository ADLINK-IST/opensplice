#include "os_report.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_socket.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef INTEGRITY
#include <os_log_cfg.h>
#endif

#define OS_REPORTSERVICES_MAX	(10)
#define OS_MAX_DESCRIPTIONSIZE  (2048)

#define SERV_IP "10.1.0.3"
#define INFO_PORT 20006
#define ERROR_PORT 20007

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

static os_int32 os_reportServicesCount = 0;

static const char *os_reportTypeText [] = {
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL ERROR",
    "FATAL ERROR",
    "REPAIRED",
    "API_INFO"
};

static void
os_reportSetApiInfoRec (
    const char   *reportContext,
    const char   *sourceLine,
    const char   *callStack,
    os_int32      reportCode,
    const char   *description,
    va_list	  args);

#if !defined(INTEGRITY)
static FILE * open_socket (char *host, unsigned short port)
{
   FILE * file = NULL;
   struct sockaddr_in sa;
   int sock;

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket");
      return NULL;
   }

   memset((char *)&sa, 0, sizeof(sa));
   sa.sin_family = AF_INET;
   sa.sin_port = htons(port);
   sa.sin_addr.s_addr = inet_addr (host);

   if (connect (sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
   {
      perror("connect");
      return NULL;
   }

   file = fdopen (sock, "w");

   return file;
}

static FILE * os_open_info_file (void)
{
   FILE *logfile=NULL;
   char *file_dir;
   char *file_name;
   char file_path[2048];
   char host[256];
   unsigned short port;
   int len;

   file_dir = os_getenv("OSPL_LOGPATH");
   if (file_dir == NULL) 
   {
#ifdef VXWORKS_RTP
      file_dir = "/tgtsvr/";
#else
      file_dir = "./";
#endif
   }
   file_name = os_getenv("OSPL_INFOFILE");
   if (file_name == NULL) 
   {
      file_name = "ospl-info.log";
   }
   if (strcmp(file_name, "<stderr>") != 0) 
   {
      if (strcmp(file_name, "<stdout>") == 0) 
      {
         logfile = stdout;
      } 
      else if (sscanf (file_name, "%255[^:]:%hd", host, &port) == 2)
      {
         logfile = open_socket (host, port);
      }
      else 
      {
         len = snprintf(file_path, sizeof(file_path), "%s/%s", file_dir, file_name);
         /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
         if ( len < (int)sizeof(file_path) && len > -1 ) 
         {
            char * filename = os_fileNormalize(file_path);
            logfile = fopen(filename, "a");
            os_free(filename);
         }
      }
   }
   if ( logfile == NULL ) 
   {
      logfile = stderr;
   }
   return( logfile );
}

static FILE * os_open_error_file (void)
{
   FILE *logfile=NULL;
   char *file_dir;
   char *file_name;
   char file_path[2048];
   char host[256];
   unsigned short port;
   int len;

   file_dir = os_getenv("OSPL_LOGPATH");
   if (file_dir == NULL) 
   {
#ifdef VXWORKS_RTP
      file_dir = "/tgtsvr/";
#else
      file_dir = "./";
#endif
   }
   file_name = os_getenv("OSPL_ERRORFILE");
   if (file_name == NULL) 
   {
      file_name = "ospl-error.log";
   }
   if (strcmp(file_name, "<stderr>") != 0) 
   {
      if (strcmp(file_name, "<stdout>") == 0) 
      {
         logfile = stdout;
      } 
      else if (sscanf (file_name, "%255[^:]:%hd", host, &port) == 2)
      {
         logfile = open_socket (host, port);
      }
      else 
      {
         len = snprintf(file_path, sizeof(file_path), "%s/%s", file_dir, file_name);
         /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
         if ( len < (int)sizeof(file_path) && len > -1 ) 
         {
            char * filename;
            filename = os_fileNormalize(file_path);
            logfile = fopen(filename, "a");
         }
      }
   }
   if ( logfile == NULL ) 
   {
      logfile = stderr;    
   }
   return(logfile);
}
#endif

static void
os_defaultReport(
    os_reportType reportType,
    const char   *reportContext,
    const char   *fileName,
    os_int32      lineNo,
    os_int32      reportCode,
    const char   *description,
    va_list 	  args)
{
    os_time ostime;
    char extended_description[512];
    char procIdentity[64];
    char threadIdentity[64];
    char node[64];
    char date_time[128];
    FILE *log;
    const char *file_name = fileName;
    const char *ptr;


    switch (reportType) {
    case OS_INFO:
    case OS_API_INFO:
    case OS_WARNING:
      /* Check info_file is NULL here to keep user loggging */
      /* plugin simple in integrtity */
      if ( info_log == NULL ) 
      {
        info_log = os_open_info_file();
      }
      log = info_log;
      break;
    case OS_ERROR:
    case OS_CRITICAL:
    case OS_FATAL:
    case OS_REPAIRED:
    default:
      /* Check error_file is NULL here to keep user loggging */
      /* plugin simple in integrtity */
      if ( error_log == NULL ) 
      {
        error_log = os_open_error_file();
      }
      log = error_log;
      break;
    }

    ostime = os_timeGet();

    vsnprintf(extended_description, sizeof(extended_description)-1, description, args);
    extended_description[sizeof(extended_description)-1] = '\0';

    os_ctime_r(&ostime, date_time);
    date_time[strlen(date_time) - 1] = '\0'; /* remove \n */
    os_gethostname(node, sizeof(node)-1);
    node[sizeof(node)-1] = '\0';
    os_threadFigureIdentity(threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity[sizeof (threadIdentity)-1] = '\0';
    os_procFigureIdentity(procIdentity, sizeof (procIdentity)-1);
    procIdentity[sizeof (procIdentity)-1] = '\0';

    for (ptr=fileName; *ptr!=0; ptr++) {
        if (*ptr == '/') {
            file_name = (char *)((long)ptr+1);
        }
    }

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
            "Internals   : %s/%s/%s/%d/%d/%d\n",
            os_reportTypeText[reportType],
            date_time,
            extended_description,
            node,
            procIdentity,
            threadIdentity,
            VERSION,
            reportContext,
            file_name,
            lineNo,
            reportCode,
            ostime.tv_nsec);
#ifndef INTEGRITY
    fflush (log);
#endif
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
    os_int32 i;

    va_start(args, description);
    if (os_reportServicesCount != 0) {
        for (i = 0; i < OS_REPORTSERVICES_MAX; i++) {
            if (os_reportServices[i].os_reportContext != 0) {
                os_reportServices[i].os_reportService(os_reportServices[i].os_reportContext,
                    reportType, reportContext, fileName, lineNo, reportCode,
                    description, args);
            }
        }
    } else {
        os_defaultReport(reportType, reportContext, fileName, lineNo, reportCode, description, args);
    }
    va_end(args);
    va_start(args, description);
    if (reportType == OS_API_INFO) {
        char sourceLine[512];

        snprintf(sourceLine, sizeof(sourceLine), "%s::%d", fileName, lineNo);
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
            report->description = os_malloc(OS_MAX_DESCRIPTIONSIZE);
            if (report->description) {
                va_start(args, description);
                vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
                va_end(args);  
            }
        }
    }
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
        memset(report, 0, sizeof(os_reportInfo));
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
            report->description = os_malloc(OS_MAX_DESCRIPTIONSIZE);
            if (report->description) {
                vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
            }
            os_free(descriptionCopy);
        }
    }
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

