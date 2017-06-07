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

#include <sys/types.h>
#include <sys/cygwin.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGS 1024
#define VSB_CONFIG_FILE_STRING "-D_VSB_CONFIG_FILE=\""

extern char **environ;
typedef char *cptr;
static cptr newargs[MAX_ARGS];
static int argNum = 0;

/* We still have some ancient Cygwin installations around that do not
   support the new interfaces, but the old ones are absent on
   64-bit. */
#ifdef __x86_64__
static char *convpath (const char *pp)
{
    return cygwin_create_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE, pp);
}

static char *convpathlist (const char *ppl)
{
  ssize_t newlen = cygwin_conv_path_list (CCP_POSIX_TO_WIN_A | CCP_RELATIVE, ppl, NULL, 0);
  if (newlen < 0) {
      perror ("cygwin_conv_path_list(1)");
      exit (127);
  } else if (newlen == 0) {
      return strdup ("");
  } else {
      char *wpl = malloc (newlen);
      if (cygwin_conv_path_list (CCP_POSIX_TO_WIN_A | CCP_RELATIVE, ppl, wpl, newlen) < 0) {
          perror ("cygwin_conv_path_list(2)");
          exit (127);
      }
      return wpl;
  }
}
#else
static char *convpath (const char *pp)
{
   int len = cygwin32_posix_to_win32_path_list_buf_size (pp);
   char *wp = malloc (len);
   (void) cygwin32_conv_to_win32_path (pp, wp);
   return wp;
}

static char *convpathlist (const char *ppl)
{
    char *ppl_copy, *nextpath;
    int winlen = 0;

    ppl_copy = strdup (ppl); /* strtok is destructive */
    nextpath = strtok (ppl_copy, ":");
    while (nextpath) {
        winlen += cygwin32_posix_to_win32_path_list_buf_size (nextpath) + 1;
        nextpath = strtok (NULL, ":");
    }
    free (ppl_copy);

    if (winlen == 0) {
        return strdup ("");
    } else {
        char *wpl = malloc (winlen);
        ppl_copy = strdup (ppl);
        nextpath = strtok (ppl_copy, ":");
        cygwin32_conv_to_win32_path (nextpath, wpl);
        while ((nextpath = strtok (NULL, ":")) != NULL) {
            strcat (wpl, ";");
            cygwin32_conv_to_win32_path (nextpath, wpl + strlen (wpl));
        }
        free (ppl_copy);
        return wpl;
    }
}
#endif

void addarg( const char *pattern, const char *val )
{
    char *winval = convpath (val);
    int newlen = strlen(pattern) + strlen(winval) -1;
    char *newarg = malloc (newlen);
    snprintf (newarg, newlen, pattern, winval);
    free(winval);
    newargs[argNum++] = newarg;
    if (argNum+1 >= MAX_ARGS)
    {
      fprintf(stderr, "Error: ospl_wincmd Max number args exceeded\n");
      fflush(stderr);
      exit (1);
    }
}

void addjavapaths (char *val)
{
   newargs [argNum++] = convpathlist (val);
   if (argNum+1 >= MAX_ARGS)
   {
      fprintf(stderr, "Error: ospl_wincmd Max number args exceeded\n");
      fflush(stderr);
      exit (1);
   }
}

void fixenv( const char *envname )
{
   char *env;
   env = getenv(envname);
   if ( env && env[0] != '\0' )
   {
     char *winval = convpath (env);
     setenv( envname, winval, 1 );
     free(winval);
   }
}

