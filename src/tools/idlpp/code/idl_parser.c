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
