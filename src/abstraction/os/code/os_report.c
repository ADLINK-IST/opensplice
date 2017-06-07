/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
#include "vortex_os.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef INTEGRITY
#include "os_log_cfg.h"
#endif

#define os_report_iserror(report) \
        (((report)->reportType >= OS_ERROR && (report)->code != 0))

#define OS_REPORTPLUGINS_MAX    (10)

#define OS_REPORT_TYPE_DEBUG     (1u)
#define OS_REPORT_TYPE_INFO      (1u<<OS_INFO)
#define OS_REPORT_TYPE_WARNING   (1u<<OS_WARNING)
#define OS_REPORT_TYPE_API_INFO  (1u<<OS_API_INFO)
#define OS_REPORT_TYPE_ERROR     (1u<<OS_ERROR)
#define OS_REPORT_TYPE_CRITICAL  (1u<<OS_CRITICAL)
#define OS_REPORT_TYPE_FATAL     (1u<<OS_FATAL)
#define OS_REPORT_TYPE_REPAIRED  (1u<<OS_REPAIRED)
#define OS_REPORT_TYPE_NONE      (1u<<OS_NONE)

#define OS_REPORT_TYPE_FLAG(x)   (1u<<(x))
#define OS_REPORT_ALWAYS(x)      ((x) & (OS_REPORT_TYPE_CRITICAL | OS_REPORT_TYPE_FATAL | OS_REPORT_TYPE_REPAIRED))
#define OS_REPORT_DEPRECATED(x)  ((x) & OS_REPORT_TYPE_API_INFO)
#define OS_REPORT_WARNING(x)     ((x) & OS_REPORT_TYPE_WARNING)
#define OS_REPORT_IS_ERROR(x)    ((x) & (OS_REPORT_TYPE_ERROR | OS_REPORT_TYPE_CRITICAL | OS_REPORT_TYPE_FATAL | OS_REPORT_TYPE_REPAIRED))

struct os_domainCallback_s {
    os_reportGetDomainCallback callback;
    void *argument;
};

typedef struct os_reportStack_s {
    os_int count;
    os_uint typeset;
    const os_char *file;
    os_int lineno;
    const os_char *signature;
    void *userInfo;
    os_iter reports;  /* os_reportEventV1 */
} *os_reportStack;

void os__report_append(os_reportStack _this, const os_reportEventV1 report);

static int os__report_fprintf(FILE *file, const char *format, ...);

void os__report_free(os_reportEventV1 report);

typedef struct os_reportPlugin_s {
    os_reportPlugin_initialize initialize_symbol;
    os_reportPlugin_report report_symbol;
    /** Pointer to the function for a typed event report method */
    os_reportPlugin_typedreport typedreport_symbol;
    os_reportPlugin_finalize finalize_symbol;
    os_reportPlugin_context plugin_context;
    struct os_reportPlugin_s* pstNext;
    struct os_reportPlugin_s* pstPrevious;
    os_int32 domainId;
} *os_reportPlugin_t;



static FILE* error_log = NULL;
static FILE* info_log = NULL;

static os_boolean doDefault = OS_TRUE;

static os_boolean reportedOnce = OS_FALSE;

static os_mutex reportMutex;
static os_mutex reportPluginMutex;

static os_boolean inited = OS_FALSE;

/**
 * Count any plugins registered with an XML string Report method
 * This is only used in an optimization. The count is protected by
 * reportPluginMutex, But can be used outside protection if linked
 * list is not accessed unprotected .
 * (worst case a report is build and not reported because plugin is just
 * unregistered)
 */
static os_uint32 xmlReportPluginsCount = 0;

static os_uint32 typedReportPluginsCount = 0;

static struct os_domainCallback_s domainCallbackInfo = {0};

/**
 * reportPluginAdmin contains pointer to first registered report plugin.
 * This is a linked list protected by reportPluginMutex.
 * If reportPluginAdmin is NULL, there are no registered plugins.
 * It is safe to check for NULL to see if we need to report data.
 * Using the linked list is only safe within locked reportPluginMutex.
 */
static os_reportPlugin_t reportPluginAdmin = NULL;

/**
 * Process global verbosity level for OS_REPORT output. os_reportType
 * values >= this value will be written.
 * This value defaults to OS_INFO, meaning that all types 'above' (i.e.
 * other than) OS_DEBUG will be written and OS_DEBUG will not be.
 */
os_reportType os_reportVerbosity = OS_INFO;

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
        "CRITICAL",
        "FATAL",
        "REPAIRED",
        "NONE"
};

enum os_report_logType {
    OS_REPORT_INFO,
    OS_REPORT_ERROR
};

static char * os_report_defaultInfoFileName = "ospl-info.log";
static char * os_report_defaultErrorFileName = "ospl-error.log";
static const os_char os_env_logdir[] = "OSPL_LOGPATH";
static const os_char os_env_infofile[] = "OSPL_INFOFILE";
static const os_char os_env_errorfile[] = "OSPL_ERRORFILE";
static const os_char os_env_verbosity[] = "OSPL_VERBOSITY";
static const os_char os_env_append[] = "OSPL_LOGAPPEND";
#ifdef VXWORKS_RTP
static const os_char os_env_procname[] = "SPLICE_PROCNAME";
#endif
#if defined VXWORKS_RTP || defined _WRS_KERNEL
static const os_char os_default_logdir[] = "/tgtsvr";
#else
static const os_char os_default_logdir[] = ".";
#endif


static const os_char
os__report_xml_head[] =
        "<%s>\n"
        "<DOMAINID>%d</DOMAINID>\n"
        "<DESCRIPTION>%.*s</DESCRIPTION>\n"
        "<CONTEXT>%s</CONTEXT>\n"
        "<FILE>%s</FILE>\n"
        "<LINE>%d</LINE>\n"
        "<CODE>%d</CODE>\n"
        "<PROCESS>\n"
        "<ID>%d</ID>\n"
        "<NAME>%s</NAME>\n"
        "</PROCESS>\n"
        "<THREAD>\n"
        "<ID>%lu</ID>\n"
        "<NAME>%s</NAME>\n"
        "</THREAD>\n"
        "<VERSION>%s</VERSION>\n"
        "<REVISION>%s %s</REVISION>\n";

static const os_char
os__report_xml_body[] =
        "<DETAIL>\n"
        "<REPORT>%s</REPORT>\n"
        "<INTERNALS>%s/%d/%d</INTERNALS>\n"
        "</DETAIL>\n";

static const os_char
os__report_xml_tail[] =
        "</%s>";

static const os_char
os__report_version[] = OSPL_VERSION_STR;

static const os_char
os__report_inner_revision[] = OSPL_INNER_REV_STR;

static const os_char
os__report_outer_revision[] = OSPL_OUTER_REV_STR;

static void
os_reportResetApiInfo (
        os_reportInfo *report);

static void
os_reportSetApiInfo (
        const os_char *context,
        const os_char *file,
        os_int32 line,
        os_int32 code,
        const os_char *message);

