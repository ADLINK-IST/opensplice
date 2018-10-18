/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef UT_XMLPARSER_H
#define UT_XMLPARSER_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

    typedef int (*ut_xmlpProcElemOpen_t) (void *varg, os_address parentinfo, os_address *eleminfo, const char *name);
    typedef int (*ut_xmlpProcAttr_t) (void *varg, os_address eleminfo, const char *name, const char *value);
    typedef int (*ut_xmlpProcElemData_t) (void *varg, os_address eleminfo, const char *data);
    typedef int (*ut_xmlpProcElemClose_t) (void *varg, os_address eleminfo);
    typedef void (*ut_xmlpError) (void *varg, const char *msg, int line);

    struct ut_xmlpCallbacks {
        ut_xmlpProcElemOpen_t elem_open;
        ut_xmlpProcAttr_t attr;
        ut_xmlpProcElemData_t elem_data;
        ut_xmlpProcElemClose_t elem_close;
        ut_xmlpError error;
    };

    struct ut_xmlpState;

    OS_API struct ut_xmlpState *ut_xmlpNewFile (FILE *fp, void *varg, const struct ut_xmlpCallbacks *cb);
    OS_API struct ut_xmlpState *ut_xmlpNewString (const char *string, void *varg, const struct ut_xmlpCallbacks *cb);
    OS_API void ut_xmlpFree (struct ut_xmlpState *st);
    OS_API int ut_xmlpParse (struct ut_xmlpState *st);

    OS_API int ut_xmlUnescapeInsitu (char *buffer, size_t *n);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
