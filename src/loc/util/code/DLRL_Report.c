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
/*  std C includes */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*  os  abstraction includes */
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "os_process.h"
#include "os_time.h"
#include "os_abstract.h"

/*  dlrl includes */
#include "DLRL_Report.h"

/*  constants */
#define MAX_FILTER          100     /*  number of allocated filters */
#define MAX_INDENTS         100     /*  number of allocated thread indents */
#define LOCATION_SPACE      40      /*  allowed space for filename and linenumber */

static os_mutex mutex;
static int mutexInitialised = 0;

/*  types */
typedef struct
{
    int infoType;
    char *filter_str;
}   DLRL_Report_FilterPair;

typedef struct
{
    os_threadId threadId;
    int         indent;
}   DLRL_Report_ThreadIdent;


/*  local functions */
static char *
DLRL_Report_infoType2String(
    DLRL_Report_InfoType infoType);

static int
DLRL_Report_string2Filter(
char *str);

static void
DLRL_Report_addFilter(
    int infoType,
    char *filter_str);

static int
DLRL_Report_checkFilter(
    DLRL_Report_InfoType infoType,
    const char *fileName);

static void
DLRL_Report_readOptions(
    char *inifile);

static void
DLRL_Report_openInfoLog(
    void);

static void
DLRL_Report_open_report_log(
    void);


/*  local global variables */
static FILE* info_log = NULL;
static FILE* report_log = NULL;

static DLRL_Report_FilterPair filter[MAX_FILTER];
static int nr_of_filters = 0;

static DLRL_Report_ThreadIdent indent[MAX_INDENTS];
static int nr_of_indents = 0;

static const char *reportTypeText [] = {
    "WARNING",
    "ERROR",
    "CRITICAL ERROR",
    "FATAL ERROR",
    "REPAIRED"
};


void
DLRL_Report_openInfoLog(
    void)
{
    char *file_dir;
    char file_path[2048];
    char * filename;
    int id = os_procIdToInteger (os_procIdSelf());


    file_dir = os_getenv ("OSPL_OUTER_HOME");
    if (file_dir == NULL)
    {
        file_dir = os_getenv ("SPLICE_LOGPATH");
        if (file_dir == NULL)
        {
#ifdef VXWORKS_RTP
            file_dir = "/tgtsvr";
#else
            file_dir = "./";
#endif
	}
    }
    /*  open logfile */
    os_sprintf(file_path, "%s/dlrl_trace_%d.log", file_dir, (int)os_procIdSelf());
    filename = os_fileNormalize(file_path);
    info_log = fopen (filename, "a");
    os_free(filename);
}


void
DLRL_Report_open_report_log(
    void)
{
    char *file_dir;
    char *file_name;
    char file_path[2048];

    file_dir = os_getenv ("SPLICE_LOGPATH");
	if (file_dir == NULL)
    {
	    file_dir = "./";
	}
    file_name = os_getenv ("SPLICE_DLRL_ERROR_FILE");
	if (file_name == NULL)
    {
	    file_name = "ospl-dlrl-error.log";
	}
	if (strcmp(file_name, "<stderr>") == 0)
    {
	    report_log = stderr;
	}
    else if (strcmp(file_name, "<stdout>") == 0)
    {
	    report_log = stdout;
	}
    else if (file_dir)
    {
        char * filename;
	    snprintf (file_path, sizeof(file_path), "%s/%s", file_dir, file_name);
        filename = os_fileNormalize(file_path);
	    report_log = fopen (filename, "a");
        os_free(filename);
    }
}


char*
DLRL_Report_infoType2String(
    DLRL_Report_InfoType infoType)
{
    switch (infoType)
    {
        case INF_API:
            return "API";
            break;

        case INF_CALLBACK:
            return "CALLBACK";
            break;

        case INF_ENTITY:
            return "ENTITY";
            break;

        case INF_DCPS:
            return "DCPS";
            break;

        case INF:
            return "INF";
            break;

        case INF_REF_COUNT:
            return "REF_COUNT";
            break;

        default:
            return "";
            break;
    }
}

int
DLRL_Report_string2Filter(
    char *str)
{

    assert(str);

    if (!os_strcasecmp(str, "INF_API"))
    {
        return INF_API;
    }
    else if (!os_strcasecmp(str, "INF_CALLBACK"))
    {
        return INF_CALLBACK;
    }
    else if (!os_strcasecmp(str, "INF_ENTITY"))
    {
        return INF_ENTITY;
    }
    else if (!os_strcasecmp(str, "INF_DCPS"))
    {
        return INF_DCPS;
    }
    else if (!os_strcasecmp(str, "INF_FLOW"))
    {
        /*  FLOW enables ENTER, EXIT AND OBJECT */
        return INF_ENTER|INF_EXIT|INF_OBJECT;
    }
    else if (!os_strcasecmp(str, "INF"))
    {
        return INF;
    }
    else if (!os_strcasecmp(str, "INF_REF_COUNT"))
    {
        return INF_REF_COUNT;
    }
    return 0;
}

