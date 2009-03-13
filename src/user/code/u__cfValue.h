
#ifndef U__CFVALUE_H
#define U__CFVALUE_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

c_bool
u_cfValueScan(
    c_value value,
    c_valueKind valueKind,
    c_value *valuePtr);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFVALUE_H */
