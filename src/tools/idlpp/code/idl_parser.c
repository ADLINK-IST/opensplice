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
#include <stdio.h>
#include <string.h>

#include "c_base.h"
#include "c_metabase.h"
#include "c_module.h"

#include "idl_parser.h"
#include "idl_unsupported.h"
#include "idl_databaseValidation.h"

c_base
idl_parseFile(
    const char *filename,	/* the file that is processed */
    c_bool traceInput)
{
    static c_base base = NULL;
    OS_UNUSED_ARG(traceInput);

    OS_UNUSED_ARG(traceInput);

    /* Create a database on heap */
    if (base == NULL) {
        base = c_create("preprocessor",NULL,0, 0);
        if (base != NULL && idl_defineUnsupportedTypes(base)) {
            base = NULL;
        }
    }
    if (base != NULL) {
        /* Initialize the IDL parser */
        idl_idlinit(c_module(base));
        /* Parse the IDL input file */
        /* QAC EXPECT 3416; No side effects here */
        if (idl_idlparse(filename /* , traceInput */) != 0) {
            /* In case of an error, base is set to NULL to prevent further processing */
            base = NULL;
        }
    }
    if (base != NULL) {
        /* check resulting database is valid */
        if (!idl_validateDatabase(base, filename)) {
            /* In case of an error, base is set to NULL to prevent further processing */
            base = NULL;
        }
    }
    return base;
}
