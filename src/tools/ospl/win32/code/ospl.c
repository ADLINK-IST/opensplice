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
#include "ospl_proc.h"

#include "os.h"
#include "os_iterator.h"
#include "os_report.h"
#include "os_signal.h"

#include "cfg_parser.h"
#include "cf_config.h"

#define SPLICED_NAME "spliced" OS_EXESUFFIX

/* These defines of exit codes are mirrored in the following files:
 * - src/tools/ospl/unix/code/ospl.c
 * - src/tools/ospl/win32/code/ospl.c
 * - src/services/spliced/code/spliced.h
 */
#define OSPL_EXIT_CODE_OK 0

#define OSPL_EXIT_CODE_RECOVERABLE_ERROR -1

#define OSPL_EXIT_CODE_UNRECOVERABLE_ERROR -2

static void
print_usage(
    char *name)
{
    printf ("\nUsage:\n"
            "      ospl -h\n"
            "      ospl -v\n"
            "      ospl [-f] start [URI]\n"
            "      ospl [[-d <domain> | -a] stop [URI]]\n"
            "      ospl list\n\n"
            "      -h       Show this help\n\n"
            "      -v       Show the ospl version information\n\n");
    printf ("      start    Start the identified system\n\n"
            "               The system is identified and configured by the URI which is\n"
            "               defined by the environment variable OSPL_URI. This setting can\n"
            "               be overruled with the command line URI definition. When none of\n"
            "               the URI definitions is specified, a default system will be\n"
            "               started.\n");
    printf ("               Upon exit an exit code will be provided to indicated the cause of\n"
            "               termination. The following exit codes are supported:\n"
            "               * 0 : normal termination as result of OSPL stop.\n"
            "               * -1: a recoverable error occurred.\n"
            "                     The system has encountered a runtime error and has terminated.\n"
            "                     A restart of the system is possible. E.g., the system ran out\n"
            "                     of resources.\n");
    printf ("               * -2: an unrecoverable error occurred.\n"
            "                     The system has encountered an error that cannot be resolved by\n"
            "                     a restart of the system. E.g., the configuration file contains\n"
            "                     errors or does not exist.\n");
    printf ("               When the -f option is specified the ospl command will not return\n"
            "               directly, but it will instead block until termination of the\n"
            "               OpenSpliceDDS deamon. If the -f option is not specified the ospl\n"
            "               command will return immediately after start up.\n\n");
    printf ("      stop     Stop the identified system\n\n"
            "               Stop is the default command, thus when no command is specified\n"
            "               stop is assumed. The system to stop is identified by the URI\n"
            "               which is defined by the environment variable OSPL_URI. This\n");
    printf ("               setting can be overruled by the command line URI definition or\n"
            "               the domain name which is associated with the URI and specified\n"
            "               via the -d option. \n"
            "               When no domain is specified by the URI or by it's name a \n"
            "               default system is assumed. The -a options specifies to stop all\n"
            "               running splice systems started by the current user.\n");
    printf ("               Upon exit an exit code will be provided to indicated the cause\n"
            "               of termination. The following exit codes are supported:\n"
            "               * 0 : normal termination as result of  OSPL stop.\n"
            "               * -1: Not Applicable\n");
    printf ("               * -2: an unrecoverable error occurred.\n"
            "                     The system has encountered an error that cannot be\n"
            "                     resolved by a retry of the same command. E.g., A\n"
            "                     faulty URI was provided.\n\n");
    printf ("      list     Show all systems started by the current user by their domain\n"
            "               name\n\n");
}

static char *key_file_path = NULL;
static const char * const key_file_prefix = "osp";

#define MAX_STATUSCHECKS (30)

static void
removeProcesses(
    os_procId pid)
{
    os_result r;
    os_int32 procResult;
    int i = MAX_STATUSCHECKS;

    r = os_procDestroy(pid, OS_SIGTERM);
    r= os_procCheckStatus((os_procId)pid, &procResult);
    while ((r == os_resultBusy) && (i > 0)) {
        i--;
        printf (".");
        fflush(stdout);
        Sleep(1000);
        r = os_procCheckStatus(pid, &procResult);
    }
    kill_descendents ((DWORD) os_procIdToInteger(pid), OS_SIGKILL);
}

#undef MAX_STATUSCHECKS


