/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <os.h>
#include <cfg_parser.h>
#include <cf_config.h>

#include "ospl_proc.h"

#define OS_PERMISSION  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

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
            "      ospl [-f] start [URI]\n"
            "      ospl [[-d <domain> | -a] stop [URI]]\n"
            "      ospl list\n\n"
            "      -h       Show this help\n\n");
    printf ("      start    Start the identified system\n\n"
            "               The system is identified and configured by the URI which is\n"
            "               defined by the environment variable OSPL_URI. This setting can\n"
            "               be overruled with the command line URI definition. When none of\n"
            "               the URI definitions is specified, a default system will be\n"
            "               started.\n");
    printf ("               Upon exit an exit code will be provided to indicated the cause of\n"
            "               termination. The following exit codes are supported:\n"
            "               * 0 : normal termination as result of �OSPL stop�.\n"
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
            "               * 0 : normal termination as result of  �OSPL stop�.\n"
            "               * -1: Not Applicable\n");
    printf ("               * -2: an unrecoverable error occurred.\n"
            "                     The system has encountered an error that cannot be\n"
            "                     resolved by a retry of the same command. E.g., A\n"
            "                     faulty URI was provided.\n\n");
    printf ("      list     Show all systems started by the current user by their domain\n"
            "               name\n\n");
}

static const char key_file_prefix[] = "/tmp/spddskey_XXXXXX";

static void
removeProcesses(
    int pid,
    os_time serviceTerminatePeriod)
{
    os_time stopTime;

#ifndef NDEBUG
    printf("\nWait %d.%d seconds for all processes to terminate\n",
           serviceTerminatePeriod.tv_sec,serviceTerminatePeriod.tv_nsec);
#endif
    stopTime = os_timeAdd(os_timeGet(), serviceTerminatePeriod);

    kill (pid, SIGTERM);
    while ((kill (pid, 0) != -1) && (os_timeCompare(os_timeGet(), stopTime) == OS_LESS) )
    {
        printf ("."); fflush (stdout);
        sleep (1);
    }
    printf("\n");
    if (kill (pid, 0) != -1)
    {
        printf ("Process %d would not terminate.\n", pid);
        printf ("Using force now on ");
        kill_descendents (pid, SIGKILL);
        kill (pid, SIGKILL);
        stopTime = os_timeAdd(os_timeGet(), serviceTerminatePeriod);
        while ((kill (pid, 0) != -1) && (os_timeCompare(os_timeGet(), stopTime) == OS_LESS))
        {
            printf ("."); fflush (stdout);
            sleep (1);
        }
        if (kill (pid, 0) != -1)
        {
            printf ("\nProcess %d would not terminate, bailing out\n", pid);
        }
    }
}

static int
removeSegment(
    key_t key)
{
    int shmid;
    int retCode = OSPL_EXIT_CODE_OK;

    shmid = shmget (key, 0, 0);
    if (shmid != -1)
    {
        int val;

        val = shmctl (shmid, IPC_RMID, NULL);
        if(val >= 0)
        {
            retCode = OSPL_EXIT_CODE_OK;
        } else
        {
            /* unrecoverable */
            retCode = OSPL_EXIT_CODE_UNRECOVERABLE_ERROR;
        }
    }
    return retCode;
}

static int
removeKeyfile(
    const char *key_file_name)
{
    /* try to unlink the key file. This is a fallback option. In normal circumstances
     * the spliced process will have deleted the key file, but something it failed
     * and we have to unlink it here. But in general the unlink call below will
     * fail because the file is already gone. This failure can be ignored.
     */
    unlink (key_file_name);

    return OSPL_EXIT_CODE_OK;
}

