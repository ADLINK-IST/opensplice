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
#include "os.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
/* #include <winioctl.h> */

#include "os_time.h"
#include "c__base.h"
#include "u_user.h"
#include "u__user.h"
#include "os_report.h"

#include "mm_orc.h"
#include "mm_trc.h"
#include "mm_ms.h"

#define SLEEP_INTERVAL_SEC 0
#define SLEEP_INTERVAL_NANO_SEC 100 * 1000 * 1000

static const struct os_time SLEEP_INTERVAL = { SLEEP_INTERVAL_SEC,  SLEEP_INTERVAL_NANO_SEC};
static const char *optflags="i:l:f:s:o:n:hetTmMa";
u_participant participant;

typedef enum {
   memoryStats,
   typeRefCount,
   objectRefCount
} monitorMode;

/*
 * Function to handle Ctrl-C presses.
 * @param fdwCtrlType Ctrl signal type
 */
static c_bool CtrlHandler(DWORD fdwCtrlType)
{
    u_participantFree(participant);
    u_userDetach();
    return TRUE;
}

static void
print_usage ()
{
    printf ("Usage:\n"
            "      mmstat -h\n"
            "      mmstat [-M|m] [-e] [-a] [-i interval] [-s sample_count] [URI]\n"
            "      mmstat [-t|T] [-i interval] [-s sample_count] [-l limit] [-o C|S|T] [-n nrEntries] [-f filter_expression] [URI]\n");
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
    printf ("      -e               Extended mode, shows bar for allocated memory\n");
    printf ("      -a               Show pre-allocated memory as well.\n");
    printf ("      -i interval      Display interval (in milliseconds)\n");
    printf ("      -s sample_count  Stop after sample_count samples\n");
    printf ("      -l limit         Show only object count >= limit\n");
    printf ("      -o <C|S|T>       Order by object[C]ount/object[S]ize/[T]otalSize\n");
    printf ("      -n nrEntries     Display only the top nrEntries items (useful only in combination with ordering)\n");
    printf ("      -f filter_expr   Show only meta objects which name passes the filter expression\n");
    printf ("\n");
    printf ("Use 'q' to terminate the monitor\n"
            "Use 't' to immediately show statistics\n");
}

int
main (int argc, char *argv[])
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
   v_participantQos pqos;
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

   HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
   INPUT_RECORD buffer[1];
   DWORD events;

   orderKind selectedOrdering  = NO_ORDERING;
   int orderCount = INT_MAX;

   /* Register handler for Ctrl-C */
   SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

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
               if (selectedOrdering  == NO_ORDERING && strlen(optarg) == 1)
               {
                   switch (optarg[0])
                   {
                       case 'C':
                           selectedOrdering = ORDER_BY_COUNT;
                           break;
                       case 'S':
                           selectedOrdering = ORDER_BY_SIZE;
                           break;
                       case 'T':
                           selectedOrdering = ORDER_BY_TOTAL;
                           break;
                       default:
                           fprintf(stderr, "mmstat: Unknown ordering kind.\n");
                           print_usage();
                           exit(-1);
                   }
               }
               else
               {
                   fprintf(stderr, "mmstat: Cannot specify multiple orderings at the same time.\n");
                   print_usage();
                   exit(-1);
               }
               break;
           case 'n':
               if (!(sscanf (optarg, "%d", &orderCount)) > 0)
               {
                   fprintf(stderr, "mmstat: Not a valid ordering nrEntries.\n");
                   print_usage();
                   exit(-1);
               }
               break;
#ifdef OBJECT_WALK
           case 'r':
               selectedAction = objectRefCount;
               break;
           case 'R':
               selectedAction = objectRefCount;
               delta = TRUE;
               break;
#endif
           case 'h':
           case '?':
           default:
               print_usage ();
               exit(0);
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
   if ((argc - optind) == 1)
   {
      uri = argv[optind];
   }

   if( !raw) {
      if(strlen(uri) > 0) {
         sddsURI = uri;
      } else {
         sddsURI = os_getenv ("OSPL_URI");
         if(!sddsURI) {
            sddsURI = DOMAIN_NAME;
         }
      }
      printf("Trying to open connection with the OpenSplice system using URI:\n" \
             "'%s'...\n", sddsURI);
   } else {
       sddsURI = uri;
   }

   ur = u_userInitialise();

   if(ur == U_RESULT_OK)
   {
      pqos = u_participantQosNew(NULL);
      participant = u_participantNew(sddsURI, 30, "mmstat", (v_qos)pqos, TRUE);
      u_participantQosFree(pqos);

      if(participant)
      {
         if( !raw )
         {
            printf("Connection established.\n\n");
         }

         lost = 0;
         switch (selectedAction)
         {
         case memoryStats:
            msData = monitor_msNew (extended, raw, delta, preallocated);
            break;
         case typeRefCount:
            trcData = monitor_trcNew (objectCountLimit, filterExpression, selectedOrdering, orderCount, delta);
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
               fflush (stdout);
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
               if (_isatty(_fileno(stdin)) && !raw)
               {
                  c = '\0';
                  PeekConsoleInput( handle, buffer, 1, &events );

                  if(events > 0)
                  {
                     ReadConsoleInput(handle, buffer, 1, &events);

                     if(buffer[0].EventType == KEY_EVENT
                        && buffer[0].Event.KeyEvent.bKeyDown)
                     {
                        c = buffer[0].Event.KeyEvent.uChar.AsciiChar;
                     }

                  }

                  if (c == 'q' || c == '\03' /* ^C */)
                  {
                     no_break = FALSE;
                  }
                  else if (c == 't')
                  {
                     trigger = 1;
                  }

                  PeekConsoleInput( handle, buffer, 1, &events );

                  if(events > 0)
                  {
                     ReadConsoleInput(handle, buffer, 1, &events);

                     if(buffer[0].EventType == KEY_EVENT
                        && buffer[0].Event.KeyEvent.bKeyDown)
                     {
                        c = buffer[0].Event.KeyEvent.uChar.AsciiChar;
                     }
                  }
               }
               if (no_break && interval)
               {
                  delay -= 100;
                  os_nanoSleep (SLEEP_INTERVAL);
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
         PeekConsoleInput( handle, buffer, 1, &events );

         if(events > 0)
         {
            ReadConsoleInput(handle, buffer, 1, &events);

            if(buffer[0].EventType == KEY_EVENT
               && buffer[0].Event.KeyEvent.bKeyDown)
            {
               c = buffer[0].Event.KeyEvent.uChar.AsciiChar;
            }
         }

         if(lost)
         {
            printf("\nConnection with domain lost. The OpenSplice system has\n" \
                   "probably been shut down.\n");
         }
         u_participantFree(participant);
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

