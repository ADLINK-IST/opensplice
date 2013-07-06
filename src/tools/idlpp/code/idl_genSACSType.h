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
#ifndef IDL_GENSACSTYPE_H
#define IDL_GENSACSTYPE_H

#include "idl_program.h"

struct SACSTypeUserData_s {
    os_iter idlpp_metaList;
    c_char *tmplPrefix;
    c_bool customPSM;
};

typedef struct SACSTypeUserData_s SACSTypeUserData;

idl_program idl_genSACSTypeProgram(SACSTypeUserData *userData);

#endif /* IDL_GENSACSTYPE_H */
