#ifndef IDL_WALK_H
#define IDL_WALK_H

#include <c_base.h>
#include <c_typebase.h>

#include "idl_program.h"
#include "idl_tmplExp.h"

void 
idl_walkPresetFile(
    const char *fileName);

void 
idl_walkPresetModule(
    const char *moduleName);

void
idl_walk(
    c_base base,
    const char *fileName,
    c_bool traceWalk,
    idl_program program);

#endif /* IDL_WALK_H */