static void
removeKeyfile(
    const char *key_file_name)
{
    char dbf_file_name[MAX_PATH];
    char * ptr;

    /* try to unlink the key file. This is a fallback option. In normal circumstances
     * the spliced process will have deleted the key file, but something it failed
     * and we have to unlink it here. But in general the unlink call below will
     * fail because the file is already gone. This failure can be ignored.
     */
    os_remove(key_file_name);

    /* as this is a fallback option we should also try to remove the corresponding
     * DBF file as the process may have been killed leaving both files behind.
     */
    os_strcpy(dbf_file_name, key_file_name);
    ptr = strchr(dbf_file_name, '.');
    dbf_file_name[ptr - dbf_file_name + 1] = 'D';
    dbf_file_name[ptr - dbf_file_name + 2] = 'B';
    dbf_file_name[ptr - dbf_file_name + 3] = 'F';
    os_remove(dbf_file_name);
}

static void
clean(void)
{
    HANDLE hFile;
    HANDLE hProcess;
    WIN32_FIND_DATA fileData;
    os_char key_file_name [MAX_PATH];
    os_char dbf_file_name [MAX_PATH];
    os_char buf[512];
    os_uint32 last = 0;
    os_uint32 pid;
    FILE *key_file;

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    hFile = FindFirstFile(key_file_name, &fileData);

    if (hFile != INVALID_HANDLE_VALUE) {
       os_strcpy(key_file_name, key_file_path);
       os_strcat(key_file_name, "\\");
       os_strcat(key_file_name, fileData.cFileName);
       key_file = fopen(key_file_name, "r");

       while (!last) {
          if (key_file != NULL) {
             if (fgets(buf, sizeof(buf), key_file) != NULL) {
                fgets(buf, sizeof(buf), key_file);
                fgets(buf, sizeof(buf), key_file);
                fgets(buf, sizeof(buf), key_file);
                fgets(buf, sizeof(buf), key_file);
                sscanf(buf, "%d", &pid);
                fclose(key_file);

                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
                if (hProcess == NULL) {
                   /* no associated process - remove tmp and DBF file */
                   removeKeyfile(key_file_name);

                   /* kill of orphaned processes e.g. durability/networking */
                   kill_descendents (pid, OS_SIGKILL);
                }
             }
          }

          if (FindNextFile(hFile, &fileData) == 0) {
             last = 1;
          } else {
             os_strcpy(key_file_name, key_file_path);
             os_strcat(key_file_name, "\\");
             os_strcat(key_file_name, fileData.cFileName);
             key_file = fopen(key_file_name, "r");
          }
       }
    }
    FindClose(hFile);

    /* We now have to check for abandoned DBF files where there is no corresponding
    *  tmp file.If there is a corresponding tmp file then it will be a valid pair
    *  as we have already removed the invalid tmp files.
    */
    last = 0;
    os_strcpy(dbf_file_name, key_file_path);
    os_strcat(dbf_file_name, "\\");
    os_strcat(dbf_file_name, key_file_prefix);
    os_strcat(dbf_file_name, "*.DBF");

    hFile = FindFirstFile(dbf_file_name, &fileData);

    if (hFile != INVALID_HANDLE_VALUE) {
       os_strcpy(dbf_file_name, key_file_path);
       os_strcat(dbf_file_name, "\\");
       os_strcat(dbf_file_name, fileData.cFileName);

       while (!last) {
          char * ptr;

          /* check that it does not have a matching tmp file, any matching tmp file will be valid */
          os_strcpy(key_file_name, dbf_file_name);
          ptr = strchr(key_file_name, '.');
          key_file_name[ptr - key_file_name + 1] = 't';
          key_file_name[ptr - key_file_name + 2] = 'm';
          key_file_name[ptr - key_file_name + 3] = 'p';

          key_file = fopen(key_file_name, "r");
          if (key_file == NULL) {
             /* there is no matching tmp file, delete the DBF file */
             unlink(dbf_file_name);
          }
          else
          {
             fclose(key_file);
          }

          if (FindNextFile(hFile, &fileData) == 0) {
             last = 1;
          } else {
             os_strcpy(dbf_file_name, key_file_path);
             os_strcat(dbf_file_name, "\\");
             os_strcat(dbf_file_name, fileData.cFileName);
          }
       }
    }
    FindClose(hFile);
}

