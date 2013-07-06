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
#ifndef IDL_MAP_H
#define IDL_MAP_H

#include "c_typebase.h"

#define idl_map(o)		((idl_map)(o))
C_CLASS(idl_map);

#define idl_mapIter(o)		((idl_mapIter)(o))
C_CLASS(idl_mapIter);

typedef enum idl_equality {
    IDL_PL = -4,    /* Partial less (structure)    */
    IDL_EL = -3,    /* Less or Equal (set)         */
    IDL_LE = -2,    /* Less or Equal               */
    IDL_LT = -1,    /* Less                        */
    IDL_EQ = 0,     /* Equal                       */
    IDL_GT = 1,     /* Greater                     */
    IDL_GE = 2,     /* Greater or Equal            */
    IDL_EG = 3,     /* Greater or Equal (set)      */
    IDL_PG = 4,     /* Partial greater (structure) */
    IDL_PE = 10,    /* Partial Equal               */
    IDL_NE = 20,    /* Not equal                   */
    IDL_ER = 99     /* Error: equality undefined   */
} idl_equality;

idl_map
idl_mapNew (
    idl_equality (*compare)(),
    int free_key,
    int free_object
    );

void
idl_mapFree (
    idl_map map
    );

int
idl_mapAdd (
    idl_map map,
    void *key,
    void *object
    );

void
idl_mapRemove (
    idl_map map,
    void *key
    );

int
idl_mapIsEmpty (
    idl_map map
    );

idl_mapIter
idl_mapFind (
    idl_map map,
    const void *key
    );

idl_mapIter
idl_mapFirst (
    idl_map map
    );

idl_mapIter
idl_mapLast (
    idl_map map
    );

idl_mapIter
idl_mapIterNew (
    void
    );

void
idl_mapIterFree (
    idl_mapIter mapIter
    );

long
idl_mapIterNext (
    idl_mapIter mapIter
    );

long
idl_mapIterPrev (
    idl_mapIter mapIter
    );

long
idl_mapIterRemove (
    idl_mapIter mapIter
    );

void *
idl_mapIterKey (
    idl_mapIter mapIter
    );

void *
idl_mapIterObject (
    idl_mapIter mapIter
    );

long
idl_mapIterSize (
    idl_mapIter mapIter
    );

#endif /* IDL_MAP_H */
