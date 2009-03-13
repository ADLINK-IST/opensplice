#ifndef IDL_REGISTERTYPE_H
#define IDL_REGISTERTYPE_H

#include <c_base.h>
#include <c_iterator.h>

void
idl_registerType (
    c_base base,
    const char *basename,
    c_iter typeNames
    );

#endif /* IDL_REGISTERTYPE_H */