void
DLRL_Report_addFilter(
    int infoType,
    char *filter_str)
{
    assert(filter_str);

    filter[nr_of_filters].infoType = infoType;
    filter[nr_of_filters].filter_str = os_strdup(filter_str);
    nr_of_filters++;
}

int
DLRL_Report_checkFilter(
    DLRL_Report_InfoType infoType,
    const char *fileName)
{
    int i;

    assert(fileName);

    for (i = 0; i < nr_of_filters; i++)
    {
        /*  check if current infotype bit is set in filter && (filename contains filterstring || filterstring == *) */
        if ((filter[i].infoType & infoType) && (strstr(fileName, filter[i].filter_str) ||
                (0 == strcmp(filter[i].filter_str, "*"))))
        {
            return 1;
        }
    }
    return 0;
}

void
DLRL_Report_readOptions(
    char *inifile)
{
    char line[256], substr[50], *p;
    int ready, filter;
    FILE *fp;
    char * filename;

    assert(inifile);

    filename = os_fileNormalize(inifile);
    fp = fopen(filename, "r");
    os_free(filename);

    if(fp)
    {
        while ((fgets(line, 256, fp) != NULL) && (nr_of_filters < MAX_FILTER))
        {
            ready = 0;
            filter = 0;

            p = line;
            while (!ready)
            {
                sscanf(p, "%[^+ \t]", substr);
                filter += DLRL_Report_string2Filter(substr);
                p += strlen(substr);
                if (*p !='+')
                    ready = 1;
                else p++;
            }
            sscanf(p, "%s", substr);
            DLRL_Report_addFilter(filter, substr);
        }
        fclose(fp);
    }
}

void
DLRL_Report_printLocationInfo(
    char *buffer,
    const char *fileName,
    int lineNo)
{
    static char *lastFile = NULL;
    const char *strippedFileName = fileName + strlen("../../code/");
    int len, offset = 0;

    assert(buffer);
    assert(fileName);

    /*  initialize the buffer with some spaces */
    os_sprintf(buffer, "                                                                ");
    if (lastFile != fileName)
    {
        /*  trace is in another file => log filename */
        len = strlen(strippedFileName);
        /*  if the filename is longer than 30 then only print the last part of the filename  */
        /* as this is the most significant part */
        if (len > 30)
        {
            offset = len - 30;
        }
        os_sprintf(buffer, "%s                            ", strippedFileName + offset);

        /*  store the current filename */
        lastFile = (char *)fileName;
    }
    /*  print line number */
    os_sprintf(buffer + 32, "%d                                             ", lineNo);
}

void
DLRL_Report_info(
    const char *fileName,
    int lineNo,
    const char *function,
    ...)
{
    DLRL_Report_InfoType infoType;
    /*  variable argument list */
    va_list args;
    /*  some statics */
    static os_threadId currentThread;
    static int indentIndex;
    char buffer[2000], *p, *fmt, *outer_home;
    int i, ready;

    assert(fileName);
    assert(function);

    if(!mutexInitialised){
        os_mutexAttr mutexAttr;
        os_result result;
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        /* Setup mutex: init the mutex with the specified attributes */
        result = os_mutexInit(&mutex, &mutexAttr);
        if(result == os_resultSuccess){
            mutexInitialised = 1;
        }
    }
    if(mutexInitialised){
        os_mutexLock(&mutex);
    }

    /*  variable arguments start after 'function' argument */
    va_start(args, function);
    infoType = va_arg(args, DLRL_Report_InfoType);


    if (!info_log)
    {
        DLRL_Report_openInfoLog();

        outer_home = os_getenv ("OSPL_OUTER_HOME");
        if (outer_home)
        {
            /*  read traceoptions from ini file */
            os_sprintf(buffer, "%s/testsuite/bin/ospl-dlrl-info.ini", outer_home);
            DLRL_Report_readOptions(buffer);
        }
    }

    if (info_log && DLRL_Report_checkFilter(infoType, fileName))
    {
        currentThread = os_threadIdSelf();
        if (!nr_of_indents ||
            (os_threadIdToInteger(indent[indentIndex].threadId) !=
             os_threadIdToInteger(currentThread)))
        {
            /*  were on the first thread or on another thread than the last one */
            /*  print thread switch marker */
            fprintf(info_log,   PA_ADDRFMT"********************************************"
                                "***********************************************\n", os_threadIdToInteger(currentThread));

            /*  find current thread or free spot in indent array */
            indentIndex = 0;
            ready = 0;
            while (!ready)
            {
                if (indentIndex == nr_of_indents)
                {
                    /*  were at a new free spot => initialize it */
                    indent[indentIndex].threadId = currentThread;
                    indent[indentIndex].indent = 0;
                    nr_of_indents++;
                    ready = 1;
                }
                else if (os_threadIdToInteger(indent[indentIndex].threadId) ==
                         os_threadIdToInteger(currentThread))
                {
                    /*  found the current thread in the history */
                    /*  start reusing the old indent */
                    ready = 1;
                }
                else if (indentIndex < MAX_INDENTS)
                {
                    indentIndex++;
                }
                else
                {
                    /*  indent admin full, reuse the last one */
                    ready = 1;
                }
            }
        }
        DLRL_Report_printLocationInfo(buffer, fileName, lineNo);
        p = buffer + LOCATION_SPACE;
        p += os_sprintf(p, ": ");

        /*  indent */
        for (i = 0 ; i < indent[indentIndex].indent ; i++)
        {
            p += os_sprintf(p, "  ");
        }
        if ((infoType == INF_ENTER) || (infoType == INF_OBJECT))
        {
            /*  entering a function, print -> + indented function name */
            p += os_sprintf(p, "-> %s", function);
            indent[indentIndex].indent++;
        }
        else if (infoType == INF_EXIT)
        {
            /*  exiting a function, print <- + indented function name */
            if (indent[indentIndex].indent) indent[indentIndex].indent--;
            p += os_sprintf(p-2, "<- %s", function);
        }
        else
        {
            /*  other info type, print the type to the buffer */
            p += os_sprintf(p, "%s ", DLRL_Report_infoType2String(infoType));
        }

        if ((infoType != INF_ENTER) && (infoType != INF_EXIT))
        {
            /*  in this case a variabale arg list must be specified of which the first argument is the format specifier */


            /*  first variable argument is format description string */
            fmt = va_arg(args, char *);
            /*  pass the format descriptor and the rest of the variable arguments */
            vsprintf(p, fmt, args);
        }
        fprintf(info_log, "%s\n", buffer);
        fflush(info_log);
    }
    va_end(args);
    if(mutexInitialised){
        os_mutexUnlock(&mutex);
    }
}

