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

#include <os.h>
#include <cfg_parser.h>
#include <cf_config.h>

#include "ospl_proc.h"

#define OS_PERMISSION  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

static void
print_uage(
    char *name)
{
    printf ("\nUsage:\n"
            "      ospl -h\n"
            "      ospl start [URI]\n"
            "      ospl [[-d <domain> | -a] stop [URI]]\n"
            "      ospl list\n\n"
            "      -h       Show this help\n\n");
    printf ("      start    Start the identified system\n\n"
            "               The system is identified and configured by the URI which is\n"
            "               defined by the environment variable OSPL_URI. This setting can\n"
            "               be overruled with the command line URI definition. When none of\n"
            "               the URI definitions is specified, a default system will be\n"
            "               started.\n\n");
    printf ("      stop     Stop the identified system\n\n"
            "               Stop is the default command, thus when no command is specified\n" 
            "               stop is assumed. The system to stop is identified by the URI\n"
            "               which is defined by the environment variable OSPL_URI. This\n");
    printf ("               setting can be overruled by the command line URI definition or\n"
            "               the domain name which is associated with the URI and specified\n"
            "               via the -d option. \n"
            "               When no domain is specified by the URI or by it's name a \n"
            "               default system is assumed. The -a options specifies to stop all\n"
            "               running splice systems started by the current user.\n\n");
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
    while ((kill (pid, 0) != -1) && (os_timeCompare(os_timeGet(), stopTime) == OS_LESS) ) {
        printf ("."); fflush (stdout);
	sleep (1);
    }
    printf("\n");
    if (kill (pid, 0) != -1) {
	printf ("Process %d would not terminate.\n", pid);
	printf ("Using force now on ");
	kill_descendents (pid, SIGKILL);
	kill (pid, SIGKILL);
        stopTime = os_timeAdd(os_timeGet(), serviceTerminatePeriod);
        while ((kill (pid, 0) != -1) && (os_timeCompare(os_timeGet(), stopTime) == OS_LESS)) {
            printf ("."); fflush (stdout);
	    sleep (1);
        }
	if (kill (pid, 0) != -1) {
	    printf ("\nProcess %d would not terminate, bailing out\n", pid);
	}
    }
}

static void
removeSegment(
    key_t key)
{
    int shmid;

    shmid = shmget (key, 0, 0);
    if (shmid != -1) {
        shmctl (shmid, IPC_RMID, NULL);
    }
}

static void
removeKeyfile(
    const char *key_file_name)
{
    unlink (key_file_name);
}

static void
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

    printf ("\nShutting down domain \"%s\" ", domain_name);
    kf = fopen (key_file_name, "r");
    if (kf) {
        fgets (uri, sizeof(uri), kf);
        fgets (map_address, sizeof(map_address), kf);
        fgets (size, sizeof(size), kf);
        fgets (implementation, sizeof(implementation), kf);
        fgets (creator_pid, sizeof(creator_pid), kf);
	fclose (kf);
        sscanf (creator_pid, "%d", &pid);
	if (strcmp (implementation, "SVR4-IPCSHM\n") == 0) {
            key = ftok (key_file_name, 'S');
	    if (key != -1) {
	        removeProcesses (pid, serviceTerminatePeriod);
	        removeSegment (key);
	        removeKeyfile (key_file_name);
	    }
	} else if (strcmp (implementation, "POSIX-SMO\n") == 0) {
	    printf ("Removal of POSIX shared memory object not yet supported\n");
	    /** @todo support POSIX shared memory objects */
	    removeProcesses (pid, serviceTerminatePeriod);
	    removeKeyfile (key_file_name);
	}
    }
    printf ("Ready\n\n");
}