static int
shutdownDDS(
    const char *key_file_name,
    const char *domain_name,
    os_time serviceTerminatePeriod)
{
    key_t key;
    char uri[512];
    char map_address[64];
    char size[64];
    char implementation[64];
    char creator_pid[64];
    int pid;
    FILE *kf;
    int retCode = OSPL_EXIT_CODE_OK;

    printf ("\nShutting down domain \"%s\" ", domain_name);
    kf = fopen (key_file_name, "r");
    if (kf)
    {
        fgets (uri, sizeof(uri), kf);
        fgets (map_address, sizeof(map_address), kf);
        fgets (size, sizeof(size), kf);
        fgets (implementation, sizeof(implementation), kf);
        fgets (creator_pid, sizeof(creator_pid), kf);
        fclose (kf);
        sscanf (creator_pid, "%d", &pid);
        if (strcmp (implementation, "SVR4-IPCSHM\n") == 0)
        {
            key = ftok (key_file_name, 'S');
            if (key != -1)
            {
                removeProcesses (pid, serviceTerminatePeriod);
                retCode = removeSegment (key);
                if(retCode == OSPL_EXIT_CODE_OK)
                {
                    retCode = removeKeyfile (key_file_name);
                }
            }
        } else if (strcmp (implementation, "POSIX-SMO\n") == 0)
        {
            printf ("Removal of POSIX shared memory object not yet supported\n");
            /** @todo support POSIX shared memory objects */
            removeProcesses (pid, serviceTerminatePeriod);
            retCode = removeKeyfile (key_file_name);
        }
    } else
    {
        /* unrecoverable, can not read keyfile.*/
        retCode = OSPL_EXIT_CODE_UNRECOVERABLE_ERROR;
    }
    printf ("Ready\n\n");
    return retCode;
}

static char *
matchKey(
    const char *key_file_name,
    const char *domain_name)
{
    FILE *key_file;
    char domain[512];
    struct stat filestat;

    if (stat (key_file_name, &filestat) == 0)
    {
        if (filestat.st_uid == geteuid())
        {
            key_file = fopen (key_file_name, "r");
            if (key_file != NULL)
            {
                if (fgets (domain, sizeof(domain), key_file) != NULL)
                {
                    if ((domain_name == NULL) ||
                        (strcmp (domain_name, "*") == 0) ||
                        (strcmp (domain_name, domain) == 0))
                    {
                        return os_strdup (domain);
                    }
                }
                fclose (key_file);
            }
        }
    }
    return NULL;
}

static char *
matchUid(
    const char *key_file_name,
    uid_t uid)
{
    FILE *key_file;
    char domain[512];
    struct stat filestat;

    if (stat (key_file_name, &filestat) == 0)
    {
        if (filestat.st_uid == uid)
        {
            key_file = fopen (key_file_name, "r");
            if (key_file != NULL)
            {
                if (fgets (domain, sizeof(domain), key_file) != NULL)
                {
                    fclose (key_file);
                    return os_strdup (domain);
                }
                fclose (key_file);
            }
        }
    }
    return NULL;
}

static int
findSpliceSystemAndRemove(
    const char *domain_name,
    os_time serviceTerminatePeriod)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(key_file_prefix)+1];
    char *shmName;
    int retCode = OSPL_EXIT_CODE_OK;

    key_dir = opendir ("/tmp");
    if (key_dir)
    {
        entry = readdir (key_dir);
        while (entry != NULL)
        {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0)
            {
                snprintf (key_file_name, sizeof(key_file_prefix)+1, "/tmp/%s", entry->d_name);
                if ((shmName = matchKey (key_file_name, domain_name)))
                {
                    retCode = shutdownDDS (key_file_name, shmName, serviceTerminatePeriod);
                    os_free (shmName);
                }
            }
            entry = readdir (key_dir);
        }
        closedir (key_dir);
    } else
    {
        /* unrecoverable */
        retCode = OSPL_EXIT_CODE_UNRECOVERABLE_ERROR;
    }
    return retCode;
}

