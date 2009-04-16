/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef NW_MISC_H
#define NW_MISC_H

#include "c_base.h"
#include "u_participant.h"
#include "nw_commonTypes.h"

#define NW_ALIGN(alignment, address)                       \
    ((nw_length)(address) + (alignment) - 1U -             \
    ((nw_length)(address) + (alignment) - 1U) % (alignment))

c_object nw_participantCreateType(u_participant participant,
                                  const c_char *typeName);
                                  
char *   nw_stringDup(const char *string); 
char *   nw_dumpToString(void *data, unsigned int length);
      

#endif /* NW_MISC_H */