#if !defined(INTEGRITY)

static FILE * open_socket (char *host, unsigned short port)
{
    FILE * file = NULL;
    struct sockaddr_in sa;
    os_socket sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        os__report_fprintf(stderr, "socket: %s\n", os_strError(os_getErrno()));
        return NULL;
    }

    memset((char *)&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr (host);

    if (connect (sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        os__report_fprintf(stderr, "connect: %s\n", os_strError(os_getErrno()));
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
#ifdef VXWORKS_RTP
                    if (logfile != NULL) {
                        if (ret == 3) {
                            os__report_fprintf (logfile, "FILENAME:%s\n", serverfile);
                        }
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

#endif
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
    os_result osr;

    if (!doneOnce || forceReInit)
    {
        if (!doneOnce)
        {
            osr = os_mutexInit(&reportMutex, NULL);
            if(osr != os_resultSuccess)
            {
                OS_REPORT(OS_WARNING, "os_reportInit", 0,
                        "Unable to create report mutex");
            }
            osr = os_mutexInit(&reportPluginMutex, NULL);
            if(osr != os_resultSuccess)
            {
                OS_REPORT(OS_WARNING, "os_reportInit", 0,
                        "Unable to create report plugin mutex");
            }
        }

        doneOnce = OS_TRUE;
        envValue = os_getenv(os_env_verbosity);
        if (envValue != NULL)
        {
            if (os_reportSetVerbosity(envValue) == os_resultFail)
            {
                OS_REPORT(OS_WARNING, "os_reportInit", 0,
                        "Cannot parse report verbosity %s value \"%s\","
                        " reporting verbosity remains %s", os_env_verbosity, envValue, os_reportTypeText[os_reportVerbosity]);
            }
        }

        envValue = os_getenv(os_env_append);
        if (envValue != NULL)
        {
            os_boolean shouldAppend;
            if (os_configIsTrue(envValue, &shouldAppend) == os_resultFail)
            {
                OS_REPORT(OS_WARNING, "os_reportInit", 0,
                        "Cannot parse report %s value \"%s\","
                        " reporting append mode unchanged", os_env_append, envValue);
            }
            else
            {
                /* Remove log files when not appending. */
                if (!shouldAppend) {
                    os_reportRemoveStaleLogs();
                }
            }
        }
    }
    inited = OS_TRUE;
}

void os_reportExit()
{
    char *name;
    os_reportStack reports;

    reports = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (reports) {
        os_report_dumpStack(OS_FUNCTION, __FILE__, __LINE__);
        /* registered destructor function will clear data in thread specific memory */
        os_threadMemFree(OS_THREAD_REPORT_STACK);
    }
    inited = OS_FALSE;
    os_mutexDestroy(&reportMutex);

    os_mutexDestroy(&reportPluginMutex);

    if (error_log)
    {
        name = os_reportGetErrorFileName();
        os_close_file(name, error_log);
        os_free (name);
        error_log = NULL;
    }

    if (info_log)
    {
        name = os_reportGetInfoFileName();
        os_close_file(name, info_log);
        os_free (name);
        info_log = NULL;
    }
}

/*
 * Makes the directory structure passed from top most to bottom
 */
static os_boolean os_mkdirs (const char *path)
{
    os_boolean ret = OS_FALSE;
    char *pathcopy, *dpath;
    os_result status;
    struct os_stat statBuf;

    if (path)
    {
        /* Directory already exists and is writable, good */
        status = os_stat(path, &statBuf);
        if (status == os_resultSuccess && OS_ISDIR (statBuf.stat_mode))
        {
            /* Check whether dir is writable */
            status = os_access(path, OS_WOK);
            if (status == os_resultSuccess)
            {
                ret = OS_TRUE;
            }
        }
        else
        {
            /* Try to make the full path, if not then recurse down the path
             * until you eventually get a dir you can create then come back up
             */
            if (os_mkdir (path, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
            {
                pathcopy = os_strdup(path);
                if (pathcopy)
                {
                    dpath = os_dirname_r(pathcopy);
                    if (dpath && strcmp(path, dpath) != 0)
                    {
                        /* Create the directory below this one */
                        if (os_mkdirs (dpath))
                        {
                            /* Looks like we created the one below so create this one
                             * should work now, if not there is an unkown error
                             */
                            if (os_mkdir (path, S_IRWXU | S_IRWXG | S_IRWXO) == 0)
                            {
                                ret = OS_TRUE;
                            }
                        }
                    }
                    os_free (dpath);
                }
                os_free (pathcopy);
            }
            else
            {
                ret = OS_TRUE;
            }
        }
    }
    return ret;
}

static os_boolean os_report_is_local_file(char * file_name, char **new_name)
{
    char host[256];
    char server_file[256];
    unsigned short port;

#if defined VXWORKS_RTP
    char *ptok;
    char *splice_procname;
    if ( file_name != NULL
            && ((ptok = os_index( file_name, '%' ) ) != NULL)
            && ptok[1] == 's'
                    && ( splice_procname = os_getenv( os_env_procname ) ) != NULL )
    {
        *new_name = (char *)os_malloc( strlen( file_name )
                + strlen( splice_procname )
                -1 );
        os_sprintf( *new_name, file_name, splice_procname );
    }
    else
    {
        *new_name = os_strdup(file_name);
    }
#else
    *new_name = file_name;
#endif
    return ((sscanf (*new_name, "%255[^:]:%hu", host, &port) != 2
            && sscanf (*new_name, "%255[^:]:%hu:%255[^:]", host, &port, server_file) != 3) ? OS_TRUE : OS_FALSE);
}

#define MAX_FILE_PATH 2048
static char *os_report_createFileNormalize(char *file_path, char *file_dir, char *file_name)
{
    int len;

    len = snprintf(file_path, MAX_FILE_PATH, "%s/%s", file_dir, file_name);
    /* Note bug in glibc < 2.0.6 returns -1 for output truncated */
    if ( len < MAX_FILE_PATH && len > -1 )
    {
#if defined VXWORKS_RTP
        os_free (file_name);
#endif
        return (os_fileNormalize(file_path));
    }
    else
    {
        return (file_name);
    }
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
os_report_file_path(char * default_file, char * override_variable, enum os_report_logType type)
{
    char *file_dir;
    char file_path[MAX_FILE_PATH];
    char *file_name = NULL;
    char *full_file_path = NULL;
    FILE *logfile = NULL;
    char *override = NULL;

    if (override_variable != NULL)
    {
        override = os_getenv(override_variable);
        file_name = override;
    }
    if (!file_name)
    {
        file_name = default_file;
    }

    file_dir = os_getenv(os_env_logdir);
    if (!file_dir)
    {
        file_dir = (os_char *)os_default_logdir;
    }
    else
    {
        /* We just need to check if a file can be written to the directory, we just use the
         * default info file as there will always be one created, if we used the variables
         * passed in we would create an empty error log (which is bad for testing) and we
         * cannot delete it as we open the file with append
         */
        if (type == OS_REPORT_INFO)
        {
            full_file_path = (char*) os_malloc(strlen(file_dir) + 1 + strlen(file_name) + 1 );
            os_strcpy(full_file_path, file_dir);
            os_strcat(full_file_path, "/");
            os_strcat(full_file_path, file_name);
            if (os_report_is_local_file(full_file_path,&full_file_path))
            {
                logfile = fopen (full_file_path, "a");
                if (!logfile && !os_mkdirs (file_dir))
                {
                    if (!reportedOnce && file_dir != NULL)
                    {
                        printf ("INFO: Could not use %s, logs can be found in %s\n", file_dir, os_default_logdir);
                        reportedOnce = OS_TRUE;
                    }
                    file_dir = (os_char *)os_default_logdir;
                }
                if (logfile)
                {
                    fclose(logfile);
                }
            }
        }
        os_free (full_file_path);
    }
#if defined VXWORKS_RTP
    if (override != NULL)
    {
        {
            char * new_name = NULL;
            if (os_report_is_local_file(file_name, &new_name))
            {
                return (os_report_createFileNormalize(file_path, file_dir, new_name));
            }
            else
            {
                return (new_name);
            }
        }
    }
#endif
    if (strcmp(file_name, "<stderr>") != 0 && strcmp(file_name, "<stdout>") != 0)
    {
        if (os_report_is_local_file(file_name, &file_name))
        {
            return (os_report_createFileNormalize(file_path, file_dir, file_name));
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
    os_reportInit(OS_FALSE);
    file_name = os_report_file_path (os_report_defaultInfoFileName, (os_char *)os_env_infofile, OS_REPORT_INFO);
    /* @todo dds2881 - Uncomment below & remove above to enable application default error logging to stderr */
    /* file_name = os_report_file_path (os_procIsOpenSpliceService() ? "ospl-info.log" : "<stdout>", os_env_infofile);*/
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
    os_reportInit(OS_FALSE);
    file_name = os_report_file_path (os_report_defaultErrorFileName, (os_char *)os_env_errorfile, OS_REPORT_ERROR);
    /* @todo dds2881 - Uncomment below & remove above to enable application default error logging to stderr */
    /* file_name = os_report_file_path (os_procIsOpenSpliceService() ? "ospl-error.log" : "<stderr>", os_env_errorfile); */
    return file_name;
}
#if !defined(INTEGRITY)

static FILE *
os_open_info_file (void)
{
    char * name;
    FILE * file;

    name = os_reportGetInfoFileName();
    file = os_open_file(name);
    if (!file)
    {
        file = os_open_file("<stdout>");
    }
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
    if (!file)
    {
        file = os_open_file("<stderr>");
    }
    os_free (name);
    return file;
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
os_sectionReport(
        os_reportEventV1 event,
        os_boolean useErrorLog)
{
    os_timeW ostime;
    FILE *log;

    if (useErrorLog) {
        if ( error_log == NULL )
        {
            error_log = os_open_error_file();
        }
        log = error_log;
    } else {
        if ( info_log == NULL )
        {
            info_log = os_open_info_file();
        }
        log = info_log;
    }

    ostime = os_timeWGet();
    os_mutexLock(&reportMutex);
#ifdef INTEGRITY
    os_logprintf( log,
#else
            os__report_fprintf(log,
#endif
                    "----------------------------------------------------------------------------------------\n"
                    "Report      : %s\n"
                    "Internals   : %s/%s/%d/%d/%" PA_PRItime "\n",
                    event->description,
                    event->reportContext,
                    event->fileName,
                    event->lineNo,
                    event->code,
                    OS_TIMEW_PRINT(ostime));
#ifndef INTEGRITY
    fflush (log);
#endif
    os_mutexUnlock(&reportMutex);
}

static void
os_headerReport(
        os_reportEventV1 event,
        os_boolean useErrorLog,
        os_int32 domainId)
{
    os_timeW ostime;
    char node[64];
    char date_time[128];
    FILE *log;

    /* Check error_file is NULL here to keep user loggging */
    /* plugin simple in integrity */
    if (useErrorLog) {
        if ( error_log == NULL )
        {
            error_log = os_open_error_file();
        }
        log = error_log;
    } else {
        if ( info_log == NULL )
        {
            info_log = os_open_info_file();
        }
        log = info_log;
    }

    ostime = os_timeWGet();
    os_ctimeW_r(&ostime, date_time, sizeof(date_time));

    if (os_gethostname(node, sizeof(node)-1) == os_resultSuccess)
    {
        node[sizeof(node)-1] = '\0';
    }
    else
    {
        os_strcpy(node, "UnkownNode");
    }

    os_mutexLock(&reportMutex);
    if (useErrorLog) {
#ifdef INTEGRITY
        os_logprintf( log,
#else
                os__report_fprintf(log,
#endif
                        "========================================================================================\n"
                        "Context     : %s\n"
                        "Date        : %s\n"
                        "Node        : %s\n"
                        "Process     : %s\n"
                        "Thread      : %s\n"
                        "Internals   : %s/%d/%s/%s/%s/%d\n",
                        event->description,
                        date_time,
                        node,
                        event->processDesc,
                        event->threadDesc,
                        event->fileName,
                        event->lineNo,
                        OSPL_VERSION_STR,
                        OSPL_INNER_REV_STR,
                        OSPL_OUTER_REV_STR,
                        domainId);
    } else {
#ifdef INTEGRITY
        os_logprintf( log,
#else
                os__report_fprintf(log,
#endif
                        "========================================================================================\n"
                        "Report      : %s\n"
                        "Context     : %s\n"
                        "Date        : %s\n"
                        "Node        : %s\n"
                        "Process     : %s\n"
                        "Thread      : %s\n"
                        "Internals   : %s/%d/%s/%s/%s/%d\n",
                        os_reportTypeText[event->reportType],
                        event->description,
                        date_time,
                        node,
                        event->processDesc,
                        event->threadDesc,
                        event->fileName,
                        event->lineNo,
                        OSPL_VERSION_STR,
                        OSPL_INNER_REV_STR,
                        OSPL_OUTER_REV_STR,
                        domainId);
    }
#ifndef INTEGRITY
    fflush (log);
#endif
    os_mutexUnlock(&reportMutex);
}

static void
os_defaultReport(
        os_reportEventV1 event,
        os_int32 domainId)
{
    os_timeW ostime;
    char node[64];
    char date_time[128];
    FILE *log;

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

    ostime = os_timeWGet();
    os_ctimeW_r(&ostime, date_time, sizeof(date_time));
    os_gethostname(node, sizeof(node)-1);
    node[sizeof(node)-1] = '\0';

    if ((domainId == -1) && domainCallbackInfo.callback) {
        domainId = domainCallbackInfo.callback(domainCallbackInfo.argument);
    }

    if (os_gethostname(node, sizeof(node)-1) == os_resultSuccess)
    {
        node[sizeof(node)-1] = '\0';
    }
    else
    {
        os_strcpy(node, "UnkownNode");
    }

    os_mutexLock(&reportMutex);
#ifdef INTEGRITY
    os_logprintf( log,
#else
            os__report_fprintf(log,
#endif
                    "========================================================================================\n"
                    "Report      : %s\n"
                    "Date        : %s\n"
                    "Description : %s\n"
                    "Node        : %s\n"
                    "Process     : %s\n"
                    "Thread      : %s\n"
                    "Internals   : %s/%s/%s/%s/%s/%d/%d/%" PA_PRItime "/%d\n",
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
                    OS_TIMEW_PRINT(ostime),
                    domainId);
#ifndef INTEGRITY
    fflush (log);
#endif
    os_mutexUnlock(&reportMutex);
}

static void
os_report_internal(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        os_int32 domainId,
        const os_char *message)
{
    os_char *file;
    os_char procid[256], thrid[64], tmp[2];
    os_reportPlugin_t plugin;
    os_reportStack stack;

    struct os_reportEventV1_s report = { OS_REPORT_EVENT_V1, /* version */
            OS_NONE, /* reportType */
            NULL, /* reportContext */
            NULL, /* fileName */
            0, /* lineNo */
            0, /* code */
            NULL, /* description */
            NULL, /* threadDesc */
            NULL /* processDesc */
    };
#if 0
    /* OS_API_INFO label is kept for backwards compatibility. */
    if (type == OS_API_INFO) {
        type = OS_ERROR;
    }
#endif

    if (inited == OS_FALSE) {
        return;
    }

    if (type < os_reportVerbosity) {
        /* This level / type of report is below the process output suppression threshold. */
        return;
    }

    if ((file = os_strrchrs (path, os_fileSep(), OS_TRUE)) == NULL) {
        file = (os_char *)path;
    } else {
        file++;
    }

    /* Only figure out process and thread identities if the user requested an
       entry in the default log file or registered a typed report plugin. */
    if ((doDefault) || (xmlReportPluginsCount > 0) || (typedReportPluginsCount > 0))
    {
        os_procFigureIdentity (procid, sizeof (procid));
        os_threadFigureIdentity (thrid, sizeof (thrid));

        report.reportType = type;
        report.reportContext = (os_char *)context;
        report.fileName = (os_char *)file;
        report.lineNo = line;
        report.code = code;
        report.description = (os_char *)message;
        report.threadDesc = thrid;
        report.processDesc = procid;
    }

    stack = (os_reportStack)os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (stack && stack->count) {
        if (report.reportType != OS_NONE) {
            os__report_append (stack, &report);
        }

    } else {
        if (doDefault) {
            os_defaultReport (&report, domainId);
        }

        if (reportPluginAdmin != NULL) {
            os_char buf[OS_REPORT_BUFLEN], *ptr = NULL;
            os_size_t len = 0;
            os_ssize_t off, cnt = 0;

            if (xmlReportPluginsCount > 0) {
                os_boolean again;
                os_char proc[256], thr[64];
                os_procId pid;
                os_ulong_int tid;

                pid = os_procIdSelf ();
                tid = os_threadIdToInteger (os_threadIdSelf ());

                os_procGetProcessName (proc, sizeof (proc));
                os_threadGetThreadName (thr, sizeof (thr));

                if ((domainId == -1) && domainCallbackInfo.callback) {
                    domainId = domainCallbackInfo.callback(domainCallbackInfo.argument);
                }

                ptr = buf;
                len = sizeof(buf);

                /* Try using the fixed length buffer first. If it's not
                   sufficiently large enough, dynamically allocate a block of
                   memory to hold the XML document. */
                do {
                    cnt = (os_ssize_t)snprintf(
                            ptr,
                            len,
                            os__report_xml_head,
                            os_reportTypeText[type],
                            domainId,
                            OS_REPORT_BUFLEN,
                            message,
                            context,
                            file,
                            line,
                            code,
                            pid, proc,
                            tid, thr,
                            os__report_version,
                            os__report_inner_revision, os__report_outer_revision);
                    again = OS_FALSE;

                    if (cnt < 0) {
                        if (ptr != buf) {
                            os_free (ptr);
                        }
                        ptr = NULL;
                    } else {
                        off = cnt;
                        cnt += 1;
                        cnt += (os_ssize_t)snprintf(
                                tmp,
                                sizeof(tmp),
                                os__report_xml_tail,
                                os_reportTypeText[type]);
                        if ((os_size_t)cnt > len) {
                            assert (ptr == buf);
                            ptr = os_malloc ((os_size_t)cnt);
                            if (ptr != NULL) {
                                len = (os_size_t)cnt;
                                again = OS_TRUE;
                            }
                        } else {
                            (void)snprintf(
                                    ptr + off,
                                    len - (os_size_t) off,
                                    os__report_xml_tail,
                                    os_reportTypeText[type]);
                        }
                    }
                } while (again == OS_TRUE);
            }
            os_mutexLock(&reportPluginMutex);
            plugin = reportPluginAdmin;
            while (plugin != NULL) {
                if ((domainId == -1) || (domainId == plugin->domainId)) {
                    if (plugin->report_symbol != NULL && ptr != NULL) {
                        plugin->report_symbol (
                                plugin->plugin_context, ptr);
                    }

                    if (plugin->typedreport_symbol != NULL) {
                        plugin->typedreport_symbol (
                                plugin->plugin_context, (os_reportEvent) &report);
                    }
                }
                plugin = plugin->pstNext;
            }
            os_mutexUnlock(&reportPluginMutex);
            /* Cleanup if memory was allocated */
            if (ptr != buf) {
                os_free (ptr);
            }
        }

        if (os_report_iserror (&report)) {
            os_reportSetApiInfo (context, file, line, code, message);
        }
    }
}

void
os_report_noargs(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        const os_char *message)
{
    os_report_internal(type, context, path, line, code, -1, message);
}


void
os_report(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        const os_char *format,
        ...)
{
    os_char buf[OS_REPORT_BUFLEN];
    va_list args;

    if (inited == OS_FALSE) {
        return;
    }

    if (type < os_reportVerbosity) {
        return;
    }

    va_start (args, format);
    (void)os_vsnprintf (buf, sizeof(buf), format, args);
    va_end (args);

    os_report_noargs (type, context, path, line, code, buf);
}

void
os_report_w_domain(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        os_int32 domainId,
        const os_char *format,
        ...)
{
    os_char buf[OS_REPORT_BUFLEN];
    va_list args;

    if (inited == OS_FALSE) {
        return;
    }

    if (type < os_reportVerbosity) {
        return;
    }

    va_start (args, format);
    (void)os_vsnprintf (buf, sizeof(buf), format, args);
    va_end (args);

    os_report_internal (type, context, path, line, code, domainId, buf);
}


static void
os_reportResetApiInfo (
        os_reportInfo *report)
{
    assert (report != NULL);

    os_free (report->reportContext);
    os_free (report->sourceLine);
    os_free (report->description);
    (void)memset (report, 0, sizeof (os_reportInfo));
}

int
os_reportDestructorApiInfo(void* pvMem, void* userArg)
{
    os_reportInfo *report = (os_reportInfo *)pvMem;
    OS_UNUSED_ARG(userArg);

    if (report != NULL) {
        os_reportResetApiInfo (report);
    }
    return os_resultSuccess;
}

static void
os_reportSetApiInfo (
        const os_char *context,
        const os_char *file,
        os_int32 line,
        os_int32 code,
        const os_char *message)
{
    const os_char *format = NULL;
    os_char point[512];
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report == NULL) {
        report = (os_reportInfo *)os_threadMemMalloc(OS_THREAD_API_INFO, sizeof(os_reportInfo), os_reportDestructorApiInfo, NULL);
        if (report) {
            memset(report, 0, sizeof(os_reportInfo));
        }
    }
    if (report != NULL) {
        os_reportResetApiInfo (report);

        if (context != NULL) {
            report->reportContext = os_strdup (context);
        }

        if (file != NULL && line > 0) {
            format = "%s:%d";
        } else if (file != NULL) {
            format = "%s";
        } else if (line > 0) {
            file = "";
            format = "%d";
        }

        if (format != NULL) {
            (void)snprintf (point, sizeof (point), format, file, line);
            report->sourceLine = os_strdup (point);
        }

        report->reportCode = code;

        if (message != NULL) {
            report->description = os_strndup (message, OS_REPORT_BUFLEN + 1);
        }
    }
}

os_reportInfo *
os_reportGetApiInfo(void)
{
    return (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
}


void
os_reportClearApiInfo(void)
{
    os_reportInfo *report;

    report = (os_reportInfo *)os_threadMemGet(OS_THREAD_API_INFO);
    if (report != NULL) {
        /* registered destructor function will clear data in thread specific memory */
        os_threadMemFree(OS_THREAD_API_INFO);
    }
}

static os_int32
os_reportPluginAddToAdministration(
        os_reportPlugin_t plugin)
{
#ifdef INCLUDE_PLUGGABLE_REPORTING
    /* protect administration for changes during reports */
    os_mutexLock(&reportPluginMutex);
    if (reportPluginAdmin == NULL) {
        plugin->pstNext = NULL;
        plugin->pstPrevious = NULL;
        reportPluginAdmin = plugin;
    }
    else {
        plugin->pstNext = reportPluginAdmin;
        plugin->pstPrevious = NULL;
        reportPluginAdmin->pstPrevious = plugin;
        reportPluginAdmin = plugin;
    }
    if (plugin->report_symbol != NULL) {
        xmlReportPluginsCount++;
    }
    if (plugin->typedreport_symbol != NULL) {
        typedReportPluginsCount++;
    }

    os_mutexUnlock(&reportPluginMutex);
    return 0;
#else
    (void)plugin;
    return -1;
#endif
}

static os_int32
os_reportPluginRemoveFromAdministration(
        os_reportPlugin_t plugin)
{
#ifdef INCLUDE_PLUGGABLE_REPORTING
    /* protect administration for changes during reports */
    os_mutexLock(&reportPluginMutex);
    assert(reportPluginAdmin != NULL);
    if (plugin->report_symbol != NULL) {
        xmlReportPluginsCount--;
    }
    if (plugin->typedreport_symbol != NULL) {
        typedReportPluginsCount--;
    }
    if (reportPluginAdmin == plugin) {
        reportPluginAdmin = plugin->pstNext;
        if (reportPluginAdmin != NULL) {
            reportPluginAdmin->pstPrevious = NULL;
        }
    } else {
        plugin->pstPrevious->pstNext = plugin->pstNext;
        if (plugin->pstNext != NULL) {
            plugin->pstNext->pstPrevious = plugin->pstPrevious;
        }
    }
    plugin->pstNext = NULL;
    plugin->pstPrevious = NULL;
    os_mutexUnlock(&reportPluginMutex);
    return 0;
#else
    (void)plugin;
    return -1;
#endif
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
        os_int32 domainId,
        os_reportPlugin *plugin)
{
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_library libraryHandle;
    os_libraryAttr attr;
    os_boolean error = OS_FALSE;
    os_int32 initResult;

    os_reportPlugin_initialize initFunction;
    os_reportPlugin_finalize finalizeFunction;
    os_reportPlugin_report reportFunction = NULL;
    os_reportPlugin_typedreport typedReportFunction = NULL;

    os_libraryAttrInit(&attr);
    libraryHandle = NULL;
    if (library_file_name != NULL )
    {
        libraryHandle = os_libraryOpen (library_file_name, &attr);
    }
    if (libraryHandle == NULL)
    {
        OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
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
            OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Unable to resolve report intialize function: %s", initialize_method_name);

            error = OS_TRUE;
        }
    }

    if (!error)
    {
        finalizeFunction = (os_reportPlugin_finalize)os_fptr(os_libraryGetSymbol (libraryHandle, finalize_method_name));

        if (finalizeFunction == NULL)
        {
            OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
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
                OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
                        "Unable to resolve report Report function: %s", report_method_name);

                error = OS_TRUE;
            }
        }

        if (typedreport_method_name != NULL)
        {
            typedReportFunction = (os_reportPlugin_typedreport)os_fptr(os_libraryGetSymbol (libraryHandle, typedreport_method_name));

            if (typedReportFunction == NULL)
            {
                OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
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
                domainId,
                plugin);
        if (initResult)
        {
            OS_REPORT (OS_ERROR, "os_reportRegisterPlugin", 0,
                    "Plug-in initialization method failed : %s", initialize_method_name);
        }
        else
        {
            return 0;
        }
    }

    OS_REPORT (OS_WARNING, "os_reportRegisterPlugin", 0,
            "Failed to register report plugin : %s", library_file_name);

    return -1;
#else
    if (library_file_name != NULL ) {
        OS_REPORT(OS_ERROR, "os_reportRegisterPlugin", 0, "Unable to register report plugin: %s because \
                   product was not built with INCLUDE_PLUGGABLE_REPORTING enabled", library_file_name);
    } else {
        OS_REPORT(OS_ERROR, "os_reportRegisterPlugin", 0, "Unable to register report plugin because \
                 product was not built with INCLUDE_PLUGGABLE_REPORTING enabled");
    }
    (void)library_file_name;
    (void)initialize_method_name;
    (void)argument;
    (void)report_method_name;
    (void)typedreport_method_name;
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
        os_int32 domainId,
        os_reportPlugin *plugin)
{
#ifdef INCLUDE_PLUGGABLE_REPORTING
    os_reportPlugin_context context;
    os_reportPlugin_t rplugin;
    int osr;

    osr = initFunction (argument, &context);

    if (osr != 0)
    {
        OS_REPORT (OS_ERROR, "os_reportInitPlugin", 0,
                "Initialize report plugin failed : Return code %d\n", osr);
        return -1;
    }

    rplugin = os_malloc(sizeof *rplugin);
    rplugin->initialize_symbol = initFunction;
    rplugin->report_symbol = reportFunction;
    rplugin->typedreport_symbol = typedReportFunction;
    rplugin->finalize_symbol = finalizeFunction;
    rplugin->plugin_context = context;
    rplugin->domainId = domainId;

    *plugin = rplugin;

    if (suppressDefaultLogs) {
        doDefault = OS_FALSE;
    }

    os_reportPluginAddToAdministration(rplugin);
    return 0;
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
    os_reportPlugin_finalize finalize_symbol = NULL;
    os_reportPlugin_context plugin_context = NULL;
    int osr;

    rplugin = (os_reportPlugin_t)plugin;
    finalize_symbol = rplugin->finalize_symbol;
    plugin_context = rplugin->plugin_context;

    os_reportPluginRemoveFromAdministration(rplugin);
    os_free(rplugin);
    rplugin = NULL;
    if (finalize_symbol) {
        osr = finalize_symbol(plugin_context);
        if (osr != 0){
            OS_REPORT (OS_ERROR,
                    "os_reportUnregisterPlugin", 0,
                    "Finalize report plugin failed : Return code %d\n",
                    osr);
            return -1;
        }
    }
    return 0;
#else
    (void)plugin;
    return -1;
#endif
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
        /* OS_API_INFO label is kept for backwards compatibility. */
        if (OS_API_INFO == (os_reportType)verbosityInt) {
            os_reportVerbosity = OS_ERROR;
        } else {
            os_reportVerbosity = (os_reportType)verbosityInt;
        }
        result = os_resultSuccess;
    }

    return result;
}

/**
 * Remove possible existing log files to start cleanly.
 */
void
os_reportRemoveStaleLogs()
{
    static os_boolean alreadyDeleted = OS_FALSE;
    char * name;

    os_reportInit(OS_FALSE);

    if (!alreadyDeleted) {
        /* Only a single process or spliced (as 1st process) is allowed to
         * delete the log files. */
        if (os_procIsOpenSpliceDomainDaemon() || os_serviceGetSingleProcess()) {
            /* Remove ospl-info.log and ospl-error.log.
             * Ignore the result because it is possible that they don't exist yet. */
            name = os_reportGetInfoFileName();
            (void)os_remove(name);
            os_free(name);
            name = os_reportGetErrorFileName();
            (void)os_remove(name);
            os_free(name);
            alreadyDeleted = OS_TRUE;
        }
    }
}

/*****************************************
 * Report-stack related functions
 *****************************************/

int
os_report_private_thread_mem_destructor(
        void* pvMem,
        void* userArgs)
{
    os_reportStack _this = (os_reportStack)pvMem;
    os_reportEventV1 report;
    OS_UNUSED_ARG(userArgs);

    if (_this) {
        while((report = os_iterTakeLast(_this->reports))) {
            os__report_free(report);
        }
        os_iterFree(_this->reports);
        _this->reports = NULL;
    }
    return os_resultSuccess;
}

void
os_report_stack()
{
    os_reportStack _this;

    if (inited == OS_FALSE) {
        return;
    }
    _this = (os_reportStack)os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (!_this) {
        /* Report stack does not exist yet, so create it */
        _this = os_threadMemMalloc(OS_THREAD_REPORT_STACK, sizeof(struct os_reportStack_s), os_report_private_thread_mem_destructor, NULL);
        if (_this) {
            _this->count = 1;
            _this->typeset = 0;
            _this->file = NULL;
            _this->lineno = 0;
            _this->signature = NULL;
            _this->reports = os_iterNew(NULL);
        } else {
            OS_REPORT(OS_ERROR, "os_report_stack", 0,
                    "Failed to initialize report stack (could not allocate thread-specific memory)");
        }
    } else {
        /* Use previously created report stack */
        if (_this->count == 0) {
            _this->file = NULL;
            _this->lineno = 0;
            _this->signature = NULL;
        }
        _this->count++;

    }
}

void
os_report_stack_open(
        const os_char *file,
        os_int lineno,
        const os_char *signature,
        void *userInfo)
{
    os_reportStack _this;

    _this = (os_reportStack)os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (!_this) {
        /* Report stack does not exist yet, so create it */
        _this = os_threadMemMalloc(OS_THREAD_REPORT_STACK, sizeof(struct os_reportStack_s), os_report_private_thread_mem_destructor, NULL);
        if (_this) {
            _this->count = 1;
            _this->typeset = 0;
            _this->file = file;
            _this->lineno = lineno;
            _this->signature = signature;
            _this->userInfo = userInfo;
            _this->reports = os_iterNew(NULL);
        } else {
            OS_REPORT(OS_ERROR, "os_report_stack", 0,
                    "Failed to initialize report stack (could not allocate thread-specific memory)");
        }
    } else {
        /* Use previously created report stack */
        if (_this->count == 0) {
            _this->file = file;
            _this->lineno = lineno;
            _this->signature = signature;
            _this->userInfo = userInfo;
        }
        _this->count++;
    }
}

os_boolean
os_report_get_context(
        const os_char **file,
        os_int *lineno,
        const os_char **signature,
        void **userInfo)
{
    os_boolean result = OS_FALSE;
    os_reportStack _this;

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count) && (_this->file)) {
        *file = _this->file;
        *lineno = _this->lineno;
        *signature = _this->signature;
        if (userInfo) {
            *userInfo = _this->userInfo;
        }
        result = OS_TRUE;
    }
    return result;
}


void
os_report_stack_free(
        void)
{
    os_reportStack _this;

    _this = (os_reportStack)os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (_this) {
        /* registered destructor will free pending reports */
        os_threadMemFree(OS_THREAD_REPORT_STACK);
    }
}

static void
os__report_stack_event_length (
        void *object,
        void *argument)
{
    os_char tmp[2];
    os_reportEventV1 event;

    assert (object != NULL);
    assert (argument != NULL);

    event = (os_reportEventV1)object;
    *((os_size_t *)argument) +=
            sizeof (os__report_xml_body) +
            strlen (event->description) +
            strlen (event->fileName) +
            (os_size_t)snprintf (tmp, sizeof(tmp), "%d", event->lineNo) +
            (os_size_t)snprintf (tmp, sizeof(tmp), "%d", event->code);
}

struct os__reportBuffer {
    os_char *str;
    os_size_t len;
    os_size_t off;
};

static void
os__report_stack_event_print (
        void *object,
        void *argument)
{
    os_reportEventV1 event;
    struct os__reportBuffer *buf;
    os_size_t len;

    assert (object != NULL);
    assert (argument != NULL);

    event = (os_reportEventV1)object;
    buf = (struct os__reportBuffer *)argument;

    len = (os_size_t)snprintf (
            buf->str + buf->off,
            buf->len - buf->off,
            os__report_xml_body,
            event->description,
            event->fileName,
            event->lineNo,
            event->code);

    assert (len <= (buf->len - buf->off));

    buf->off += len;
}

#if 0
static os_reportType
os__report_stack_report_type(
        os_reportStack _this)
{
    os_reportType reportType;

    if (_this->typeset & OS_REPORT_TYPE_REPAIRED) {
        reportType = OS_REPAIRED;
    } else if (_this->typeset & OS_REPORT_TYPE_FATAL) {
        reportType = OS_FATAL;
    } else if (_this->typeset & OS_REPORT_TYPE_CRITICAL) {
        reportType = OS_CRITICAL;
    } else if (_this->typeset & OS_REPORT_TYPE_ERROR) {
        reportType = OS_ERROR;
    } else if (_this->typeset & OS_REPORT_TYPE_API_INFO) {
        reportType = OS_API_INFO;
    } else if (_this->typeset & OS_REPORT_TYPE_WARNING) {
        reportType = OS_WARNING;
    } else if (_this->typeset & OS_REPORT_TYPE_INFO) {
        reportType = OS_INFO;
    }

    return reportType;
}
#endif

static void
os__report_stack_unwind(
        os_reportStack _this,
        os_boolean valid,
        const os_char *context,
        const os_char *path,
        os_int line,
        os_int32 domainId)
{
    static const os_char description[] = "Operation failed.";
    struct os_reportEventV1_s header;
    os_reportEventV1 report;
    struct os__reportBuffer buf = { NULL, 0, 0 };
    os_iter tempList;
    os_char *file;
    os_char tmp[2];
    os_int32 code = 0;
    os_reportPlugin_t plugin;
    os_boolean update = OS_TRUE;
    os_reportType filter = OS_NONE;
    os_boolean useErrorLog;
    os_reportType reportType = OS_ERROR;

    if (!valid) {
        if (OS_REPORT_ALWAYS(_this->typeset)) {
            valid = OS_TRUE;
        } else if (OS_REPORT_DEPRECATED(_this->typeset)) {
            filter = OS_API_INFO;
            reportType = OS_API_INFO;
        } else {
            filter = OS_NONE;
        }

        if (filter != OS_NONE) {
            tempList = os_iterNew(NULL);
            if (tempList) {
                while ((report = os_iterTakeLast(_this->reports))) {
                    if (report->reportType == filter) {
                        (void)os_iterAppend(tempList, report);
                    } else {
                        os__report_free(report);
                    }
                }
                while ((report = os_iterTakeLast(tempList))) {
                    os_iterAppend(_this->reports, report);
                }
                os_free(tempList);
            }
            valid = OS_TRUE;
        }
    }

    useErrorLog = OS_REPORT_IS_ERROR(_this->typeset);

    _this->typeset = 0;

    if (valid) {
        os_char proc[256], procid[256];
        os_char thr[64], thrid[64];
        os_procId pid;
        os_ulong_int tid;

        assert (context != NULL);
        assert (path != NULL);

        if ((file = os_strrchrs (path, os_fileSep(), OS_TRUE)) == NULL) {
            file = (os_char *)path;
        } else {
            file++;
        }

        pid = os_procIdSelf ();
        tid = os_threadIdToInteger (os_threadIdSelf ());

        os_procFigureIdentity (procid, sizeof (procid));
        os_procGetProcessName (proc, sizeof (proc));
        os_threadFigureIdentity (thrid, sizeof (thrid));
        os_threadGetThreadName (thr, sizeof (thr));

        if (reportPluginAdmin != NULL && xmlReportPluginsCount > 0) {
            buf.len = sizeof (os__report_xml_head) +
                    sizeof (os__report_xml_tail) +
                    sizeof (description) +
                    (os_size_t)(strlen (os_reportTypeText[reportType]) * 2) +
                    (os_size_t)snprintf (tmp, sizeof(tmp), "%d", 255) +
                    strlen (context) +
                    strlen (file) +
                    (os_size_t)snprintf (tmp, sizeof(tmp), "%d", line) +
                    (os_size_t)snprintf (tmp, sizeof(tmp), "%d", code) +
                    (os_size_t)snprintf (tmp, sizeof(tmp), "%d", pid) +
                    strlen (proc) +
                    (os_size_t)snprintf (tmp, sizeof(tmp), "%lu", tid) +
                    strlen (thr) +
                    strlen (os__report_version) +
                    strlen (os__report_inner_revision) +
                    strlen (os__report_outer_revision);

            os_iterWalk (
                    _this->reports, &os__report_stack_event_length, &buf.len);

            buf.str = (os_char *)os_malloc(buf.len);
            if (buf.str != NULL) {
                buf.off = (os_size_t)snprintf (
                        buf.str,
                        buf.len,
                        os__report_xml_head,
                        os_reportTypeText[reportType],
                        domainId,
                        OS_REPORT_BUFLEN,
                        description,
                        context,
                        file,
                        line,
                        code,
                        pid, proc,
                        tid, thr,
                        os__report_version,
                        os__report_inner_revision, os__report_outer_revision);

                os_iterWalk (
                        _this->reports, &os__report_stack_event_print, &buf);

                buf.off = (os_size_t)snprintf (
                        buf.str + buf.off,
                        buf.len - buf.off,
                        os__report_xml_tail,
                        os_reportTypeText[reportType]);
            }
        }

        if (doDefault) {
            header.reportType = reportType;
            header.description = (os_char *)context;
            header.processDesc = procid;
            header.threadDesc = thrid;
            header.fileName = file;
            header.lineNo = line;

            os_headerReport (&header, useErrorLog, domainId);
        }
    }

    while ((report = os_iterTakeLast (_this->reports))) {
        if (valid) {
            if (update == OS_TRUE && os_report_iserror (report)) {
                os_reportSetApiInfo (
                        report->reportContext,
                        report->fileName,
                        report->lineNo,
                        report->code,
                        report->description);
                update = OS_FALSE;
            }

            if (doDefault) {
                os_sectionReport (report, useErrorLog);
            }
            if (reportPluginAdmin != NULL) {
                os_mutexLock(&reportPluginMutex);
                plugin = reportPluginAdmin;
                while (plugin != NULL) {
                    if ((domainId == -1) || (domainId == plugin->domainId)) {
                        if (plugin->typedreport_symbol != NULL) {
                            plugin->typedreport_symbol (
                                    plugin->plugin_context, (os_reportEvent) report);
                        }
                    }
                    plugin = plugin->pstNext;
                }
                os_mutexUnlock(&reportPluginMutex);
            }
        }
        os__report_free(report);
    }

    if (valid && (reportPluginAdmin != NULL)) {
        os_mutexLock(&reportPluginMutex);
        plugin = reportPluginAdmin;
        while (plugin != NULL) {
            if ((domainId == -1) || (domainId == plugin->domainId)) {
                if ((plugin->report_symbol != NULL) && (buf.str != NULL)) {
                    plugin->report_symbol (
                            plugin->plugin_context, buf.str);
                }
            }
            plugin = plugin->pstNext;
        }
        os_mutexUnlock(&reportPluginMutex);
    }
    os_free (buf.str);
}

static void
os__report_stack_clear(
		os_reportStack _this)
{
    os_reportEventV1 report;

	while ((report = os_iterTakeLast (_this->reports))) {
		os__report_free(report);
	}
}

void
os_report_dumpStack(
        const os_char *context,
        const os_char *path,
        os_int line)
{
    os_reportStack _this;

    if (inited == OS_FALSE) {
        return;
    }
    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count > 0)) {
        os__report_stack_unwind(_this, TRUE, context, path, line, -1);
    }
}

os_boolean
os_report_stack_flush_required(
    _In_ os_boolean valid)
{
	os_reportStack _this;
	os_boolean flush = OS_FALSE;

	_this = os_threadMemGet(OS_THREAD_REPORT_STACK);
	if (_this && _this->count) {
		if (_this->count == 1) {
			if (valid ||
				OS_REPORT_ALWAYS(_this->typeset)     ||
			    OS_REPORT_WARNING(_this->typeset)    ||
			    OS_REPORT_DEPRECATED(_this->typeset)) {
				flush = OS_TRUE;
			} else {
				os__report_stack_clear(_this);
				_this->count--;
			}
		} else {
			_this->count--;
		}
	}

	return flush;
}

void
os_report_stack_unwind(
    _In_ os_boolean valid,
    _In_ const os_char *context,
    _In_ const os_char *path,
    _In_ os_int line,
    _In_ os_int32 domainId)
{
	os_reportStack _this;

	if (inited == OS_FALSE) {
		return;
	}
	_this = os_threadMemGet(OS_THREAD_REPORT_STACK);
	if ((_this) && (_this->count)) {
		assert(_this->count == 1);
		os__report_stack_unwind(_this, valid, context, path, line, domainId);
		_this->file = NULL;
		_this->signature = NULL;
		_this->lineno = 0;
		_this->count--;
	}
}

void
os_report_stack_flush(
    _In_ os_boolean valid,
    _In_ const os_char *context,
    _In_ const os_char *path,
    _In_ os_int line,
    _In_ os_int32 domainId)
{
    os_reportStack _this;

    if (inited == OS_FALSE) {
        return;
    }
    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count)) {
        if (_this->count == 1) {
            os__report_stack_unwind(_this, valid, context, path, line, domainId);
            _this->file = NULL;
            _this->signature = NULL;
            _this->lineno = 0;
        }
        _this->count--;
    }
}


void
os_report_flush_context(
        os_boolean valid,
        os_report_context_callback callback,
        void *arg)
{
    os_reportStack _this;
    os_char buffer[1024];
    const os_char *context = NULL;

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count)) {
        if (_this->count == 1) {
            if ((os_char *)callback) {
                context = callback(_this->signature, buffer, sizeof(buffer), arg);
            }
            if (context == NULL) {
                context = _this->signature;
            }
            os__report_stack_unwind(_this, valid, context, _this->file, _this->lineno, -1);
            _this->file = NULL;
            _this->signature = NULL;
            _this->lineno = 0;
        }
        _this->count--;
    }
}

void
os_report_flush_context_unconditional(
        os_report_context_callback callback,
        void *arg)
{
    os_reportStack _this;
    os_char buffer[1024];
    const os_char *context = NULL;

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count)) {
        if ((os_char *)callback) {
            context = callback(_this->signature, buffer, sizeof(buffer), arg);
        }
        if (context == NULL) {
            context = _this->signature;
        }
        os__report_stack_unwind(_this, OS_TRUE, context, _this->file, _this->lineno, -1);
        _this->file = NULL;
        _this->signature = NULL;
        _this->lineno = 0;
        _this->count = 0;
    }
}

