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
#ifndef _ACCUM_
#define _ACCUM_

#ifdef __cplusplus
extern "C"
{
#endif

   typedef struct
   {
      int have;
      int used;
      char *buf;
   }
   ACCUM;

   extern char * init_accum (void);
   extern char * accum_result (char *);
   extern char accum_regret (char *);
   extern char * accum_buf (char *);
   extern void accum_char (char *, char);
   extern char * accum_sofar (char *);
   extern int accum_howfar (char *);

#ifdef __cplusplus
}
#endif

#endif
