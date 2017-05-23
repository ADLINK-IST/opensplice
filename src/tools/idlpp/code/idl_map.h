/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

int
idl_mapIterNext (
    idl_mapIter mapIter
    );

int
idl_mapIterPrev (
    idl_mapIter mapIter
    );

int
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

os_uint32
idl_mapIterSize (
    idl_mapIter mapIter
    );

#endif /* IDL_MAP_H */
