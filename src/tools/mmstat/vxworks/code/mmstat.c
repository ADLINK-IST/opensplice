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
#include <os.h>

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ioLib.h>
#include <taskLib.h>
#include <sysLib.h>

#include "c__base.h"
#include <u_user.h>
#include <u__user.h>
#include <os_report.h>

#include "mm_orc.h"
#include "mm_trc.h"
#include "mm_ms.h"

#define DATABASE_NAME "c_base_a02"
#define SHM_NAME "The default Domain"

typedef enum {
    memoryStats,
    typeRefCount,
    objectRefCount
} monitorMode;

typedef struct {
    c_long magic;		/* magic definition */
    c_bool stop;		/* When TRUE, stop background task */
    monitorMode selectedAction;
    c_long interval;
    c_long sampleCount;
    c_long objectCountLimit;
    c_char *filterExpression;
    c_bool extended;
    c_bool raw;
    c_bool delta;
    c_char *uri;
    c_char *outputFile;		/* output file, running in background when set */
    c_long priority;		/* priority for background task */
} monitorControl;

static void
print_usage (
    char *name
    )
{
    printf ("\nUsage:\n"
	    "      mmstat \"-h\"\n"
	    "      mmstat \"[-e] [-i interval] [-s sample_count] [-b output_file [-p priority]] [URI]\"\n"
            "      mmstat \"-t [-i interval] [-s sample_count] [-l limit] [-f filter_expression] [-b output_file [-p priority]] [URI]\"\n\n");
    printf ("      -h       Show this help\n\n");
    printf ("               Show the memory statistics of the system identified by\n"
	    "               the specified URI. If no URI is specified, the environment\n"
            "               variable OSPL_URI will be searched for. When even the\n"
	    "               the environment variable is unspecified, the default system\n"
	    "               will be selected. The default display interval is 3 seconds\n\n");
    printf ("      -e       Extended mode, shows bar for allocated memory\n\n"
	    "      -i interval\n"
	    "               Show memory statistics every interval milli seconds\n\n"
            "      -s sample_count\n"
            "               Stop after sample_count samples\n\n");
    printf ("      -t       Show meta object reference count of the system identified by\n"
	    "               the specified URI. If no URI is specified, the environment\n"
            "               variable OSPL_URI will be searched for. When even the\n"
	    "               the environment variable is unspecified, the default system\n"
	    "               will be selected. The default display interval is 3 seconds\n\n"
	    "      -l limit\n"
            "               Show only extent count >= limit\n\n"
            "      -f filter_expression\n"
            "               Show only meta objects which name passes the filter expression\n\n");
    printf ("      -b       Backgound processing\n"
            "               Runs the mmstat command in background, logging the result in output_file\n\n");
    printf ("      -p       Backgound processing priority\n"
            "               Priority of the background task (0-255), default 254\n\n");
    printf ("      Use 'q' to terminate the monitor\n\n"
	    "      Use 't' to immediately show statistics\n\n");
    printf ("Returns:\n"
            "      0        mmstat is correctly executed\n"
            "      -1       mmstat has detected an error\n"
            "      other    command is exected in background, the task can be stopped calling\n"
            "               mmstatStop with the return value of mmstat as parameter\n\n");
}