static int
findSpliceSystemAndShow(void)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(key_file_prefix)+1];
    char *shmName;
    int found_count;

    found_count = 0;
    key_dir = opendir ("/tmp");
    if (key_dir) {
        entry = readdir (key_dir);
        while (entry != NULL) {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
                snprintf (key_file_name, sizeof(key_file_prefix)+1, "/tmp/%s", entry->d_name);
                if ((shmName = matchUid (key_file_name, geteuid()))) {
                                    printf ("Splice System with domain name \"%s\" is found running\n", shmName);
                                    ++found_count;
                    os_free (shmName);
                }
            }
            entry = readdir (key_dir);
        }
        closedir (key_dir);
    }

    return found_count;
}

static int
spliceSystemRunning(
    char *domain)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(key_file_prefix)+1];
    char *shmName;
    int found = 0;

    key_dir = opendir ("/tmp");
    if (key_dir) {
        entry = readdir (key_dir);
        while (entry != NULL) {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
                snprintf (key_file_name, sizeof(key_file_prefix)+1, "/tmp/%s", entry->d_name);
                if ((shmName = matchUid (key_file_name, getuid()))) {
            if (strcmp (shmName, domain) == 0) {
            found = 1;
            }
                    os_free (shmName);
                }
            }
            entry = readdir (key_dir);
        }
        closedir (key_dir);
    }
    return found;
}

static char *
findDomain(
    cf_element platformConfig,
    cf_element *domain)
{
    char *domain_name = NULL;
    cf_element dc = NULL;
    cf_element elemName = NULL;
    cf_data dataName;
    c_value value;

    dc = cf_element (cf_elementChild (platformConfig, CFG_DOMAIN));
    if (dc) {
        elemName = cf_element(cf_elementChild(dc, CFG_NAME));
        if (elemName) {
            dataName = cf_data(cf_elementChild(elemName, "#text"));
            if (dataName) {
            value = cf_dataValue(dataName);
            domain_name = value.is.String;
                *domain = dc;
            }
    }
    }
    return domain_name;
}

static void
findServiceTerminatePeriod(
    cf_element domain,
    os_time *serviceTerminatePeriod)
{
    cf_element elem;
    cf_attribute attr;
    cf_data data;
    c_value value;
    float offset, update_factor, expiry_time;

    /* Spliced uses 4 seconds by default or a value defined in the configuration file. */
    /* But the timing in ospl starts earlier. Therefore, increase the period in ospl   */
    /* with an additional 4 seconds such that it is larger than that in spliced. */

    offset = 4.0;
    (*serviceTerminatePeriod).tv_sec = offset + 4;
    (*serviceTerminatePeriod).tv_nsec = 0;

    if (domain != NULL) {
        elem = cf_element(cf_elementChild(domain, CFG_LEASE));
        if (elem) {
            elem = cf_element(cf_elementChild(elem, CFG_EXPIRYTIME));
            if (elem) {
                data = cf_data(cf_elementChild(elem, "#text"));
                if (data) {
                    value = cf_dataValue(data);
                    expiry_time = atof(value.is.String);
                }
                attr = cf_attribute(cf_elementAttribute(elem,"update_factor"));
                if (attr) {
                    value = cf_attributeValue(attr);
                    update_factor = atof(value.is.String);
                    offset += (expiry_time * update_factor);
                }
            }
        }
        elem = cf_element(cf_elementChild(domain, CFG_TERMPERIOD));
        if (elem) {
            data = cf_data(cf_elementChild(elem, "#text"));
            if (data) {
                value = cf_dataValue(data);
                *serviceTerminatePeriod = os_realToTime(offset + atof(value.is.String));
            }
        }
    }
}

#ifdef OS_LINUX_DEFS_H
void
check_for_LD_ASSUME_KERNEL (void)
{
    if (os_getenv ("LD_ASSUME_KERNEL")) {
    fprintf (stderr, "\nWarning: LD_ASSUME_KERNEL is set, this might cause OpenSpliceDDS to fail.\n");
        fprintf (stderr, "         OpenSpliceDDS requires the Native Posix Thread Library to be linked.\n");
        fprintf (stderr, "         You can check by calling: \'ldd `which ospl`', libc.so.* should be linked\n");
        fprintf (stderr, "         with /lib/tls/libc.so.* and not /lib/i686/libc.so.* nor /lib/libc.so.*.\n\n");
    }
}
#endif

