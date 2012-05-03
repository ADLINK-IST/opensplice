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
#include "os_report.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_socket.h"
#include "os_library.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef INTEGRITY
#include "os_log_cfg.h"
#endif

#define OS_REPORTSERVICES_MAX	(10)
#define OS_REPORTPLUGINS_MAX	(10)
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

typedef void *os_reportPlugin_context;

typedef int
(*os_reportPlugin_initialize)(
    const char *argument,
    os_reportPlugin_context *context);

typedef int
(*os_reportPlugin_report)(
    os_reportPlugin_context context,
    const char *report);

typedef int
(*os_reportPlugin_finalize)(
    os_reportPlugin_context context);

typedef struct os_reportPlugin_s {
    os_reportPlugin_initialize initialize_symbol;
    os_reportPlugin_report report_symbol;
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

os_reportPluginAdmin reportPluginAdmin = NULL;

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

static FILE *
os_open_file (char * file_name)
{
    FILE *logfile=NULL;
    char host[256];
    unsigned short port;

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
            logfile = fopen(file_name, "a");
        }
    }
    if ( logfile == NULL )
    {
        logfile = stderr;
    }
    return logfile;
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
                if (!OS_ISDIR (statBuf.stat_mode))
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

                if (!OS_ISDIR (statBuf.stat_mode))
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
    return result;
}