void
os_report_flush_unconditional(
        os_boolean valid,
        const os_char *context,
        const os_char *path,
        os_int line,
		os_int32 domainId)
{
    os_reportStack _this;

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if ((_this) && (_this->count)) {
        os__report_stack_unwind(_this, valid, context, path, line, domainId);
        _this->file = NULL;
        _this->signature = NULL;
        _this->lineno = 0;
        _this->count = 0;
    }
}

os_int32
os_report_stack_size()
{
    os_reportStack _this;
    os_int32 result = -1; /* -1 means disabled */

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (_this && _this->count) {
        result = (os_int32) os_iterLength(_this->reports);
    }
    return result;
}

os_reportEventV1
os_report_read(
        os_int32 index)
{
    os_reportEventV1 report = NULL;
    os_reportStack _this;

    _this = os_threadMemGet(OS_THREAD_REPORT_STACK);
    if (_this) {
        if (index < 0) {
            report = NULL;
        } else {
            report = (os_reportEventV1)os_iterObject(_this->reports, (os_uint32) index);
        }
    } else {
        OS_REPORT(OS_ERROR, "os_report_read", 0,
                "Failed to retrieve report administration from thread-specific memory");
    }
    return report;
}

#define OS__STRDUP(str) (str != NULL ? os_strdup(str) : os_strdup("NULL"))