int main( int argc, char ** argv)
{
   int count;

/* Executable name - strip off path if it was specified */

   char *exe = strrchr (argv[1], '/');
   if (exe)
   {
      exe += 1;
   }
   else
   {
      exe = argv[1];
   }

/* Argument processing */

   for ( count=1 ; count < argc; count++ )
   {
      char *arg = argv[count];
      if ( arg[0] == '-' )
      {
         if ( !strcmp( exe, "arpentium" )
              || !strcmp( exe, "arppc" ) )
         {
            addarg( "%s", argv[count] );
            fixenv( "WIND_HOME");
         }
         else if ( !strcmp( exe, "ccpentium" )
              || !strcmp( exe, "c++pentium" )
              || !strcmp( exe, "cpppentium" )
              || !strcmp( exe, "ldpentium" )
              || !strcmp( exe, "c++ppc" )
              || !strcmp( exe, "cppppc" )
              || !strcmp( exe, "ccppc" )
              || !strcmp( exe, "ldppc" )
              || !strcmp( exe, "qcc" )
              || !strcmp( exe, "ntoarm-gcc" )
              || !strcmp( exe, "ntoarm-g++" )
              || !strcmp( exe, "ccint86")
              || !strcmp( exe, "cxint86")
              || !strcmp( exe, "ccintppc")
              || !strcmp( exe, "cxintppc"))
         {
            fixenv( "WIND_HOME");
            switch ( arg[1] )
            {
               case 'I' :
               {
                  addarg( "-I%s", &arg[2] );
                  break;
               }
               case 'L' :
               {
                  addarg( "-L%s", &arg[2] );
                  break;
               }
               case 'o' :
               {
                  if ( arg[2] == '\0' )
                  {
                     char *narg=argv[count+1];
                     addarg( "%s", "-o" );
                     addarg( "%s", narg );
                     count++;
                  }
                  else
                  {
                     addarg( "%s", arg );
                  }
                  break;
               }
               case 'D' :
               {
                  if ( strncmp( arg, VSB_CONFIG_FILE_STRING, strlen(VSB_CONFIG_FILE_STRING) ) == 0 )
                  {
                     char *tmp= strdup(&arg[strlen(VSB_CONFIG_FILE_STRING)]);
                     tmp[strlen(tmp)-1] = '\0'; /* strip off trailing double quote */
                     addarg( VSB_CONFIG_FILE_STRING "%s\"",tmp  );
                     free( tmp );
                     break;
                  }
                  else
                  {
                     addarg( "%s", argv[count] );
                  }
                  break;
               }
               default:
               {
                  addarg( "%s", argv[count] );
               }
            }
         }
         else if ( !strcmp( exe, "idlpp" )
              || !strcmp( exe, "tao_idl" )
              || !strcmp( exe, "idl" )
              || !strcmp( exe, "rmipp" )
              || !strcmp( exe, "odlpp" )
         )
         {
            switch ( arg[1] )
            {
               case 'I' :
               {
                  addarg( "-I%s", &arg[2] );
                  break;
               }
               case 'o' :
               {
                  if ( arg[2] == '\0' )
                  {
                     char *narg=argv[count+1];
                     addarg( "%s", "-o" );
                     addarg( "%s", narg );
                     count++;
                  }
                  else
                  {
                     addarg( "%s", arg );
                  }
                  break;
               }
               default:
               {
                  addarg( "%s", argv[count] );
               }
            }
         }
         else if ( !strcmp( exe, "mt" ))
         {
            if (!strcmp( arg, "-manifest" ))
            {
               char *narg=argv[count+1];
               addarg( "%s", arg );
               addarg( "%s", narg );
               count++;
            }
            else if (!strncmp( arg, "-outputresource:", 16 ))
            {
               addarg( "-outputresource:%s", &arg[16]);
            }
            else
            {
               addarg( "%s", argv[count] );
            }
         }
         else if ( !strcmp( exe, "Csc" ))
         {
            if (!strncmp( arg, "-out:", 5 ))
            {
               addarg( "-out:%s", &arg[5]);
            }
            else if (!strncmp( arg, "-reference:", 11 ))
            {
               addarg( "-reference:%s", &arg[11]);
            }
            else
            {
               addarg( "%s", argv[count] );
            }
         }
         else if ( !strcmp( exe, "cl" )
                   || !strcmp( exe, "link" )
                   || !strcmp( exe, "lib" ))
         {
            switch ( arg[1] )
            {
               case 'L' :
               {
                  addarg( "-LIBPATH:%s", &arg[2] );
                  break;
               }
               case 'I' :
               {
                  addarg( "-I%s", &arg[2] );
                  break;
               }
               case 'l' :
               {
                  addarg( "%s.lib", &arg[2] );
                  break;
               }
               case 'o' :
               {
                  if ( arg[2] == '\0' )
                  {
                     char *narg=argv[count+1];
                     int arglen = strlen( narg );

                     if ( narg[arglen-4] == '.'
                          && narg[arglen-3] == 'o'
                          && narg[arglen-2] == 'b'
                          && narg[arglen-1] == 'j' )
                     {
                        addarg( "-Fo%s", narg );
                        count++;
                     }
                     else
                     {
                        addarg( "-OUT:%s", narg );
                        count++;
                     }
                  }
                  else
                  {
                     addarg( "%s", arg );
                  }
                  break;
               }
               default:
               {
                  addarg( "%s", argv[count] );
               }
            }
         }
         else if (!strcmp (exe, "java" )
                   || !strcmp( exe, "javac" )
                   || !strcmp( exe, "javah" ))
         {
            if ( !strcmp (arg, "-cp")
                  || !strcmp( arg, "-classpath")
                  || !strcmp( arg, "-sourcepath")
                  || !strcmp( arg, "-endorseddirs")
                  || !strcmp( arg, "-bootclasspath"))
            {
               addarg ("%s", arg);
               addjavapaths (argv[++count]);
            }
            else if (!strcmp (arg, "-d"))
            {
               addarg ("%s", arg);
               addarg ("%s", argv[++count]);
            }
            else if (arg[1] == 'I') /* JacORB IDL compiler opt */
            {
                  addarg( "-I%s", &arg[2] );
            }
            else
            {
               addarg( "%s", argv[count] );
            }
         }
         else
         {
            addarg( "%s", argv[count] );
         }
      }
      else
      {
         addarg( "%s", argv[count] );
      }
   }
   newargs[argNum] = NULL;

   /*printf("\nRUNNING CMD:");
   for ( i=0 ; i < argNum; i++ )
   {
      printf(" %s", newargs[i]);
   }
   printf("\n");
   fflush(stdout);*/

   if ( execvp( argv[1], newargs ) == -1 )
   {
      fprintf(stderr, "ERROR: exec failed %d\n", errno);
      fflush(stderr);
   }
   return 127;
}
