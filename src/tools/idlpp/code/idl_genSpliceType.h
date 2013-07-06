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
#ifndef IDL_GENSPLICETYPE_H
#define IDL_GENSPLICETYPE_H

#include "idl_program.h"

idl_program idl_genSpliceTypeProgram (void);

void
idl_genSpliceTypeSetTestMode (
    c_bool val);

void
idl_genSpliceTypeSetIncludeFileName (
    os_char* val);

void
idl_genSpliceTypeUseVoidPtrs (
    c_bool val);

#endif /* IDL_GENSPLICETYPE_H */
