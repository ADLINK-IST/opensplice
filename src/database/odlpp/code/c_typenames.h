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

#ifndef C_TYPENAMES_H
#define C_TYPENAMES_H

#include "c_typebase.h"
#include "c_metabase.h"

typedef enum c_scopeWhen {
    C_SCOPE_ALWAYS,
    C_SCOPE_NEVER,
    C_SCOPE_SMART
} c_scopeWhen;

c_char *c_getScopedTypeName(c_metaObject scope,
                            c_type type,
                            c_char *separator,
                            c_scopeWhen scopeWhen);

c_char *c_getScopedConstName(c_metaObject scope,
                             c_constant c,
                             c_char *separator,
                             c_scopeWhen scopeWhen);


#endif /* C_TYPENAMES_H */
