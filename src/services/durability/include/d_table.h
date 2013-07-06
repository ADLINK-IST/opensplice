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
#ifndef D_TABLE_H
#define D_TABLE_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_table(t) ((d_table)(t))

d_table d_tableNew    ( int ( * compare )(), void ( * cleanAction )() );
void    d_tableFree   ( d_table table );
void    d_tableDeinit (d_object object);
/** returns zero if the entry is added */
c_voidp d_tableInsert ( d_table table, c_voidp object );
/** returns non-zero (the data) if the entry is removed */
c_voidp d_tableRemove ( d_table table, c_voidp arg );
/** returns non-zero (the data) if the entry is taken */
c_voidp d_tableTake   ( d_table table );
/** returns non-zero (the data) if the entry is found */
c_voidp d_tableFind   ( d_table table, c_voidp arg );
/** returns non-zero (the data) if the entry is found */
/** returns non-zero (the data) if an entry is found */
c_voidp d_tableFirst  ( d_table table );
/** returns non-zero (the data) if an entry is taken */

c_bool  d_tableWalk   ( d_table table, c_bool ( * action ) (), c_voidp userData );
c_ulong d_tableSize   ( d_table table );


#if defined (__cplusplus)
}
#endif

#endif /* D_TABLE_H */
