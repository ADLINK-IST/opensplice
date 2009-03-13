#ifndef IDL_GENSACHELPER_H
#define IDL_GENSACHELPER_H

#include <c_typebase.h>

#include "idl_program.h"

c_char *idl_scopedSacTypeIdent (const idl_typeSpec typeSpec);

c_char *idl_sacTypeFromTypeSpec (const idl_typeSpec typeSpec);

char *idl_sequenceElementIdent (const idl_typeSpec typeSpec);

char *idl_sequenceIdent (const idl_typeSeq typeSeq);

#endif /* IDL_GENSACHELPER_H */
