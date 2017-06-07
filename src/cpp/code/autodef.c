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
#include "cpp_malloc.h"
#include "symtbl.h"
#include "if.h"

static unsigned char * temp;

extern void autodef_file (const char * f)
{
   int i;

   i = strlen (f);
   temp = os_malloc (i + 2 + 1);
   check_os_malloc (temp);
   os_sprintf ((char*) temp, "\"%s\"", f);
   undef ("__FILE__");
   define ("__FILE__", -1, temp, DEF_DEFINE);
}

extern void autodef_line (int l)
{
   defd ("__LINE__", l);
}