static int
mmstat_function (
    monitorControl *mc
    )
{
    u_result ur;
    u_participant participant;
    v_participantQos pqos;
    int old_termios = 0;
    int count;
    int no_break = TRUE;
    char c;
    int lost;
    char* sddsURI;
    monitor_ms msData = NULL;
    monitor_trc trcData = NULL;
    monitor_orc orcData = NULL;
    int delay = 0;
    int trigger = 0;
    int sample = 0;
    int outFd = 0;

    if (mc->outputFile) {
	outFd = open (mc->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (outFd == -1) {
	    printf ("Can not open file %s for writing\n", mc->outputFile);
	    mc->stop = TRUE;
	    mc->magic = 0;
	} else {
	    ioTaskStdSet (0, 1, outFd);
	}
    }
    if ((mc->outputFile == NULL) || (mc->stop == FALSE)) {
        if (!mc->raw) {
            if(strlen(mc->uri) > 0) {
                sddsURI = os_strdup(mc->uri);
            } else {
                sddsURI = os_getenv ("OSPL_URI");
            
                if(!sddsURI){
                    sddsURI = (c_char*)os_malloc(19);
                    sprintf(sddsURI, "%s", "The default Domain");
                } else {
                    sddsURI = os_strdup(sddsURI);
                }
            }
            printf("Trying to open connection with the OpenSplice system using URI:\n" \
               "'%s'...\n", sddsURI);
            os_free(sddsURI);
        }
    
        ur = u_userInitialise();
    
        if (ur == U_RESULT_OK) {
            pqos = u_participantQosNew(NULL);
            participant = u_participantNew(mc->uri, 30, "mmstat", (v_qos)pqos, TRUE);
            u_participantQosFree(pqos);
        
            if (participant) {
                if (!mc->raw) {
                    printf("Connection established.\n\n");
                    if (isatty (fileno(stdin))) {
                        old_termios = ioctl (fileno(stdin), FIOGETOPTIONS, 0);
                        ioctl (fileno(stdin), FIOSETOPTIONS, OPT_RAW | OPT_MON_TRAP | OPT_CRMOD);
                    }
                }
                lost = 0;
	        switch (mc->selectedAction) {
		    case memoryStats:
		        msData = monitor_msNew (mc->extended, mc->raw, mc->delta);
		        break;
		    case typeRefCount:
		        trcData = monitor_trcNew (mc->objectCountLimit, mc->filterExpression, mc->delta);
		        break;
		    case objectRefCount:
		        orcData = monitor_orcNew (mc->objectCountLimit, mc->filterExpression, mc->delta);
		        break;
	        }
            
                while (no_break && !lost) {
		    if (delay <= 0 || trigger) {
		        switch (mc->selectedAction) {
                        case memoryStats:
			    ur = u_entityAction(u_entity(participant), monitor_msAction, msData);
			    break;
		        case typeRefCount:
			    ur = u_entityAction(u_entity(participant), monitor_trcAction, trcData);
			    break;
		        case objectRefCount:
			    ur = u_entityAction(u_entity(participant), monitor_orcAction, orcData);
			    break;
		        }
		        sample++;
		        if (trigger) {
			    trigger = 0;
		        } else {
			    delay = mc->interval;
		        }
		    }
                
                    if (ur == U_RESULT_OK) {
                        if (isatty (fileno(stdin)) && !mc->raw) {
                            ioctl (fileno(stdin), FIONREAD, (int)&count);
			    if (count > 0) {
			        count = read (fileno(stdin), &c, 1);
			    }
                            /* if count = -1, mmstat is started in background */
                            /* if count = 0, mmstat is started in foreground, */
                            /* but there is no input */
                            while (count > 0) {
                                if (c == 'q' || c == '\03' /* ^C */) {
                                    no_break = FALSE;
                                } else if (c == 't') {
                                    trigger = 1;
                                }
                                ioctl (fileno(stdin), FIONREAD, (int)&count);
			        if (count > 0) {
                                    count = read (fileno(stdin), &c, 1);
                                }
                            }
                        }
			if (mc->stop) {
			    printf ("\nterminate command recieved\n");
			    no_break = FALSE;
                            mc->magic = 0;
		        }
                        if (no_break && mc->interval) {
                            delay -= 100;
                            taskDelay (sysClkRateGet()/10); /* 10 Hz interval */
                        }
                    } else {
                        /* Participant is no longer accessible, terminate now... */
                        no_break = FALSE;
                        lost = TRUE;
                    }
		    if (mc->sampleCount && (sample == mc->sampleCount)) {
		        printf("\nsample_count limit is reached\n");
                        no_break = FALSE;
                    }
                }
                if (isatty (fileno(stdin)) && !mc->raw) {
                    ioctl (fileno(stdin), FIONREAD, (int)&count);
                    if (count > 0) {
		        count = read (fileno(stdin), &c, 1);
		    }
                
                    if(count != -1){
                        ioctl (fileno(stdin), FIOSETOPTIONS, old_termios);
                    }
                }
                u_participantFree(participant);
            
                if(lost) {
                    printf("\nConnection with domain lost. The OpenSplice system has\n" \
                       "probably been shut down.\n");
                }
            } else {
                printf("Connection could NOT be established (creation of participant failed).\n");
                printf("Is the OpenSplice system running?\n");
                OS_REPORT(OS_ERROR,"mmstat", 0, "Creation of participant failed.");
            }
            u_userDetach();
	    switch (mc->selectedAction) {
	    case memoryStats:
	        monitor_msFree (msData);
	        break;
	    case typeRefCount:
	        monitor_trcFree (trcData);
	        break;
	    case objectRefCount:
	        monitor_orcFree (orcData);
	        break;
	    }
        } else {
            printf("Connection could NOT be established (could not initialise).\n");
            printf("Is the OpenSplice system running?\n");
            OS_REPORT(OS_ERROR,"mmstat", 0, "Failed to initialise.");
        }
        printf("\nExiting now...\n");
    }
    free (mc->uri);
    if (mc->filterExpression) {
        free (mc->filterExpression);
    }
    if (mc->outputFile) {
	free (mc->outputFile);
    }
    if (outFd > 0) {
	fflush (stdout);
	close (outFd);
    }
    free (mc);
    return 0;
}

int
mmstat_command (
    int argc,
    char *argv[]
    )
{
    monitorControl *mc;

    int arg_index;
    int arg_len;
    char *opt_arg = NULL;
    c_bool skip_arg;
    c_bool option_val;
    c_long returnVal = 0;

    mc = malloc (sizeof(monitorControl));

    mc->interval = 3000;
    mc->objectCountLimit = 0;
    mc->sampleCount = 0;
    mc->extended = FALSE;
    mc->raw = FALSE;
    mc->delta = FALSE;
    mc->uri = NULL;
    mc->filterExpression = NULL;
    mc->outputFile = NULL;
    mc->selectedAction = memoryStats;
    mc->stop = FALSE;
    mc->magic = (c_long)0xfedcba98;
    mc->priority = -1;

    arg_index = 0;
    while (argv[arg_index]) {
	skip_arg = FALSE;
	option_val = FALSE;
	opt_arg = NULL;
	if (argv[arg_index][0] == '-') {
	    arg_len = strlen (argv[arg_index]);
	    if (arg_len > 2) {
		opt_arg = &argv[arg_index][2];
	    } else {
		opt_arg = argv[arg_index+1];
		skip_arg = TRUE;
	    }
	    switch (argv[arg_index][1]) {
	    case 'i':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
	        sscanf (opt_arg, "%d", &mc->interval);
		option_val = TRUE;
	        break;
	    case 's':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
	        sscanf (opt_arg, "%d", &mc->sampleCount);
		option_val = TRUE;
	        break;
	    case 'l':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
	        sscanf (opt_arg, "%d", &mc->objectCountLimit);
		option_val = TRUE;
	        break;
	    case 'f':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
		mc->filterExpression = malloc (strlen(opt_arg)+1);
		strcpy (mc->filterExpression, opt_arg);
		option_val = TRUE;
	        break;
	    case 'b':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
		mc->outputFile = malloc (strlen(opt_arg)+1);
		strcpy (mc->outputFile, opt_arg);
		option_val = TRUE;
	        break;
	    case 'p':
                if (opt_arg == NULL) {
	            print_usage ("mmstat");
	            return (-1);
		}
		sscanf (opt_arg, "%d", &mc->priority);
		if ((mc->priority < 0 || mc->priority > 255)) {
		    print_usage ("mmstat");
		    return (-1);
		}
		option_val = TRUE;
	        break;
	    case 'e':
	        mc->extended = TRUE;
	        break;
	    case 'r':
	        mc->raw = TRUE;
	        break;
	    case 'h':
	        print_usage ("mmstat");
	        return (0);
	        break;
	    case 'm':
	        mc->selectedAction = memoryStats;
	        break;
	    case 'M':
	        mc->selectedAction = memoryStats;
	        mc->delta = TRUE;
	        break;
	    case 't':
	        mc->selectedAction = typeRefCount;
	        break;
	    case 'T':
	        mc->selectedAction = typeRefCount;
	        mc->delta = TRUE;
	        break;
	    case 'o':
	        mc->selectedAction = objectRefCount;
	        break;
	    case 'O':
	        mc->selectedAction = objectRefCount;
	        mc->delta = TRUE;
	        break;
	    default:
	        print_usage ("mmstat");
	        return (-1);
	        break;
	    }
	} else {
	    if (mc->uri == NULL) {
		mc->uri = malloc (strlen(argv[arg_index])+1);
		strcpy (mc->uri, argv[arg_index]);
	    } else {
	        print_usage ("mmstat");
	        return (-1);
	    }
	}
	if (option_val && skip_arg) {
	    arg_index++;
	}
	arg_index++;
    }
    if (mc->uri == NULL) {
        mc->uri = malloc(1);
	strcpy (mc->uri, "");
    }
    if (mc->outputFile) {
	mc->raw = TRUE;
	if (mc->priority == -1) {
	    mc->priority = 254;
	}
	taskSpawn ("mmstat", mc->priority, VX_FP_TASK, 40 * 1024, mmstat_function, (int)mc, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	returnVal = (c_long)mc;
    } else {
	if (mc->priority != -1) {
	    print_usage ("mmstat");
	    return (-1);
	}
	returnVal = mmstat_function (mc);
    }
    return returnVal;
}

int
mmstatStop (
    monitorControl *mc
)
{
    if (mc->magic != (c_long)0xfedcba98) {
	printf ("No active session passed\n");
        return -1;
    } else {
	mc->stop = TRUE;
    }
    return 0;
}

int
mmstat (
    char *arg
    )
{
    char *arguments;
    char *argv[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    unsigned int argc  = 0;
    c_bool go_on = TRUE;
    int result;
    unsigned int i = 0;

    if (arg) {
	arguments = malloc (strlen(arg) + 1);
	strcpy (arguments, arg);
    } else {
	arguments = malloc (1);
	strcpy (arguments, "");
    }
    while (go_on == TRUE && (argc < (sizeof(argv)/sizeof(char *)))) {
        while ((arguments[i] == ' ') || (arguments[i] == '\t')) {
            i++;
        }
        if (arguments[i] == '\0') {
            break;
        }
        argv[argc] = &arguments[i];
        argc ++;
        while ((arguments[i] != ' ') && (arguments[i] != '\t')) {
            if (arguments[i] == '\0') {
                go_on = FALSE;
                break;
            }
            i++;
        }
        arguments[i] = '\0';
        i++;
    }
    result = (int)mmstat_command (argc, argv);
    free (arguments);
    return (result);
}    
