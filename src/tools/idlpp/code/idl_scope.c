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

#include "idl_scope.h"
#include "idl_genCxxHelper.h"

#include "os_heap.h"
#include "os_stdlib.h"

#define IDL_MAXSCOPE	(20)

/*
   This modules handles the name scopes of objects by providing
   a scope stack. Each time a name scope is defined, the name
   is pushed on the stack. Each time the name scope is left,
   the name is popped from the stack. Name scopes are defined
   by the IDL definition "module <name>", "struct <name>" and
   "union <name>".
*/

/* idl_scopeElement specifies a scope stack element where
   "scopeName" specifies the name of the scope and "scopeType"
   specifies the kind of scope, either "idl_tFile" which
   is currently not used, "idl_tModule" for a module definition,
   "idl_tStruct" for a structure definition and "idl_tUnion"
   for a union definition.
*/
C_STRUCT(idl_scopeElement) {
    c_char *scopeName;
    idl_scopeType scopeType;
};

/* idl_scope specifies the scope stack where "stack" is an
   array of "idl_scopeElement", "baseName" specifies the
   basename of the file that contains the scope stack and
   "scopePointer" specifies the actual top of the stack
   (-1 specifies an empty stack).
*/
C_STRUCT(idl_scope) {
    idl_scopeElement stack[IDL_MAXSCOPE];
    c_char *baseName;
    c_long scopePointer;
};

/* Create a new scope element with the specified name and type */
idl_scopeElement
idl_scopeElementNew (
    const char *scopeName,
    idl_scopeType scopeType)
{
    idl_scopeElement element;

    assert (scopeName);
    assert (strlen(scopeName));

    element = os_malloc ((size_t)C_SIZEOF(idl_scopeElement));
    element->scopeName = os_strdup (scopeName);
    element->scopeType = scopeType;

    return element;
}

/* Free a scope element */
void
idl_scopeElementFree (
    idl_scopeElement element)
{
    assert (element);

    os_free (element->scopeName);
    os_free (element);
}

/* Create a copy of an existing scope element */
idl_scopeElement
idl_scopeElementDup (
    idl_scopeElement element)
{
    idl_scopeElement new_element;

    assert (element);
    new_element = os_malloc ((size_t)C_SIZEOF(idl_scopeElement));

    new_element->scopeName = os_strdup (element->scopeName);
    new_element->scopeType = element->scopeType;

    return new_element;
}

/* Return the scope name related to a specific scope element */
c_char *
idl_scopeElementName (
    idl_scopeElement element)
{
    if (element) {
        return element->scopeName;
    }
    return "";
}

/* Return the scope type related to a specific scope element */
idl_scopeType
idl_scopeElementType (
    idl_scopeElement element)
{
    if (element == NULL) {
        /* Empty scope stack will deliver NULL scope element */
        return idl_tModule;
    }
    return element->scopeType;
}

/* Create a new and empty scope stack, for a specified basename */
idl_scope
idl_scopeNew (
    const char *baseName)
{
    idl_scope scope = os_malloc ((size_t)C_SIZEOF(idl_scope));

    scope->baseName = os_strdup (baseName);
    scope->scopePointer = -1;
    return scope;
}

/* Create a new and empty scope stack, for a specified basename */
idl_scope
idl_scopeDup (
    idl_scope scope)
{
    idl_scope newScope = idl_scopeNew(idl_scopeBasename(scope));
    c_long si;

    for (si = 0; si < (scope->scopePointer+1); si++) {
        idl_scopePush (newScope, idl_scopeElementDup(scope->stack[si]));
    }
    return newScope;
}

/* Free a scope stack, also freeing all scope elements */
void
idl_scopeFree (
    idl_scope scope)
{
    c_long si;

    assert (scope);
    for (si = 0; si < (scope->scopePointer+1); si++) {
        idl_scopeElementFree(scope->stack[si]);
    }
    os_free (scope->baseName);
    os_free (scope);
    return;
}

/* Push a scope element on the scope stack */
void
idl_scopePush (
    idl_scope scope,
    idl_scopeElement element
    )
{
    assert (scope);
    assert (scope->scopePointer < IDL_MAXSCOPE);
    assert (element);

    scope->scopePointer++;
    scope->stack[scope->scopePointer] = element;
}

/* Return the size of a scope stack (the amount of scope elements) */
c_long
idl_scopeStackSize (
    idl_scope scope)
{
    return (scope->scopePointer+1);
}

