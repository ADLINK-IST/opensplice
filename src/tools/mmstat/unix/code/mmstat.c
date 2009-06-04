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
#include <termios.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#ifdef OS_SOLARIS_DEFS_H
#include <sys/filio.h>
#endif

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

static void
print_usage (
    char *name
    )
{
    printf ("\nUsage:\n"
	    "      mmstat -h\n"
	    "      mmstat [-e] [-i interval] [-s sample_count] [URI]\n"
            "      mmstat -t [-i interval] [-s sample_count] [-l limit] [-f filter_expression] [URI]\n\n");
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
    printf ("      Use 'q' to terminate the monitor\n\n"
	    "      Use 't' to immediately show statistics\n\n");
}

int
main (
    int argc,
    char *argv[]
    )
{
    int opt;
    int interval = 3000;
    int sampleCount = 0;
    c_bool extended = FALSE;
    c_bool raw = FALSE;
    c_bool delta = FALSE;
    char *uri = "";
    u_result ur;
    u_participant participant;
    v_participantQos pqos;
    struct termios old_termios;
    struct termios new_termios;
    sigset_t sigmask;
    int count;
    int no_break = TRUE;
    char c;
    int lost;
    char* sddsURI;
    monitorMode selectedAction = memoryStats;
    monitor_ms msData = NULL;
    monitor_trc trcData = NULL;
    monitor_orc orcData = NULL;
    int delay = 0;
    int trigger = 0;
    int sample = 0;
    c_long objectCountLimit = 0;
    char *filterExpression = NULL;

    while ((opt = getopt (argc, argv, "i:l:f:s:hertToOmM")) != -1) 
    {
       switch (opt) 
       {
          case 'i':
             sscanf (optarg, "%d", &interval);
             break;
          case 's':
             sscanf (optarg, "%d", &sampleCount);
             break;
          case 'l':
             sscanf (optarg, "%d", &objectCountLimit);
             break;
          case 'f':
             filterExpression = optarg;
             break;
          case 'e':
             extended = TRUE;
             break;
          case 'r':
             raw = TRUE;
             break;
          case 'h':
             print_usage (argv[0]);
             exit (0);
             break;
          case 'm':
             selectedAction = memoryStats;
             break;
          case 'M':
             selectedAction = memoryStats;
             delta = TRUE;
             break;
          case 't':
             selectedAction = typeRefCount;
             break;
          case 'T':
             selectedAction = typeRefCount;
             delta = TRUE;
             break;
          case 'o':
             selectedAction = objectRefCount;
             break;
          case 'O':
             selectedAction = objectRefCount;
             delta = TRUE;
             break;
          case '?':
             print_usage (argv[0]);
             exit (-1);
             break;
       }
    }
    if ((argc - optind) > 1) 
    {
       print_usage (argv[0]);
       exit (-1);
    }
    if ((argc - optind) == 1) 
    {
       uri = argv[optind];
    }
    
    if( !raw) 
    {
       if(strlen(uri) > 0) 
       {
          sddsURI = os_strdup(uri);
       } 
       else 
       {
          sddsURI = os_getenv ("OSPL_URI");
            
          if(!sddsURI)
          {
             sddsURI = (c_char*)os_malloc(19);
             sprintf(sddsURI, "%s", "The default Domain");
          } 
          else 
          {
             sddsURI = os_strdup(sddsURI);
          }
       }
       printf("Trying to open connection with the OpenSplice system using URI:\n" \
              "'%s'...\n", sddsURI);
       os_free(sddsURI);
    }
    
    ur = u_userInitialise();
    
    if(ur == U_RESULT_OK) 
    {
       pqos = u_participantQosNew(NULL);
       participant = u_participantNew(uri, 30, "mmstat", (v_qos)pqos, TRUE);
       u_participantQosFree(pqos);
        
       if(participant) 
       {
          if( !raw ) 
          {
             printf("Connection established.\n\n");
             if (isatty (fileno(stdin))) 
             {
                sigemptyset (&sigmask);     /* empty signal mask */
                sigaddset (&sigmask, SIGTTOU);  /* add SIGTTOU signal */
                sigaddset (&sigmask, SIGTTIN);  /* add SIGTTIN signal */
                tcgetattr (fileno(stdin), &old_termios);
                tcgetattr (fileno(stdin), &new_termios);
                new_termios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
                new_termios.c_oflag &= ~OPOST;
                new_termios.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN|TOSTOP);
                new_termios.c_cflag &= ~(CSIZE|PARENB);
                new_termios.c_cflag |= CS8;
                new_termios.c_cc[VTIME] = 0;
                new_termios.c_cc[VMIN] = 0;
                sigprocmask (SIG_BLOCK, &sigmask, NULL);    /* Igore input/output signals */
                tcsetattr (fileno(stdin), TCSAFLUSH, &new_termios);
             }
          }
          lost = 0;
          switch (selectedAction) 
          {
             case memoryStats:
                msData = monitor_msNew (extended, raw, delta);
                break;
             case typeRefCount:
                trcData = monitor_trcNew (objectCountLimit, filterExpression, delta);
                break;
             case objectRefCount:
                orcData = monitor_orcNew (objectCountLimit, filterExpression, delta);
                break;
          }
            
          while (no_break && !lost) 
          {
             if (delay <= 0 || trigger) 
             {
                switch (selectedAction) 
                {
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
                if (trigger) 
                {
                   trigger = 0;
                } 
                else 
                {
                   delay = interval;
                }
             }
                
             if(ur == U_RESULT_OK)
             {
                if (isatty (fileno(stdin)) && !raw) 
                {
                   count = read (fileno(stdin), &c, 1);
                   /* if count = -1, mmstat is started in background */
                   /* if count = 0, mmstat is started in foreground, */
                   /* but there is no input */
                   while (count > 0) 
                   {
                      if (c == 'q' || c == '\03' /* ^C */) 
                      {
                         no_break = FALSE;
                      } 
                      else if (c == 't') 
                      {
                         trigger = 1;
                      }
                      count = read (fileno(stdin), &c, 1);
                   }
                } 
                else 
                {
                   if (ioctl (fileno(stdin), FIONREAD, &count) == 0) 
                   {
                      count = read (fileno(stdin), &c, 1);
                      if (count) 
                      {
                         if (c == 'q' || c == '\03' /* ^C */) 
                         {
                            no_break = FALSE;
                         } 
                         else if (c == 't') 
                         {
                            trigger = 1;
                         }
                      }
                   } 
                   else 
                   {
                      no_break = 0;
                   }
                }
                if (no_break && interval) 
                {
                   delay -= 100;
                   usleep (100 * 1000);
                }
             } 
             else 
             {
                /* Participant is no longer accessible, terminate now... */
                no_break = 0;
                lost = TRUE;
             }
             if (sampleCount && (sample == sampleCount)) 
             {
                printf ("\nsample_count limit reached\n");
                no_break = 0;
             }
          }
          if (isatty (fileno(stdin)) && !raw) 
          {
             count = read (fileno(stdin), &c, 1);
                
             if(count != -1)
             {
                tcsetattr (fileno(stdin), TCSAFLUSH, &old_termios);
             }
          }
          u_participantFree(participant);
            
          if(lost) 
          {
             printf("\nConnection with domain lost. The OpenSplice system has\n" \
                    "probably been shut down.\n");
          }
       } 
       else 
       {
          printf("Connection could NOT be established (creation of participant failed).\n");
          printf("Is the OpenSplice system running?\n");
          OS_REPORT(OS_ERROR,"mmstat", 0, "Creation of participant failed.");
       }
       u_userDetach();
       switch (selectedAction) 
       {
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
    } 
    else 
    {
       printf("Connection could NOT be established (could not initialise).\n");
       printf("Is the OpenSplice system running?\n");
       OS_REPORT(OS_ERROR,"mmstat", 0, "Failed to initialise.");
    }
    printf("\nExiting now...\n");

    return 0;
}

