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
#ifndef IDL_GENCORBACXXSTREAMSCCPP_H
#define IDL_GENCORBACXXSTREAMSCCPP_H

#include "idl_program.h"

idl_program idl_genCorbaCxxStreamsCcppProgram (void);

void
idl_genCorbaCxxStreamsCcpp_setClientHeader(
    os_char* newClientHeader);

#endif /* IDL_GENCORBACXXSTREAMSCCPP_H */