void
os__report_append(
        os_reportStack _this,
        const os_reportEventV1 report)
{
    os_reportEventV1 copy;
    assert(report);

    copy = os_malloc(sizeof(*copy));
    if (copy) {
        copy->code = report->code;
        copy->description = OS__STRDUP(report->description);
        copy->fileName = OS__STRDUP(report->fileName);
        copy->lineNo = report->lineNo;
        copy->processDesc = OS__STRDUP(report->processDesc);
        copy->reportContext = OS__STRDUP(report->reportContext);
        copy->reportType = report->reportType;
        copy->threadDesc = OS__STRDUP(report->threadDesc);
        copy->version = report->version;
        _this->typeset |= OS_REPORT_TYPE_FLAG(report->reportType);
        os_iterAppend(_this->reports, copy);
    } else {
        os__report_fprintf(stderr, "Failed to allocate %d bytes for log report!", (int)sizeof(*copy));
        os__report_fprintf(stderr, "Report: %s\n", report->description);
    }
}

void
os__report_free(os_reportEventV1 report)
{
    os_free(report->description);
    os_free(report->fileName);
    os_free(report->processDesc);
    os_free(report->reportContext);
    os_free(report->threadDesc);
    os_free(report);
}

static int
os__report_fprintf(FILE *file,
        const char *format,
        ...)
{
    int BytesWritten = 0;
    va_list args;
    va_start(args, format);
    BytesWritten = os_vfprintfnosigpipe(file, format, args);
    va_end(args);
    if (BytesWritten == -1) {
        /* error occured ?, try to write to stdout. (also with no sigpipe,
         * stdout can also give broken pipe)
         */
        va_start(args, format);
        os_vfprintfnosigpipe(stdout, format, args);
        va_end(args);
    }
    return BytesWritten;
}

void
os_reportRegisterDomainCallback(
        os_reportGetDomainCallback callback,
        void *arg)
{
    domainCallbackInfo.callback = callback;
    domainCallbackInfo.argument = arg;
}

os_int32
os_reportGetDomain(void)
{
    if (domainCallbackInfo.callback) {
        return domainCallbackInfo.callback(domainCallbackInfo.argument);
    }
    return -1;
}




