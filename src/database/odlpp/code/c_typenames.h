
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