static int
shutdownDDS(
    const char *key_file_name,
    const char *domain_name)
{
    char uri[512];
    char map_address[64];
    char size[64];
    char implementation[64];
    char creator_pid[64];
    int pid;
    FILE *kf;
    int len;
    int retCode = OSPL_EXIT_CODE_OK;

    printf("\nShutting down domain \"%s\" ", domain_name);
    kf = fopen(key_file_name, "r");
    if (kf) {
       HANDLE hProcess;

       fgets(uri, sizeof(uri), kf);
       len = strlen(uri);
       if (len > 0) {
          uri[len-1] = 0;
       }
       fgets(map_address, sizeof(map_address), kf);
       fgets(size, sizeof(size), kf);
       fgets(implementation, sizeof(implementation), kf);
       fgets(creator_pid, sizeof(creator_pid), kf);
       fclose(kf);
       printf("Signalling Shutdown\n\n");

       sscanf(creator_pid, "%d", &pid);

       hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

       if (hProcess == INVALID_HANDLE_VALUE) {
          int err = GetLastError();
          printf("Access was denied for the process (%d) with error code (%d)", pid, err);
          return os_resultFail;
       }
       else {
          removeProcesses((os_procId)hProcess);
          removeKeyfile(key_file_name);
       }
    } else {
        /* unrecoverable, can not read keyfile.*/
        retCode = OSPL_EXIT_CODE_UNRECOVERABLE_ERROR;
    }
    printf(" Ready\n\n");

    return retCode;
}

static char *
matchKey(
    const char *key_file_name,
    const char *domain_name)
{
    FILE *key_file;
    char domain[512];
    int len;

    key_file = fopen(key_file_name, "r");
    if (key_file != NULL) {
        if (fgets(domain, sizeof(domain), key_file) != NULL) {
            len = strlen(domain);
            if (len > 0) {
                domain[len-1] = 0;
            }
            if ((domain_name == NULL) ||
                (strcmp(domain_name, "*") == 0) ||
                (strcmp(domain_name, domain) == 0)) {
                fclose(key_file);
                return os_strdup(domain);
            }
        }
        fclose(key_file);
    }
    return NULL;
}

static int
findSpliceSystemAndRemove(
    const char *domain_name)
{
    HANDLE hFile;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    int last = 0;
    char *shmName;
    int retCode = OSPL_EXIT_CODE_OK;

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    hFile = FindFirstFile(key_file_name, &fileData);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);

        while (!last) {
            shmName = matchKey(key_file_name, domain_name);
            if (shmName != NULL) {
                retCode = shutdownDDS(key_file_name, shmName);
                free(shmName);
            }

            if (FindNextFile(hFile, &fileData) == 0) {
                last = 1;
            } else {
                os_strcpy(key_file_name, key_file_path);
                os_strcat(key_file_name, "\\");
                os_strcat(key_file_name, fileData.cFileName);
            }
        }
        FindClose(hFile);
    } else
    {
        retCode = OSPL_EXIT_CODE_UNRECOVERABLE_ERROR;
    }
    return retCode;

}

static int
findSpliceSystemAndShow(const char* specific_domain_name)
{
    HANDLE hFile;
    WIN32_FIND_DATA fileData;
    char key_file_name [MAX_PATH];
    char uri[512];
    int last = 0;
    FILE *key_file;
    int len;
    int found_count;

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    hFile = FindFirstFile(key_file_name, &fileData);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    found_count = 0;
    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, fileData.cFileName);
    key_file = fopen(key_file_name, "r");

    while (!last) {
        if (key_file != NULL) {
            if (fgets(uri, sizeof(uri), key_file) != NULL) {
                len = strlen(uri);
                if (len > 0) {
                    uri[len-1] = 0;
                }
                /* Confusion between URIs and domain names here. And elsewhere.
                This 'uri' is the domain name from the tag of that name in the XML config file */
                if (specific_domain_name == NULL
                    || (strcmp(specific_domain_name, uri) == 0))
                {
                    printf("Splice System with domain name \"%s\" is found running\n", uri);
                    ++found_count;
                }
            }
        }

        if (FindNextFile(hFile, &fileData) == 0) {
            last = 1;
        } else {
            fclose(key_file);
            os_strcpy(key_file_name, key_file_path);
            os_strcat(key_file_name, "\\");
            os_strcat(key_file_name, fileData.cFileName);
            key_file = fopen(key_file_name, "r");
        }
    }
    fclose(key_file);
    FindClose(hFile);
    return found_count;
}

