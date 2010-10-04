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

#ifdef INTEGRITY
#include <netinet/in.h>
static int conn;
static int port=2323;
static int orig_stdout;
static const char *optflags="p:i:l:f:s:hetTmMa";
#else
static const char *optflags="i:l:f:s:hetTmMa";
#endif

typedef enum {
    memoryStats,
    typeRefCount,
    objectRefCount
} monitorMode;

static void
print_usage ()
{
    printf ("Usage:\n"
	    "      mmstat -h\n"
	    "      mmstat [-M|m] [-e] [-a] [-i interval] [-s sample_count] [URI]\n"
        "      mmstat [-t|T] [-i interval] [-s sample_count] [-l limit] [-f filter_expression] [URI]\n");
    printf ("\n");
    printf ("Show the memory statistics of the OpenSplice system identified by the specified URI. "
            "If no URI is specified, the environment variable OSPL_URI will be used. "
            "If the environment variable is not set either, the default domain will be selected. "
	    "The default display interval is 3 seconds.\n");
    printf ("\n");
    printf ("Mode:\n");
    printf ("      -m               Show memory statistics (default mode)\n");
    printf ("      -M               Show memory statistics difference\n");
    printf ("      -t               Show meta object references\n");
    printf ("      -T               Show meta object references difference\n");
    printf ("\n");
    printf ("Options:\n");
    printf ("      -h               Show this help\n");
#ifdef INTEGRITY
    printf ("      -p port          Set the portnumber of the telnet port (default %d)\n", port);
#endif
    printf ("      -e               Extended mode, shows bar for allocated memory\n");
    printf ("      -a               Show pre-allocated memory as well.\n");
    printf ("      -i interval      Display interval (in milliseconds)\n");
    printf ("      -s sample_count  Stop after sample_count samples\n");
    printf ("      -l limit         Show only object count >= limit\n");
    printf ("      -f filter_expr   Show only meta objects which name passes the filter expression\n");
    printf ("\n");
    printf ("Use 'q' to terminate the monitor\n"
	    "Use 't' to immediately show statistics\n");
}

int
#ifdef INTEGRITY
mmstat_main (
#else
main (
#endif
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
    c_bool preallocated = FALSE;
    char *uri = "";
    u_result ur;
    u_participant participant;
    v_participantQos pqos;
#ifndef INTEGRITY
    struct termios old_termios;
    struct termios new_termios;
    sigset_t sigmask;
#endif
    int count;
    int no_break = TRUE;
    char c;
    int lost;
    const char* sddsURI = NULL;
    monitorMode selectedAction = memoryStats;
    monitor_ms msData = NULL;
    monitor_trc trcData = NULL;
    monitor_orc orcData = NULL;
    int delay = 0;
    int trigger = 0;
    int sample = 0;
    c_long objectCountLimit = 0;
    char *filterExpression = NULL;

    while (((opt = getopt (argc, argv, optflags)) != -1) && !errno) {
        switch (opt) {
            case 'i':
                if (!(sscanf (optarg, "%d", &interval)) > 0) {
                    fprintf(stderr, "mmstat: Not a valid interval.\n");
                    print_usage();
                    exit(-1);
                }
                break;
            case 's':
                if (!(sscanf (optarg, "%d", &sampleCount)) > 0) {
                    fprintf(stderr, "mmstat: Not a valid sample count.\n");
                    print_usage();
                    exit(-1);
                }
                break;
            case 'l':
                if (!(sscanf (optarg, "%d", &objectCountLimit)) > 0) {
                    fprintf(stderr, "mmstat: Not a valid limit.\n");
                    print_usage();
                    exit(-1);
                }
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
            case 'a':
                preallocated = TRUE;
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
#ifdef INTEGRITY
            case 'p':
                if (!(sscanf (optarg, "%d", &port)) > 0) {
                    fprintf(stderr, "mmstat: Not a valid port number.\n");
                    print_usage();
                    exit(-1);
                }
                break;
#endif
            case 'h':
            case '?':
            default:
               print_usage ();
               exit (0);
               break;
        }
    }

    if (errno) {
        fprintf(stderr, strerror(errno));
        fprintf(stderr, "\n");
        print_usage();
        exit (-1);
    }
    if ((argc - optind) > 1) {
        fprintf(stderr, "Too many arguments");
        print_usage ();
        exit (-1);
    }
    if (selectedAction == memoryStats) {
        if (objectCountLimit > 0) {
            fprintf(stderr, "Can't use object limit in memory stats mode.\n");
            print_usage();
            exit(-1);
        }
        if (filterExpression != NULL) {
            fprintf(stderr, "Can't use filter expression in memory stats mode.\n");
            print_usage();
            exit(-1);
        }
    } else {
        if (extended) {
            fprintf(stderr, "Extended mode can only be used in memory stats mode.\n");
            print_usage();
            exit(-1);
        }
        if (preallocated) {
            fprintf(stderr, "Preallocated memory can only be shown in memory stats mode.\n");
            print_usage();
            exit(-1);
        }
    }

    if ((argc - optind) == 1) {
       uri = argv[optind];
    }

    if( !raw) {
       if(strlen(uri) > 0) {
          sddsURI = uri;
       } else {
          sddsURI = os_getenv("OSPL_URI");
          if(!sddsURI) {
              sddsURI = DOMAIN_NAME;
          }
       }

       printf("Trying to open connection with the OpenSplice system using URI:\n" \
              "'%s'...\n", sddsURI);
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
#ifdef INTEGRITY
             {
                int flag=1;
                int res;
                int alen;
                int sock;
                struct sockaddr_in sin;
                printf("Please connect with telnet to port %d.\n\n", port);
                memset( &sin, 0 , sizeof(struct sockaddr_in));
                alen=sizeof(struct sockaddr_in);
                sock = socket(PF_INET, SOCK_STREAM, 0);
                assert(sock != -1 );
                sin.sin_family=AF_INET;
                sin.sin_addr.s_addr = INADDR_ANY;
                sin.sin_port=htons(port);
                res=bind(sock, (struct sockaddr *)&sin, alen);
                assert(res != -1 );
                res=listen(sock, 1);
                assert(res != -1 );
                conn = accept(sock, NULL, NULL);
                assert(conn != -1 );
                close(sock);
                res=ioctl(conn, FIONBIO, &flag);
                assert (res == 0);
                orig_stdout = dup(fileno(stdout));
                dup2(conn, fileno(stdout));
             }
#else
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
#endif
          }
          lost = 0;
          switch (selectedAction)
          {
             case memoryStats:
                msData = monitor_msNew (extended, raw, delta, preallocated);
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
#ifndef INTEGRITY
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
#endif
                {
#ifdef INTEGRITY
                   count = read (conn, &c, 1);
                   if( count != -1 || errno == EAGAIN  )
                   {
#else
                   if (ioctl (fileno(stdin), FIONREAD, &count) == 0) 
                   {
                      count = read (fileno(stdin), &c, 1);
#endif
                      if (count > 0) 
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
#ifdef INTEGRITY
                      dup2( orig_stdout, fileno(stdout));
#endif
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
#ifndef INTEGRITY
          if (isatty (fileno(stdin)) && !raw) 
          {
             count = read (fileno(stdin), &c, 1);

             if(count != -1)
             {
                tcsetattr (fileno(stdin), TCSAFLUSH, &old_termios);
             }
          }
#endif
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
#ifdef INTEGRITY
    fclose(stdout);
    shutdown(conn, SHUT_RDWR);
    close(conn);
#endif

    return 0;
}