static char *
matchKey(
    const char *key_file_name,
    const char *domain_name)
{
    FILE *key_file;
    char domain[512];
    struct stat filestat;

    if (stat (key_file_name, &filestat) == 0) {
	if (filestat.st_uid == geteuid()) {
            key_file = fopen (key_file_name, "r");

            if (key_file != NULL) {
                if (fgets (domain, sizeof(domain), key_file) != NULL) {
                    if ((domain_name == NULL) ||
                        (strcmp (domain_name, "*") == 0) ||
                        (strcmp (domain_name, domain) == 0)) {
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

    if (stat (key_file_name, &filestat) == 0) {
	if (filestat.st_uid == uid) {
            key_file = fopen (key_file_name, "r");
            if (key_file != NULL) {
                if (fgets (domain, sizeof(domain), key_file) != NULL) {
                    fclose (key_file);
		    return os_strdup (domain);
                }
                fclose (key_file);
	    }
	}
    }
    return NULL;
}

static void
findSpliceSystemAndRemove(
    const char *domain_name,
    os_time serviceTerminatePeriod)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(key_file_prefix)+1];
    char *shmName;

    key_dir = opendir ("/tmp");

    if (key_dir) {
        entry = readdir (key_dir);

        while (entry != NULL) {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
                snprintf (key_file_name, sizeof(key_file_prefix)+1, "/tmp/%s", entry->d_name);

                if ((shmName = matchKey (key_file_name, domain_name))) {
                    shutdownDDS (key_file_name, shmName, serviceTerminatePeriod);
                    os_free (shmName);
                }
            }
            entry = readdir (key_dir);
        }
        closedir (key_dir);
    }
}

static void
findSpliceSystemAndShow(void)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(key_file_prefix)+1];
    char *shmName;

    key_dir = opendir ("/tmp");
    if (key_dir) {
        entry = readdir (key_dir);
        while (entry != NULL) {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
                snprintf (key_file_name, sizeof(key_file_prefix)+1, "/tmp/%s", entry->d_name);
                if ((shmName = matchUid (key_file_name, geteuid()))) {
		    printf ("Splice System with domain name \"%s\" is found running\n", shmName);
                    os_free (shmName);
                }
            }
            entry = readdir (key_dir);
        }
        closedir (key_dir);
    }
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
	fprintf (stderr, "\nWarning: LD_ASSUME_KERNEL is set, this might cause SPLICE-DDS to fail.\n");
        fprintf (stderr, "         SPLICE-DDS requires the Native Posix Thread Library to be linked.\n");
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
    char *domain_name = NULL;
    char *uri = NULL;
    char *command = NULL;
    char start_command[1024];
    cf_element platformConfig = NULL;
    cf_element domain = NULL;
    cfgprs_status r;
    os_time serviceTerminatePeriod;

    os_osInit();
    uri = os_getenv ("OSPL_URI");

    while ((opt = getopt (argc, argv, "had:")) != -1) {
	switch (opt) {
	case 'h':
	    print_uage (argv[0]);
	    exit (0);
	    break;
	case 'd':
	    if (domain_name) {
	        print_uage (argv[0]);
	        exit (-1);
	    }
	    domain_name = optarg;
	    break;
	case 'a':
	    if (domain_name) {
	        print_uage (argv[0]);
	        exit (-1);
	    }
        uri = NULL;
	    domain_name = "*";
	    break;
	case '?':
	    print_uage (argv[0]);
	    exit (-1);
	    break;
	default:
	    break;
	}
    }
    if ((argc-optind) > 2) {
	print_uage (argv[0]);
        exit(-1);
    }
    command = argv[optind];
    if (command && argv[optind+1]) {
        uri = argv[optind+1];
    }
    if (uri && (strlen(uri) > 0)) {
        r = cfg_parse (uri, &platformConfig);
        if (r == CFGPRS_OK) {
            domain_name = findDomain (platformConfig, &domain);
            if (domain_name == NULL) {
                printf ("The domain name could not be determined from the configuration\n");
                exit (-1);
            }
        } else {
            printf ("Errors are detected in the configuration. Exiting now...\n");
            exit (-1);
        }
    }
    if ((command == NULL) || (strcmp (command, "stop") == 0)) {
	if (domain_name == NULL) {
	    domain_name = "The default Domain";
	}
        findServiceTerminatePeriod(domain, &serviceTerminatePeriod);
        findSpliceSystemAndRemove (domain_name, serviceTerminatePeriod);
    } else if (strcmp (command, "start") == 0) {
#ifdef OS_LINUX_DEFS_H
	check_for_LD_ASSUME_KERNEL ();
#endif
	if (domain_name == NULL) {
	    domain_name = "The default Domain";
	}
	if (!spliceSystemRunning (domain_name)) {
            printf ("\nStarting up domain \"%s\" .", domain_name);
	    if (uri) {
	        snprintf (start_command, sizeof(start_command), "spliced \"%s\" &", uri);
	    } else {
	        snprintf (start_command, sizeof(start_command), "spliced &");
	    }
	    printf (" Ready\n");
	    system (start_command);
	    sleep (2); /* take time to first show the license message from spliced */
	} else {
	    printf ("Splice System with domain name \"%s\" is found running, ignoring command\n",
		domain_name);
	}
    } else if (strcmp (command, "list") == 0) {
	findSpliceSystemAndShow ();
    } else {
	print_uage (argv[0]);
	exit (-1);
    }
    return 0;
}
