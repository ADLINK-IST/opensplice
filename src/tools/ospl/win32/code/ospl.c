#include <os.h>
#include <os_iterator.h>

#include <cfg_parser.h>
#include <cf_config.h>

#define SPLICED_NAME "spliced" OS_EXESUFFIX

static void
print_usage(
    char *name)
{
    printf ("\nUsage:\n"
            "      ospl -h\n"
            "      ospl start [URI]\n"
            "      ospl [[-d <domain> | -a] stop [URI]]\n"
            "      ospl list\n\n"
            "      -h       Show this help\n\n");
    printf ("      start    Start the identified system\n\n"
            "               The system is identified and configured by the URI which is defined\n"
            "               by the environment variable OSPL_URI. This setting can\n"
            "               be overruled with the command line URI definition. When none of the\n"
            "               URI definitions is specified, a default system will be started.\n\n");
    printf ("      stop     Stop the identified system\n\n"
            "               Stop is the default command, this when no command is specified stop\n"
            "               is assumed. The system to stop is identified by the URI which is defined\n"
            "               by the environment variable OSPL_URI. This setting can\n");
    printf ("               be overruled by the command line URI definition or the domain name\n"
            "               which is associated with the URI and specified via the -d option\n"
            "               When no domain is specified by the URI or by it's name a default\n"
            "               system is assumed. The -a options specifies to stop all running\n"
            "               splice systems started by the current user.\n\n");
    printf ("      list     Show all systems started by the current user by their domain name\n\n");
}

static char *key_file_path = NULL;
static const char * const key_file_prefix = "osp";

static void
removeProcesses(
    int pid)
{
    os_result r;
    os_int32 procResult;

    r = os_procDestroy(pid, OS_SIGTERM);
    r= os_procCheckStatus((os_procId)pid, &procResult);
    while (r == os_resultBusy) {
        printf (".");
        fflush(stdout);
        Sleep(1000);
        r = os_procCheckStatus((os_procId)pid, &procResult);
    }
}

static void
removeKeyfile(
    const char *key_file_name)
{
    unlink(key_file_name);
}

static void
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

    printf("\nShutting down domain \"%s\" ", domain_name);
    kf = fopen(key_file_name, "r");
    if (kf) {
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
        removeProcesses(pid);
        removeKeyfile(key_file_name);
    }
    printf(" Ready\n\n");
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

static void
findSpliceSystemAndRemove(
    const char *domain_name)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    int last = 0;
    char *shmName;

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, key_file_prefix);
    strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, fileData.cFileName);

    while (!last) {
        shmName = matchKey(key_file_name, domain_name);
        if (shmName != NULL) {
            shutdownDDS(key_file_name, shmName);
            free(shmName);
        }

        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        } else {
            strcpy(key_file_name, key_file_path);
            strcat(key_file_name, "\\");
            strcat(key_file_name, fileData.cFileName);
        }
    }
    FindClose(fileHandle);
}

static void
findSpliceSystemAndShow(void)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name [MAX_PATH];
    char uri[512];
    int last = 0;
    FILE *key_file;
    int len;

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, key_file_prefix);
    strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, fileData.cFileName);
    key_file = fopen(key_file_name, "r");

    while (!last) {
        if (key_file != NULL) {
            if (fgets(uri, sizeof(uri), key_file) != NULL) {
                len = strlen(uri);
                if (len > 0) {
                    uri[len-1] = 0;
                }
                printf("Splice System with domain name \"%s\" is found running\n", uri);
            }
        }

        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        } else {
            fclose(key_file);
            strcpy(key_file_name, key_file_path);
            strcat(key_file_name, "\\");
            strcat(key_file_name, fileData.cFileName);
            key_file = fopen(key_file_name, "r");
        }
    }
    fclose(key_file);
    FindClose(fileHandle);
}

static int
spliceSystemRunning(
    char *domain)
{
    HANDLE fileHandle;
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

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, key_file_prefix);
    strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    strcpy(key_file_name, key_file_path);
    strcat(key_file_name, "\\");
    strcat(key_file_name, fileData.cFileName);
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
                    if (os_procCheckStatus((os_procId)pid, &procResult) == os_resultBusy) {
                        FindClose(fileHandle);
                        fclose(key_file);
                        return 1;
                    }
                    FindClose(fileHandle);
                    fclose(key_file);
     /* try to delete the file in case applications/services were not terminated correctly! */
                    removeKeyfile(key_file_name);
                    search++;
                    /* restart search! */
                    strcpy(key_file_name, key_file_path);
                    strcat(key_file_name, "\\");
                    strcat(key_file_name, key_file_prefix);
                    strcat(key_file_name, "*.tmp");
                    fileHandle = FindFirstFile(key_file_name, &fileData);
                    if (fileHandle == INVALID_HANDLE_VALUE) {
                        return 0;
                    }

                    strcpy(key_file_name, key_file_path);
                    strcat(key_file_name, "\\");
                    strcat(key_file_name, fileData.cFileName);
                    key_file = fopen(key_file_name, "r");
                    continue;
                }
            }
        }

        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        } else {
            fclose(key_file);
            strcpy(key_file_name, key_file_path);
            strcat(key_file_name, "\\");
            strcat(key_file_name, fileData.cFileName);
            key_file = fopen(key_file_name, "r");
        }
    }
    fclose(key_file);
    FindClose(fileHandle);

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
                strcpy(domain_name, value.is.String);
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
            strncpy(nibble,tail,length);
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

