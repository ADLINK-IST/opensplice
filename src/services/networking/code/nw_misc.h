/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

/*
#define NW_ALIGN(alignment, address)                       \
    ((os_address)(address) + (alignment) - 1U -             \
    ((os_address)(address) + (alignment) - 1U) % (alignment))
*/
#define NW_ALIGN(alignment,address) 						\
	(((os_address)(address) + (alignment) - 1U) & (~( (os_address) ((alignment)- 1U ) ) ))

c_object nw_participantCreateType(u_participant participant,
                                  const c_char *typeName);
                                  
char *   nw_stringDup(const char *string); 
char *   nw_dumpToString(void *data, os_uint32 length);
      

#endif /* NW_MISC_H */

