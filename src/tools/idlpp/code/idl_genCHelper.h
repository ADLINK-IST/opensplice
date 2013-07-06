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
#ifndef IDL_GENCHELPER_H
#define IDL_GENCHELPER_H

#include "c_typebase.h"

#include "idl_scope.h"
#include "idl_program.h"

c_char *idl_cId(const char *identifier);

c_char *idl_scopeStackC(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaCTypeFromTypeSpec(idl_typeSpec typeSpec);

/* clean definitions */
void idl_definitionClean(void);

/* add a definition within the specified scope */
void idl_definitionAdd(char *class, char *name);

/* return 1 if the definition already exists within the specified scope */
/* return 0 if the definition does not exists within the specified scope */
int idl_definitionExists(char *class, char *name);

c_char *idl_genCConstantGetter(void);

#endif /* IDL_GENCHELPER_H */
