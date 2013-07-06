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
#include <assert.h>

#include "c_typebase.h"
#include "c_iterator.h"

#include "os_heap.h"
#include "os_stdlib.h"

#include "idl_dependencies.h"

/* "idl_dep" contains all files the are included and contain definitions
   the processed file is dependent on
*/
C_STRUCT(idl_dep) {
    c_iter deps;
};

/* defaultDep stores the default dependency list */
static idl_dep defaultDep;

/* Create a new dependency list */
idl_dep
idl_depNew (
    void)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_dep dep = os_malloc ((size_t)C_SIZEOF(idl_dep));

    dep->deps = c_iterNew (0);

    return dep;
}

/* QAC EXPECT 5007; suppress QACtools error */
/* Free a dependency list freeing all of its contained resources */
void
idl_depFree (
    const idl_dep dep)
{
    char *basename;

    basename = c_iterTakeFirst (dep->deps);
    while (basename != NULL) {
        /* QAC EXPECT 5007; will not use wrapper */
	os_free (basename);
        basename = c_iterTakeFirst (dep->deps);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (dep);
}

/* Create a new dependency list and make it the default */
void
idl_depDefInit (
    void)
{
    defaultDep = idl_depNew ();
}

/* Get the default dependency list */
idl_dep
idl_depDefGet (
    void)
{
    return defaultDep;
}

/* Free the default dependency list */
void
idl_depDefExit (
    void)
{
    idl_depFree (defaultDep);
}

/* Check if the provided basename list element is equal
   to the provided basename
*/
static c_equality
idl_depCompare (
    void *_listBasename,
    void *_basename)
{
    char *listBasename;
    char *basename;
    c_equality result = C_NE;

    listBasename = _listBasename;
    basename = _basename;

    /* QAC EXPECT 5007, 3416; will not use wrapper, No side effects in this case, expected behaviour */
    if (strcmp (listBasename, basename) == 0) {
	result = C_EQ;
    }

    return result;
}

/* Add a file dependency to the specified dependency list */
void
idl_depAdd (
    const idl_dep dep,
    const char *basename)
{
    c_char *b;

    /* Find the file in the specified dependency list */
    b = c_iterResolve (dep->deps, idl_depCompare, (c_iterResolveCompareArg)basename);
    if (b == NULL) {
	/* If not found, the add it */
        dep->deps = c_iterAppend (dep->deps, os_strdup(basename));
    }
}

/* Get the dependencies by index, where "index" >= 0 and
   "index" < the dependency list length
*/
c_char *
idl_depGet (
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_dep dep,
    c_long index)
{
    c_char *basename;

    /* QAC EXPECT 3416; No side effects in this case, expected behaviour */
    assert (index >= 0);
    /* QAC EXPECT 3416; No side effects in this case, expected behaviour */
    assert (index < c_iterLength(dep->deps));

    basename = c_iterObject (dep->deps, index);

    return basename;
}

/* return the length of the provided dependency list */
c_long
idl_depLength (
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_dep dep)
{
    return c_iterLength (dep->deps);
}