/* Remove the top element from a scope stack */
void
idl_scopePop (
    idl_scope scope)
{
    assert (scope);
    assert (scope->scopePointer >= 0);

    scope->scopePointer--;
}

/* Remove the top element from a scope stack, and free its resources */
void
idl_scopePopFree (
    idl_scope scope)
{
    assert (scope);
    assert (scope->scopePointer >= 0);

    idl_scopeElementFree(scope->stack[scope->scopePointer]);
    scope->scopePointer--;
}

/* Return the top element from a scope stack */
idl_scopeElement
idl_scopeCur (
    idl_scope scope)
{
    assert (scope);
    assert (scope->scopePointer >= -1);

    if (scope->scopePointer == -1) {
        return NULL;
    }
    return scope->stack[scope->scopePointer];
}

/* Return the element from a scope stack, by index 
   where "index" >= 0 and "index" <= "scopePointer"
*/
idl_scopeElement
idl_scopeIndexed (
    idl_scope scope,
    c_long index)
{
    assert (index >= 0);
    assert (index <= scope->scopePointer);

    return scope->stack[index];
}

/* Determine if two scope stacks are equal */
c_bool
idl_scopeEqual (
    idl_scope scope1,
    idl_scope scope2)
{
    c_long i;

    /* If the "scopePointer"s are unequal, the stack do not equal */
    if (scope1->scopePointer != scope2->scopePointer) {
        return FALSE;
    }
    /* Per stack element compare the names, if any does not equal the stacks are unequal */
    for (i = 0; i < (scope1->scopePointer + 1); i++) {
        if (strcmp(idl_scopeElementName(scope1->stack[i]), idl_scopeElementName(scope2->stack[i])) != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Determine if a scope stack is contained by a second stack */
c_bool
idl_scopeSub (
    idl_scope scope, /* moduleScope */
    idl_scope scopeSub) /* keyScope */
{
    c_long i;

    /* If the "scopePointer" of the stack is higher than the "scopePointer" of the second
       stack, the second stack can not contain the first stack
    */

    if (scope->scopePointer > scopeSub->scopePointer) {
        return FALSE;
    }
    /* For all scope elements of the stack with the scope elements of the second stack.
       If one of them does not equal, the second stack can not contain the first.
       The scope element types are not compared, this should not a real problem.
    */
    for (i = 0; i < (scope->scopePointer + 1); i++) {
        if (strcmp(idl_scopeElementName(scope->stack[i]), idl_scopeElementName(scopeSub->stack[i])) != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Build a textual representation of a scope stack with a
   specified seperator and optionally add a user specified
   identifier
*/
c_char *
idl_scopeStack (
    idl_scope scope,
    const char *scopeSepp,
    const char *name)
{
    c_long si;
    c_char *scopeStack;
    c_char *elName;

    if (scope && (scope->scopePointer >= 0)) {
        /* If the stack is not empty */
    si = 0;
        /* copy the first scope element name */
        scopeStack = os_strdup (idl_scopeElementName(scope->stack[si]));
        si++;
    /* for all scope elements */
        while (si <= scope->scopePointer) {
        elName = idl_scopeElementName(scope->stack[si]);
        /* allocate space for current scope stack +
           separator + next scope name
        */
        scopeStack = os_realloc (scopeStack, (size_t)(
        (int)strlen(scopeStack)+
            (int)strlen(scopeSepp)+
        (int)strlen(elName)+1));
        /* concatinate the separator */
        os_strcat (scopeStack, scopeSepp);
        /* concatinate scope name */
        os_strcat (scopeStack, elName);
        si++;
        }
    if (name) {
        /* if a user identifier is specified,
           allocate space for current scope stack +
           separator + user identifier
        */
        scopeStack = os_realloc (scopeStack, (size_t)(
        (int)strlen(scopeStack)+
            (int)strlen(scopeSepp)+
        (int)strlen(name)+1));
        /* concatinate the separator */
        os_strcat (scopeStack, scopeSepp);
        /* concatinate user identifier */
        os_strcat (scopeStack, name);
    }
    } else {
        /* Empty scope stack */
        if (name) {
            /* if a user identifier is specified,
               copy the user identifier
            */
            scopeStack = os_strdup (name);
        } else {
            /* make the scope stack representation empty */
            scopeStack = os_strdup("");
        }
    }

    /* return the scope stack represenation */
    return scopeStack;
}

/* Return the basename related to a scope */
c_char *
idl_scopeBasename (
    idl_scope scope)
{
    return os_strdup(scope->baseName);
}
