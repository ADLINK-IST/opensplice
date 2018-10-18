/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef PA_VXWORKS_ABSTRACT_H
#define PA_VXWORKS_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <vxWorks.h>

#define CONF_PARSER_NOFILESYS

#if (_BYTE_ORDER == _LITTLE_ENDIAN)
#define PA__LITTLE_ENDIAN
#elif (_BYTE_ORDER == _BIG_ENDIAN)
#define PA__BIG_ENDIAN
#else
#error "_BYTE_ORDER must be defined, check compilation flags"
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */

#if defined (__cplusplus)
}
#endif

#define OPENSPLICE_ENTRYPOINT(n) int n##_unique_main (int argc, char ** argv)

#define OPENSPLICE_ENTRYPOINTCALL(n, argc, argv) n##_unique_main ( ( argc ), ( argv ) )

#define OPENSPLICE_ENTRYPOINT_DECL(n) int n##_unique_main (int argc, char ** argv)

#define OPENSPLICE_MAIN(n) \
int n##_vx_unique_main (int argc, char ** argv); \
OPENSPLICE_ENTRYPOINT_DECL(n); \
int n (char * args) \
{ \
   int argc=1; \
   char *argv[256]; \
   char *saveptr; \
   char *str1; \
   argv[0] = os_strdup (#n);\
   if ( args != NULL ) \
   { \
      str1 = os_strtok_r(args, " ", &saveptr); \
      while (str1) \
      { \
         argv[argc] = os_strdup (str1); \
         argc++; \
         str1 = os_strtok_r(NULL, " ", &saveptr); \
      } \
   } \
   argv[argc] = NULL; \
   return n##_vx_unique_main (argc, argv); \
} \
int n##_vx_unique_main (int argc, char ** argv)



#endif /* PA_VXWORKS_ABSTRACT_H */
