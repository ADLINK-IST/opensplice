#ifndef IDL_GENSPLHELPER_H
#define IDL_GENSPLHELPER_H

#include <c_typebase.h>

#include "idl_program.h"

void idl_printIndent (c_long indent);

c_char *idl_typeFromTypeSpec (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeIdent (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeIdent (const idl_typeSpec typeSpec);

#endif /* IDL_GENSPLHELPER_H */