#if 0
/* No longer needed, we now have os_locate */
static int
findSpliced(
    char **command)
{
    int valid;
    char *path;
    char *d;
    char *c;
    const char *fsep;
    os_iter dirs;

    assert(command != NULL);
    assert(*command != NULL);

    valid = TRUE;
    /* If the command contains an absolute or relative path,
       only check the permissions, otherwise search the file
       in the PATH environment
    */
    fsep = os_fileSep();
    if ((**command == '.') || (strncmp(*command, fsep, strlen(fsep)) == 0)) {
        if (os_access(*command, OS_ROK | OS_XOK) == os_resultSuccess) {
            valid = TRUE;
        } else {
            valid = FALSE;
        }
    } else {
        valid = FALSE;
        path = os_getenv("PATH");
        dirs = splitString(path, ";"); /* ':' for unix and ';' for windows */
        d = (char *)os_iterTakeFirst(dirs);
        while (d != NULL) {
            if (valid == FALSE) {
                c = (char *)os_malloc(strlen(d) + strlen(fsep) + strlen(*command) + 1);
                if (c != NULL) {
                    strcpy(c, d);
                    strcat(c, fsep);
                    strcat(c, *command);
                    /* Check file permissions. Do not have to check if file exists, since
                       permission check fails when the file does not exist.
                    */
                    if (os_access(c, OS_ROK | OS_XOK) == os_resultSuccess) {
                        valid = TRUE;
                        os_free(*command);
                        *command = c;
                        c = NULL;
                    }
                }
                os_free(c);
            }
            os_free(d);
            d = (char *)os_iterTakeFirst(dirs);
        }
        os_iterFree(dirs);
    }

    return valid;
}
#endif

int
main(
    int argc,
    char *argv[])
{
    int opt;
    char *domain_name = NULL;
    char *uri = NULL;
    char *command = NULL;
    cf_element platformConfig = NULL;
    cfgprs_status r;
    os_procAttr pa;
    os_procId pi;

    os_osInit();
    uri = os_getenv("OSPL_URI");

    if (key_file_path == NULL) {
        key_file_path = os_getenv("OSPL_TEMP");
    }
    if (key_file_path == NULL) {
        key_file_path = os_getenv("TEMP");
    }
    if (key_file_path == NULL) {
        key_file_path = os_getenv("TMP");
    }

    while ((opt = getopt(argc, argv, "had:")) != -1) {
        switch (opt) {
        case 'h':
            print_usage(argv[0]);
            exit(0);
        break;
        case 'd':
            if (domain_name) {
                print_usage(argv[0]);
                exit (-1);
            }
            domain_name = optarg;
        break;
        case 'a':
            if (domain_name) {
                print_usage(argv[0]);
                exit(-1);
            }
            uri = NULL;
            domain_name = "*";
        break;
        case '?':
            print_usage(argv[0]);
            exit(-1);
        break;
        default:
        break;
        }
    }
    if ((argc-optind) > 2) {
        print_usage(argv[0]);
        exit(-1);
    }
    if (key_file_path == NULL) {
        fprintf(stderr, "No basic path found\n");
        exit(-1);
    }

    command = argv[optind];
    if (command && argv[optind+1]) {
        uri = argv[optind+1];
    }
    safeUri(&uri);
    if (uri && (strlen(uri) > 0)) {
        r = cfg_parse(uri, &platformConfig);
        if (r == CFGPRS_OK) {
            domain_name = findDomain(platformConfig);
            if (domain_name == NULL) {
                printf("The domain name could not be determined from the configuration\n");
                exit(-1);
            }
        } else {
            printf("Errors are detected in the configuration. Exiting now...\n");
            exit(-1);
        }
    }
    if ((command == NULL) || (strcmp(command, "stop") == 0)) {
        if (domain_name == NULL) {
            domain_name = "The default Domain";
        }
        findSpliceSystemAndRemove(domain_name);
    } else {
        if (strcmp (command, "start") == 0) {
            if (domain_name == NULL) {
                domain_name = "The default Domain";
            }
            if (!spliceSystemRunning(domain_name)) {
                printf("\nStarting up domain \"%s\" .\n", domain_name);
                if (uri == NULL) {
                    uri = os_strdup("");
                }
                // command = os_strdup("spliced.exe");
                // findSpliced(&command); */
                command = os_locate(SPLICED_NAME, OS_ROK|OS_XOK);
                os_procAttrInit(&pa);
                os_procCreate(command, "OpenSplice Control Service", uri, &pa, &pi);
                Sleep(2000); /* take time to first show the license message from spliced */
            } else {
                printf("Splice System with domain name \"%s\" is found running, ignoring command\n",
                    domain_name);
            }
        } else {
            if (strcmp(command, "list") == 0) {
                findSpliceSystemAndShow();
            } else {
                print_usage(argv[0]);
                exit(-1);
            }
        }
    }
    os_free(uri);
    uri = NULL;
    return 0;
}
