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
#ifndef IDL_GENMETAHELPER_H
#define IDL_GENMETAHELPER_H

#include "idl_scope.h"

#include "c_metabase.h"

char *idl_genXMLmeta (c_type type);

/*
 * dds#2745
 * Allows to replace
 * 'A very very big string'
 * by :
 * 'a very"
 * "very big"
 * "string'
 */
char *idl_cutXMLmeta (char *meta, c_ulong *nrOfElements);

#endif /* IDL_GENMETAHELPER_H */
