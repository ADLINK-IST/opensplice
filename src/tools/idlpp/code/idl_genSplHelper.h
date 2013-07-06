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
#ifndef IDL_GENSPLHELPER_H
#define IDL_GENSPLHELPER_H

#include "c_typebase.h"

#include "idl_program.h"

enum idl_boundsCheckFailKind {
    CASE,
    DISCRIMINATOR,
    MEMBER,
    ELEMENT
};

void idl_boundsCheckFail (enum idl_boundsCheckFailKind kind, idl_scope scope, idl_typeSpec type, const c_char* name);

void idl_boundsCheckFailNull (enum idl_boundsCheckFailKind kind, idl_scope scope, idl_typeSpec type, const c_char* name);

void idl_printIndent (c_long indent);

c_char *idl_typeFromTypeSpec (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeIdent (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeIdent (const idl_typeSpec typeSpec);

#endif /* IDL_GENSPLHELPER_H */
