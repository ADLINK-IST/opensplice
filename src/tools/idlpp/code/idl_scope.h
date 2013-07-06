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
#ifndef IDL_SCOPE_H
#define IDL_SCOPE_H

#include "c_typebase.h"

typedef enum {
    idl_tFile,
    idl_tModule,
    idl_tStruct,
    idl_tUnion
} idl_scopeType;

#define idl_scopeElement(o) ((idl_scopeElement)(o))
#define idl_scope(o) ((idl_scope)(o))

C_CLASS(idl_scopeElement);
C_CLASS(idl_scope);

idl_scopeElement idl_scopeElementNew (const char *name, idl_scopeType type);

void idl_scopeElementFree (idl_scopeElement element);

idl_scopeElement idl_scopeElementDup (idl_scopeElement element);

c_char *idl_scopeElementName (idl_scopeElement element);

idl_scopeType idl_scopeElementType (idl_scopeElement element);

idl_scope idl_scopeNew (const char *baseName);

idl_scope idl_scopeDup (idl_scope scope);

void idl_scopeFree (idl_scope scope);

void idl_scopePush (idl_scope scope, idl_scopeElement element);

c_long idl_scopeStackSize (idl_scope scope);

void idl_scopePop (idl_scope scope);

void idl_scopePopFree (idl_scope scope);

idl_scopeElement idl_scopeCur (idl_scope scope);

idl_scopeElement idl_scopeIndexed (idl_scope scope, c_long index);

c_bool idl_scopeEqual (idl_scope scope1, idl_scope scope2);

c_bool idl_scopeSub (idl_scope scope, idl_scope scopeSub);

c_char *idl_scopeStack (idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_scopeBasename (idl_scope scope);

#endif /* IDL_SCOPE_H */
