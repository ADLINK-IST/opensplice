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

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 512
#define VSB_CONFIG_FILE_STRING "-D_VSB_CONFIG_FILE=\""

extern char **environ;
typedef char *cptr;
static cptr newargs[MAX_ARGS];
static int argNum = 0;

void addarg( const char *pattern, const char *val )
{
   char *winval;
   int newlen;
   int len = cygwin32_posix_to_win32_path_list_buf_size (val);
   winval = malloc(len);
   cygwin32_conv_to_win32_path(val, winval );
   newlen=strlen(pattern) + strlen(winval) -1;
   char *newarg = malloc( newlen );
   snprintf( newarg, newlen, pattern, winval );
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
   char *winval = NULL;
   char *nextpath;
   char *val1;
   int newlen = 0;

   val1 = strdup (val); /* strtok is destructive */
   nextpath = strtok (val1, ":");
   while (nextpath)
   {
      newlen += cygwin32_posix_to_win32_path_list_buf_size (nextpath) + 1;
      nextpath = strtok (NULL, ":");
   }
   free (val1);

   if (newlen)
   {
      winval = malloc (newlen);
      nextpath = strtok (val, ":");
      cygwin32_conv_to_win32_path (nextpath, winval);
      while (nextpath = strtok (NULL, ":"))
      {
         strcat (winval, ";");
         cygwin32_conv_to_win32_path (nextpath, winval + strlen (winval));
      }
   }
   else
   {
      winval = "";
   }
   newargs [argNum++] = winval;
   if (argNum+1 >= MAX_ARGS)
   {
      fprintf(stderr, "Error: ospl_wincmd Max number args exceeded\n");
      fflush(stderr);
      exit (1);
   }
}

void fixenv( const char *envname )
{
   char *winval;
   int newlen;
   char *env;
   env = getenv(envname);
   if ( env && env[0] != '\0' )
   {
     int len = cygwin32_posix_to_win32_path_list_buf_size (env);
     winval = malloc(len);
     cygwin32_conv_to_win32_path(env, winval );
     setenv( envname, winval, 1 );
     free(winval);
   }
}

int main( int argc, char ** argv)
{
   int count;
   int i;

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
              || !strcmp( exe, "ldppc" ))
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
                     int arglen = strlen( narg );
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
                     int arglen = strlen( narg );
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
               int arglen = strlen( narg );
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

   if ( execvp( argv[1], newargs, environ ) == -1 )
   {
      fprintf(stderr, "ERROR: exec failed %d\n", errno);
      fflush(stderr);
   }
}