/* The result of os_report_file_path must be freed with os_free */
static char *
os_report_file_path(char * default_file, char * override_variable)
{
    char *file_dir;
    char *file_name;
    char file_path[2048];
#ifdef WIN32
    char profile_path[2048];
#endif
    char host[256];
    unsigned short port;
    int len;

    file_dir = os_getenv("OSPL_LOGPATH");
#ifdef WIN32
    if (file_dir && strcmp(file_dir, "<userprofile>") == 0)
    {
        /* This is used for win32 service use. The only env property out of the ones
        that you might expect to be defined when running as a service is USERPROFILE.
        There's no APPDATA. You also can't set a Session Manager system wide env
        REG_EXPAND_SZ value like %USERPROFILE%\foo\bar and have it expand to what
        you'd expect either. The Local Service user Environment key is a no-op.
        Trust me on this. Hence the need for this fudge. (N.B. all per WinXP) */
        file_dir = os_getenv("USERPROFILE");
        len = snprintf(profile_path, sizeof(profile_path),
                 "%s\\Application Data\\PrismTech\\OpenSpliceDDS\\%s" , file_dir, VERSION);
        /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
        if ( len < (int)sizeof(file_path) && len > -1 )
        {
            file_dir = (char*) &profile_path;
        }
        else
        {
            file_dir = NULL;
        }
    }
#endif
    if (! os_createLogDir(file_dir))
    {
#if defined VXWORKS_RTP || defined _VXWORKS
        file_dir = "/tgtsvr";
#else
        file_dir = ".";
#endif
    }
    if (override_variable != NULL)
    {
        file_name = os_getenv(override_variable);
    }
    if (file_name == NULL)
    {
        file_name = default_file;
    }
    if (strcmp(file_name, "<stderr>") != 0 && strcmp(file_name, "<stdout>") != 0)
    {
        if (sscanf (file_name, "%255[^:]:%hd", host, &port) != 2)
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

char *
os_reportGetInfoFileName()
{
    return os_report_file_path ("ospl-info.log", "OSPL_INFOFILE");
}

char *
os_reportGetErrorFileName()
{
    return os_report_file_path ("ospl-error.log", "OSPL_ERRORFILE");
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

    admin = os_malloc(sizeof(struct os_reportPluginAdmin_s));
    admin->reportArray = os_malloc(sizeof(os_reportPlugin_t) * size);

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
    char procIdentity[256];
    char threadIdentity[64];
    char node[64];
    char date_time[128];
    FILE *log;

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

    os_vsnprintf(extended_description, sizeof(extended_description)-1, description, args);
    extended_description[sizeof(extended_description)-1] = '\0';

    os_ctime_r(&ostime, date_time);
    date_time[strlen(date_time) - 1] = '\0'; /* remove \n */
    os_gethostname(node, sizeof(node)-1);
    node[sizeof(node)-1] = '\0';
    os_threadFigureIdentity(threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity[sizeof (threadIdentity)-1] = '\0';
    os_procFigureIdentity(procIdentity, sizeof (procIdentity)-1);
    procIdentity[sizeof (procIdentity)-1] = '\0';

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
            fileName,
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
    unsigned int i;
    const char *file_name = fileName;
    const char *ptr;
    char xml_description[1024];
    char extended_description[512];

#ifdef WIN32
    char file_separator = '\\';
#else
    char file_separator = '/';
#endif

    for (ptr=fileName; *ptr!=0; ptr++) {
       if (*ptr == file_separator) {
          file_name = (char *)((long)ptr+1);
       }
    }

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

    va_start(args, description);
    if (reportPluginAdmin != NULL){
       os_vsnprintf (extended_description, sizeof(extended_description)-1, description, args);
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
                fileName,
                lineNo,
                reportCode,
                os_reportTypeText[reportType]);

       for (i = 0; i < reportPluginAdmin->size; i++){
          if (reportPluginAdmin->reportArray[i] != NULL) {
             if (reportPluginAdmin->reportArray[i]->report_symbol != 0) {
                reportPluginAdmin->reportArray[i]->report_symbol
                (reportPluginAdmin->reportArray[i]->plugin_context, xml_description);
             }
          }
       }
    }
    va_end(args);

    if (doDefault)
    {
       va_start(args, description);
       os_defaultReport(reportType, reportContext, file_name, lineNo, reportCode, description, args);
       va_end(args);
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
            report->description = os_malloc(OS_MAX_DESCRIPTIONSIZE);
            if (report->description) {
                va_start(args, description);
                os_vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
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
                os_vsnprintf(report->description, OS_MAX_DESCRIPTIONSIZE-1, descriptionCopy, args);
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

 os_int32
 os_reportRegisterPlugin(
    const char *library_file_name,
    const char *initialize_method_name,
    const char *argument,
    const char *report_method_name,
    const char *finalize_method_name,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin)
 {
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_library libraryHandle;
    os_libraryAttr attr;
    os_result osr;
    os_boolean error = OS_FALSE;

    os_reportPlugin_initialize initFunction;
    os_reportPlugin_finalize finalizeFunction;
    os_reportPlugin_report reportFunction;
    os_reportPlugin_context context;
    os_reportPlugin_t rplugin;

    osr = os_libraryAttrInit(&attr);

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

    if (!error)
    {
        initFunction =  (os_reportPlugin_initialize)os_libraryGetSymbol (libraryHandle, initialize_method_name);

        if (initFunction == NULL)
        {
           OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Unable to resolve report intialize function: %s", initialize_method_name);

           error = OS_TRUE;
	 }
    }

    if (!error)
    {
        finalizeFunction = (os_reportPlugin_finalize)os_libraryGetSymbol (libraryHandle, finalize_method_name);

        if (finalizeFunction == NULL)
        {
            OS_REPORT_1 (OS_ERROR, "os_reportRegsiterPlugin", 0,
                    "Unable to resolve report finalize function: %s", finalize_method_name);

            error = OS_TRUE;
        }
    }

    if (!error)
    {
        reportFunction = (os_reportPlugin_report)os_libraryGetSymbol (libraryHandle, report_method_name);

        if (reportFunction == NULL)
        {
            OS_REPORT_1 (OS_ERROR, "os_reportRegisterPlugin", 0,
                        "Unable to resolve report Report function: %s", report_method_name);

            error = OS_TRUE;
        }
    }

    if (!error)
    {
        if (reportPluginAdmin == NULL)
        {
            reportPluginAdmin = os_reportPluginAdminNew (OS_REPORTPLUGINS_MAX);
        }

        if (reportPluginAdmin->length < reportPluginAdmin->size)
        {
            osr = initFunction (argument, &context);

            if (osr != 0)
            {
                OS_REPORT_2 (OS_ERROR, "os_reportRegisterPlugin", 0,
                              "Initialize report plugin failed : %s : Return code %d\n", initialize_method_name, osr);
                return -1;
            }

            reportPluginAdmin->reportArray[reportPluginAdmin->length] = os_malloc(sizeof(struct os_reportPlugin_s));

            rplugin = reportPluginAdmin->reportArray[reportPluginAdmin->length++];

            rplugin->initialize_symbol = initFunction;
            rplugin->report_symbol = reportFunction;
            rplugin->finalize_symbol = finalizeFunction;
            rplugin->plugin_context = context;

            *plugin = rplugin;

            if (suppressDefaultLogs)
            {
                doDefault = OS_FALSE;
            }

            return 0;
        }
    }

    OS_REPORT_1 (OS_WARNING, "os_reportRegisterPlugin", 0,
                 "Failed to register report plugin : %s", library_file_name);

    return -1;
#else
    return 0;
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
    return 0;
#endif
 }
