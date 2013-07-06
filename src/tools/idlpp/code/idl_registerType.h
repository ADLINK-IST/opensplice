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
#ifndef IDL_REGISTERTYPE_H
#define IDL_REGISTERTYPE_H

#include "c_base.h"
#include "c_iterator.h"

void
idl_registerType (
    c_base base,
    const char *basename,
    c_iter typeNames
    );

#endif /* IDL_REGISTERTYPE_H */
