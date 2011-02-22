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
#ifndef NW_STRINGLIST_H
#define NW_STRINGLIST_H

typedef struct nw_stringList_s *nw_stringList;

nw_stringList nw_stringListNew     (const char *string,
		                            const char *separators);

void          nw_stringListFree    (nw_stringList this);

unsigned int  nw_stringListGetSize (nw_stringList this);

const char *  nw_stringListGetValue(nw_stringList this,
		                            unsigned int index);

#endif