static int
spliceSystemRunning(
    char *domain)
{
    HANDLE hFile;
    HANDLE hProcess;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    int last = 0;
    FILE *key_file;
    char uri[512];
    char buf[64];
    int len;
    int found = 0;
    int search = 0;
    int pid;
    os_int32 procResult;

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    hFile = FindFirstFile(key_file_name, &fileData);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, fileData.cFileName);
    key_file = fopen(key_file_name, "r");

    while ((!last) && (search < 2)) { /* search can be restarted 1 time! */
        if (key_file != NULL) {
            if (fgets(uri, sizeof(uri), key_file) != NULL) {
                len = strlen(uri);
                if (len > 0) {
                    uri[len-1] = 0;
                }
                if (strcmp(domain, uri) == 0) {
                    fgets(buf, sizeof(buf), key_file);
                    fgets(buf, sizeof(buf), key_file);
                    fgets(buf, sizeof(buf), key_file);
                    fgets(buf, sizeof(buf), key_file);
                    sscanf(buf, "%d", &pid);

                    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
                    if (hProcess != NULL) {
                        if (os_procCheckStatus((os_procId)hProcess, &procResult) == os_resultBusy) {
                            FindClose(hFile);
                            CloseHandle(hProcess);
                            fclose(key_file);
                            return 1;
                        }
                    }
                    FindClose(hFile);
                    fclose(key_file);
                    /* try to delete the file in case applications/services were not terminated correctly! */
                    removeKeyfile(key_file_name);

                    search++;
                    /* restart search! */
                    os_strcpy(key_file_name, key_file_path);
                    os_strcat(key_file_name, "\\");
                    os_strcat(key_file_name, key_file_prefix);
                    os_strcat(key_file_name, "*.tmp");
                    hFile = FindFirstFile(key_file_name, &fileData);
                    if (hFile == INVALID_HANDLE_VALUE) {
                        return 0;
                    }

                    os_strcpy(key_file_name, key_file_path);
                    os_strcat(key_file_name, "\\");
                    os_strcat(key_file_name, fileData.cFileName);
                    key_file = fopen(key_file_name, "r");
                    continue;
                }
            }
        }

        if (FindNextFile(hFile, &fileData) == 0) {
            last = 1;
        } else {
            fclose(key_file);
            os_strcpy(key_file_name, key_file_path);
            os_strcat(key_file_name, "\\");
            os_strcat(key_file_name, fileData.cFileName);
            key_file = fopen(key_file_name, "r");
        }
    }
    fclose(key_file);
    FindClose(hFile);

    return found;
}

static char *
findDomain(
    cf_element platformConfig)
{
    char *domain_name = NULL;
    cf_element dc = NULL;
    cf_element elemName = NULL;
    cf_data dataName;
    c_value value;

    dc = cf_element(cf_elementChild(platformConfig, CFG_DOMAIN));
    if (dc) {
        elemName = cf_element(cf_elementChild(dc, "Name"));
        if (elemName) {
            dataName = cf_data(cf_elementChild(elemName, "#text"));
            if (dataName) {
                value = cf_dataValue(dataName);
                domain_name = os_malloc(strlen(value.is.String) + 2);
                os_strcpy(domain_name, value.is.String);
                os_free(value.is.String);
            }
        }
    }
    return domain_name;
}

static int
isOneOf(
    char c,
    const char *symbolList)
{
    const char *symbol;

    symbol = symbolList;
    while ((symbol != NULL) && (*symbol != '\0')) {
        if (c == *symbol) {
            return TRUE;
        }
        symbol++;
    }
    return FALSE;
}

static char *
skipUntil(
    const char *str,
    const char *symbolList)
{
    char *ptr = (char *)str;

    assert(symbolList != NULL);
    if (ptr == NULL) {
        return NULL;
    }

    while ((*ptr != '\0') && (!isOneOf(*ptr,symbolList))) {
        ptr++;
    }

    return ptr;
}

static os_iter
splitString(
    const char *str,
    const char *delimiters)
{
    const char *head, *tail;
    char *nibble;
    os_iter iter = NULL;
    int length;

    if (str == NULL) {
        return NULL;
    }

    tail = str;
    while (*tail != '\0') {
        head = skipUntil(tail,delimiters);
        length = abs(head - tail);
        if (length != 0) {
            length++;
            nibble = os_malloc(length);
            os_strncpy(nibble,tail,length);
            nibble[length-1]=0;
            iter = os_iterAppend(iter,nibble);
        }
        tail = head;
        if (isOneOf(*tail,delimiters)) {
            tail++;
        }
    }
    return iter;
}

static void
safeUri(
    char **uri)
{
    if (*uri != NULL) {
        *uri = os_strdup(*uri);
    }
}

#ifdef WINDOWS_SERVICE
int
ospl_main(
     int argc,
     char *argv[])
