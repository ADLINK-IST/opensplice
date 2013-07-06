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
#ifndef IDL_GENSACSSPLDCPS_H_
#define IDL_GENSACSSPLDCPS_H_

#include "idl_program.h"

struct SACSSplDcpsUserData_s {
    c_bool customPSM;
};

typedef struct SACSSplDcpsUserData_s SACSSplDcpsUserData;

idl_program
idl_genSACSSplDcpsProgram(SACSSplDcpsUserData *userData);

#endif /* IDL_GENSACSSPLDCPS_H_ */