void
DLRL_Report_report(
    const char *fileName,
    int lineNo,
    const char *function,
    ...)
{
    os_time ostime;
    char extended_description[512];
    char procIdentity[64];
    char threadIdentity[64];
    char node[64];
    char date_time[64];
    char *fmt;
    FILE *log;
    va_list args;
    DLRL_Report_ReportType reportType;

    assert(fileName);
    assert(function);

    if (!report_log)
    {
        DLRL_Report_open_report_log();
    }
    ostime = os_timeGet();

    /*  variable arguments start after 'function' argument */
    va_start(args, function);

    reportType = va_arg(args, DLRL_Report_ReportType);
    assert(reportType < DLRL_Report_ReportType_elements);
    /*  first variable argument is format description string */
    fmt = va_arg(args, char *);
    /*  pass the format descriptor and the rest of the variable arguments */
    os_vsnprintf (extended_description, sizeof(extended_description)-1, fmt, args);
    extended_description [sizeof(extended_description)-1] = '\0';
    va_end(args);

    os_ctime_r (&ostime, date_time);
    date_time[strlen(date_time)-1] = '\0'; /* remove \n */
    os_gethostname (node, sizeof(node)-1);
    node [sizeof(node)-1] = '\0';
    os_threadFigureIdentity (threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity [sizeof (threadIdentity)-1] = '\0';
    os_procFigureIdentity (procIdentity, sizeof (procIdentity)-1);
    procIdentity [sizeof (procIdentity)-1] = '\0';

	if (report_log) {
	    log = report_log;
	} else {
	    log = stderr;
	}

    fprintf (
        log,
        "### Report Message ###\n"
        "Type        : %s\n"
        "File        : %s\n"
        "Line        : %d\n"
        "Function    : %s\n"
        "Description : %s\n"
        "Node        : %s\n"
        "Process     : %s\n"
        "Thread      : %s\n"
        "Timestamp   : %d.%9.9d (%s)\n",
        reportTypeText[reportType],
        fileName,
        lineNo,
        function,
        extended_description,
        node,
        procIdentity,
        threadIdentity,
        ostime.tv_sec,
        ostime.tv_nsec,
    	date_time);
    fflush (log);

    if (info_log)
    {
        fprintf (
            info_log,
            "### Report Message ###\n"
            "Type        : %s\n"
            "File        : %s\n"
            "Line        : %d\n"
            "Function    : %s\n"
            "Description : %s\n"
            "Node        : %s\n"
            "Process     : %s\n"
            "Thread      : %s\n"
            "Timestamp   : %d.%9.9d (%s)\n",
            reportTypeText[reportType],
            fileName,
            lineNo,
            function,
            extended_description,
            node,
            procIdentity,
            threadIdentity,
            ostime.tv_sec,
            ostime.tv_nsec,
            date_time);
        fflush (info_log);
    }
}
