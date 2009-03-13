#ifndef IDL_GENCXXHELPER_H
#define IDL_GENCXXHELPER_H

#include <c_typebase.h>

#include "idl_scope.h"
#include "idl_program.h"

c_char *idl_cxxId(const char *identifier);

c_char *idl_scopeStackCxx(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaCxxTypeFromTypeSpec(idl_typeSpec typeSpec);

c_char *idl_genCxxConstantGetter(void);

#endif /* IDL_GENCXXHELPER_H */
