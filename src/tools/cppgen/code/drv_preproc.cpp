/*
COPYRIGHT

Copyright 1992, 1993, 1994 Sun Microsystems, Inc.  Printed in the United
States of America.  All Rights Reserved.

This product is protected by copyright and distributed under the following
license restricting its use.

The Interface Definition Language Compiler Front End (CFE) is made
available for your use provided that you include this license and copyright
notice on all media and documentation and the software program in which
this product is incorporated in whole or part. You may copy and extend
functionality (but may not remove functionality) of the Interface
Definition Language CFE without charge, but you are not authorized to
license or distribute it to anyone else except as part of a product or
program developed by you or with the express written consent of Sun
Microsystems, Inc. ("Sun").

The names of Sun Microsystems, Inc. and any of its subsidiaries or
affiliates may not be used in advertising or publicity pertaining to
distribution of Interface Definition Language CFE as permitted herein.

This license is effective until terminated by Sun for failure to comply
with this license.  Upon termination, you shall destroy or return all code
and documentation for the Interface Definition Language CFE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED AS IS WITH NO WARRANTIES OF
ANY KIND INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR ARISING FROM A COURSE OF
DEALING, USAGE OR TRADE PRACTICE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED WITH NO SUPPORT AND WITHOUT
ANY OBLIGATION ON THE PART OF Sun OR ANY OF ITS SUBSIDIARIES OR AFFILIATES
TO ASSIST IN ITS USE, CORRECTION, MODIFICATION OR ENHANCEMENT.

SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES SHALL HAVE NO LIABILITY WITH
RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY
INTERFACE DEFINITION LANGUAGE CFE OR ANY PART THEREOF.

IN NO EVENT WILL SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES BE LIABLE FOR
ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL, INDIRECT AND CONSEQUENTIAL
DAMAGES, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Use, duplication, or disclosure by the government is subject to
restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
Technical Data and Computer Software clause at DFARS 252.227-7013 and FAR
52.227-19.

Sun, Sun Microsystems and the Sun logo are trademarks or registered
trademarks of Sun Microsystems, Inc.

SunSoft, Inc.
2550 Garcia Avenue
Mountain View, California  94043

NOTE:

SunOS, SunSoft, Sun, Solaris, Sun Microsystems or the Sun logo are
trademarks or registered trademarks of Sun Microsystems, Inc.

 */


/*
 * DRV_pre_proc.cc - pass an IDL file through the C preprocessor
 */

#include "os_stdlib.h"
#include "os_heap.h"
#include "idl.h"
#include "idl_extern.h"
#include "drv_private.h"
#include "drv_link.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>    /// fpm 5/2/95: for error handling

#include "symtbl.h"
#include "include.h"
#include "preprocess.h"

#include "os_process.h"

#if defined(_WIN32)
# include <PROCESS.H>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "io.h"
# include <errno.h>

// you must delete the returned char * !
char * doubleslashes (const char * path)
{
   char * dbleslashes = new char[strlen(path) * 2 + 1];
   char *ds_ptr = dbleslashes;

   for (; *path != '\0'; ds_ptr++, path++)
   {
      if (*path == '\\')
      {
         *ds_ptr = '\\';
         ds_ptr++;
         *ds_ptr = '\\';
      }
      else
      {
         *ds_ptr = *path;
      }
   }

   *ds_ptr = '\0';
   return dbleslashes;
}

#endif

#ifdef DEBUG_MAIN
extern int debugging;
#endif

/*
 * Push an argument into the arglist
 */
void DRV_cpp_putarg(const char * arg)
{
   char *str = os_strdup (arg);
   char *str_orig = str;
   char *cp;
   char *dp;
   
   // Always define _DDS_CPP_ and _DDS_

   dp = os_strdup ("1");
   define ("_DDS_CPP_", -1, (unsigned char *) dp, DEF_CMDLINE);
   dp = os_strdup ("1");
   define ("_DDS_", -1, (unsigned char *) dp, DEF_CMDLINE);

   // skip '-'
   ++str;
   switch (*str)
   {
      case 'D':
      {
         ++str;
         cp = strchr(str,'=');
         if (cp)
         {
            if (cp != str)
            {
               *cp++ = '\0';
               dp = os_strdup (cp);
               define(str,-1,(unsigned char *) dp,DEF_CMDLINE);
            }
            else
            {
               fprintf(stderr,"Must give a name for -D\n");
            }
         }
         else
         {
            dp = os_strdup ("1");
            define (str,-1,(unsigned char *) dp,DEF_CMDLINE);
         }
         break;
      }
      case 'I':
      {
         ++str;
         if (*str)
         {
           Ifile(str);
         }
         else
         {
           fprintf(stderr,"Must give a directory name for -I\n");
         }
         break;
      }
      case 'U':
      {
         ++str;
         if (*str)
         {
            undef(str);
         }
         else
         {
            fprintf(stderr,"Must give a name for -U\n");
         }
         break;
      }
   }

   os_free (str_orig);
}

/*
 * Initialize the cpp argument list
 */
void
DRV_cpp_init()
{
   DRV_cpp_putarg("-DIDL");
   DRV_cpp_putarg("-I.");
}

/*
 * Strip down a name to the last component, i.e. everything after the last
 * '/' character
 */
static const char *
DRV_stripped_name(const char *fn)
{
   const char *n = fn;
   long l;

   if (n == NULL)
      return NULL;

   l = strlen(n);

   for (n += l; l > 0 && *n != '/'; l--, n--)

      ;
   if (*n == '/')
      n++;

   return n;
}


/*
 * Pass input through preprocessor
 */
void DRV_pre_proc(const char *myfile)
{

//   char catbuf[1664];
   const char *inname = "";
   FILE *inf;

   if (strcmp(myfile, "standard input") == 0)
   {
      inf = stdin;
      idl_global->set_filename((*DRV_FE_new_UTL_String)(""));
      idl_global->set_main_filename((*DRV_FE_new_UTL_String)(""));
      idl_global->set_stripped_filename(
         (*DRV_FE_new_UTL_String)(""));
      idl_global->set_real_filename((*DRV_FE_new_UTL_String)(""));
      idl_global->set_read_from_stdin(I_TRUE);
   }
   else
   {
#if !defined(_WIN32)
      idl_global->set_filename((*DRV_FE_new_UTL_String)(myfile));
      idl_global->set_main_filename((*DRV_FE_new_UTL_String)(myfile));
      idl_global->set_real_filename((*DRV_FE_new_UTL_String)(myfile));
#else

      char * dbslashes = doubleslashes (myfile);
      idl_global->set_filename ((*DRV_FE_new_UTL_String)(dbslashes));
      idl_global->set_main_filename ((*DRV_FE_new_UTL_String)(dbslashes));
      idl_global->set_real_filename ((*DRV_FE_new_UTL_String)(dbslashes));
      delete dbslashes;
#endif

      FILE *fd = fopen(myfile, "r");
      inf = fd;
      inname = myfile;
      idl_global->set_read_from_stdin(I_FALSE);
      idl_global->set_stripped_filename(
         (*DRV_FE_new_UTL_String)(DRV_stripped_name(myfile)));
   }

   if (inf == NULL)
   {
      fprintf (stderr, "Failed to open IDL file: %s\n", myfile);
      exit (1);
   }

   preprocess(inf, inname);

   if (idl_global->compile_flags() & IDL_CF_ONLY_PREPROC)
   {
      int ch;

      while ((ch = preprocess_getc()) !=  0)
      {
         putchar(ch);
      }

      exit(0);
   }
}
