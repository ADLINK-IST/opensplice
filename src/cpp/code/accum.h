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