int
main(
    int argc,
    char *argv[])
{
    int opt;
    int retCode = OSPL_EXIT_CODE_OK;
    char *domain_name = NULL;
    char *uri = NULL;
    char *command = NULL;
    char start_command[1024];
    cf_element platformConfig = NULL;
    cf_element domain = NULL;
    cfgprs_status r;
    os_time serviceTerminatePeriod;
    os_boolean blocking = OS_FALSE;
    os_boolean blockingDefined = OS_FALSE;

    os_osInit();
    os_procAtExit(os_osExit);

    uri = os_getenv ("OSPL_URI");

    while ((opt = getopt (argc, argv, "hafd:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage (argv[0]);
            exit (OSPL_EXIT_CODE_OK);
            break;
        case 'd':
            if (domain_name)
            {
                print_usage (argv[0]);
                exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            }
            domain_name = optarg;
            break;
        case 'a':
            if (domain_name)
            {
                print_usage (argv[0]);
                exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
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
            print_usage (argv[0]);
            exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
            break;
        default:
            break;
        }
    }
    if ((argc-optind) > 3)
    {
        print_usage (argv[0]);
        exit(OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
    }
    command = argv[optind];
    if (command && argv[optind+1])
    {
        uri = argv[optind+1];
    }
    if (uri && (strlen(uri) > 0))
    {
        r = cfg_parse_ospl (uri, &platformConfig);
        if (r == CFGPRS_OK)
        {
            domain_name = findDomain (platformConfig, &domain);
            if (domain_name == NULL)
            {
                printf ("The domain name could not be determined from the configuration\n");
                exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
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
            exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
        }
    }
    if ((command == NULL) || (strcmp (command, "stop") == 0))
    {
        if (domain_name == NULL)
        {
            domain_name = "The default Domain";
        }
        findServiceTerminatePeriod(domain, &serviceTerminatePeriod);
        retCode = findSpliceSystemAndRemove (domain_name, serviceTerminatePeriod);
    } else if (strcmp (command, "start") == 0)
    {
#ifdef OS_LINUX_DEFS_H
        check_for_LD_ASSUME_KERNEL ();
#endif
        if (domain_name == NULL)
        {
            domain_name = "The default Domain";
        }
        if (!spliceSystemRunning (domain_name))
        {
            if(!blocking)
            {
                printf ("\nStarting up domain \"%s\" .", domain_name);
            } else
            {
                printf ("\nStarting up domain \"%s\" and blocking.", domain_name);
            }
            if (uri)
            {
                if(!blocking)
                {
                    snprintf (start_command, sizeof(start_command), "spliced \"%s\" &", uri);
                } else
                {
                    snprintf (start_command, sizeof(start_command), "spliced \"%s\"", uri);
                }
            } else
            {
                if(!blocking)
                {
                    snprintf (start_command, sizeof(start_command), "spliced &");
                } else
                {
                    snprintf (start_command, sizeof(start_command), "spliced");
                }
            }
            printf (" Ready\n");
            int systemRetCode = system(start_command);
            int *systemRetCode_p = &systemRetCode;
            retCode = WEXITSTATUS(systemRetCode_p);
            if(!blocking)
            {
                sleep (2); /* take time to first show the license message from spliced */
            }
        } else
        {
            printf ("Splice System with domain name \"%s\" is found running, ignoring command\n",
            domain_name);
        }
    } else if (strcmp (command, "list") == 0)
    {
        findSpliceSystemAndShow ();
    } else
    {
        print_usage (argv[0]);
        exit (OSPL_EXIT_CODE_UNRECOVERABLE_ERROR);
    }
    return retCode;
}
