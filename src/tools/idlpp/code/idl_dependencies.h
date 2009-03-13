#ifndef IDL_DEPENDENCIES_H
#define IDL_DEPENDENCIES_H

#include <c_typebase.h>

C_CLASS(idl_dep);

idl_dep idl_depNew (void);

void idl_depFree (const idl_dep dep);

void idl_depDefInit (void);

idl_dep idl_depDefGet (void);

void idl_depDefExit (void);

void idl_depAdd (const idl_dep dep, const char *basename);

c_char *idl_depGet (const idl_dep dep, c_long index);

c_long idl_depLength (const idl_dep dep);

#endif /* IDL_DEPENDENCIES */