#else
OPENSPLICE_MAIN(ospl)
#endif
{
    int opt;
    int retCode = OSPL_EXIT_CODE_OK;
    char *domain_name = NULL;
    char *uri = NULL;
    char *command = NULL;
    os_time sleepTime;
    os_result result;
    os_int32 status;

    cf_element platformConfig = NULL;
    cfgprs_status r;
    os_procAttr pa;
    os_procId pi;
    os_boolean blocking = OS_FALSE;
    os_boolean blockingDefined = OS_FALSE;

    os_osInit();

    uri = os_getenv("OSPL_URI");

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    /* (re)set default optind & opterr to make this re-entrant */
    optind = 1;
    opterr = 1;

    while ((opt = getopt(argc, argv, "hvafd:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(argv[0]);
            exit(OSPL_EXIT_CODE_OK);
            break;
        case 'v':
            printf ("OpenSplice version : %s\n", VERSION);
            exit(OSPL_EXIT_CODE_OK);
            break;
        case 'd':
            if (domain_name)
            {
                print_usage(argv[0]);
                exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
            domain_name = optarg;
            break;
        case 'a':
            if (domain_name)
            {
                print_usage(argv[0]);
                exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
            uri = NULL;
            domain_name = "*";
            break;
        case 'f':
            if(blockingDefined)
            {
                print_usage (argv[0]);
                exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
            blocking = OS_TRUE;
            blockingDefined = OS_TRUE;
            break;
        case '?':
            print_usage(argv[0]);
            exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            break;
        default:
            break;
        }
    }
    if ((argc-optind) > 3)
    {
        print_usage(argv[0]);
        exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
    }
    if (key_file_path == NULL) {
        fprintf(stderr, "No temporary directory path found\n");
        exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
    }

    command = argv[optind];

    if (command && argv[optind+1])
    {
        uri = argv[optind+1];
    }
    safeUri(&uri);
    if (uri && (strlen(uri) > 0))
    {
        r = cfg_parse_ospl(uri, &platformConfig);
        if (r == CFGPRS_OK)
        {
            domain_name = findDomain(platformConfig);
            if (domain_name == NULL)
            {
                printf("The domain name could not be determined from the configuration\n");
                exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
        } else
        {
            if (r == CFGPRS_NO_INPUT)
            {
                printf ("Error: Cannot open URI \"%s\". Exiting now...\n", uri);
            }
            else
            {
                printf ("Errors are detected in the configuration. Exiting now...\n");
            }
            exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
        }
    }

    if ((command == NULL) || (strcmp(command, "stop") == 0))
    {
        if (domain_name == NULL)
        {
            domain_name = "The default Domain";
        }

        retCode = findSpliceSystemAndRemove(domain_name);
    } else {
        if (strcmp (command, "start") == 0)
        {
            clean();
            if (domain_name == NULL)
            {
                domain_name = "The default Domain";
            }
            if (!spliceSystemRunning(domain_name))
            {
                if(!blocking)
                {
                    printf("\nStarting up domain \"%s\" .\n", domain_name);
                } else
                {
                    printf("\nStarting up domain \"%s\" and blocking.\n", domain_name);
                }

                /* Display locations of info and error files */
                os_reportDisplayLogLocations();

                if (uri == NULL)
                {
                    uri = os_strdup("");
                }
                command = os_locate(SPLICED_NAME, OS_ROK|OS_XOK);
                os_procAttrInit(&pa);
                os_procCreate(command, "OpenSplice Control Service", uri, &pa, &pi);
                if(blocking)
                {
                    /* just check every 100 ms if the deamon has terminated already */
                    sleepTime.tv_sec = 0;
                    sleepTime.tv_nsec = 100000000; /*100 ms*/
                    result = os_procCheckStatus(pi, &status);
                    while(result != os_resultSuccess)
                    {
                        result = os_procCheckStatus(pi, &status);
                        os_nanoSleep(sleepTime);
                    }
                } else
                {
                    Sleep(2000); /* take time to first show the license message from spliced */
                }
                CloseHandle (pi);
            } else
            {
                printf("Splice System with domain name \"%s\" is found running, ignoring command\n",
                    domain_name);
                if (blocking)
                {
                    /* If we requested blocking start then this is an error as we did not succeed
                    in starting the service and then blocking until spliced exits as per */
                    retCode = OSPL_EXIT_CODE_RECOVERABLE_ERROR;
                }
            }
        } else
        {
            if (strcmp(command, "list") == 0)
            {
                return findSpliceSystemAndShow(domain_name);
            } else
            {
                print_usage(argv[0]);
                exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
        }
    }
    os_free(uri);
    uri = NULL;
    return retCode;
}

