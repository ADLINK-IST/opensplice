/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef D__TABLE_H
#define D__TABLE_H

#include "d__types.h"
#include "ut_avl.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_table validity.
 * Because d_table is a concrete class typechecking is required.
 */
#define             d_tableIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_TABLE)

/**
 * \brief The d_table cast macro.
 *
 * This macro casts an object to a d_table object.
 */
#define d_table(_this) ((d_table)(_this))

C_STRUCT(d_tableNode) {
    ut_avlNode_t avlnode;
    void *object;
};
C_CLASS(d_tableNode);

C_STRUCT(d_table) {
    C_EXTENDS(d_object);
    void ( *   cleanAction) (); /**< the user's cleanup action             */
    ut_avlCTreedef_t td;
    ut_avlCTree_t tree;
};


typedef struct {
    ut_avlCIter_t it;
} d_tableIter;


d_table                 d_tableNew              (int ( * compare )(),
                                                 void ( * cleanAction )() );

void                    d_tableFree             (d_table table);

void                    d_tableDeinit           (d_table table);

/** returns zero if the entry is added */
c_voidp                 d_tableInsert           (d_table table,
                                                 c_voidp object );

/** returns non-zero (the data) if the entry is removed */
c_voidp                 d_tableRemove           (d_table table,
                                                 c_voidp arg);

/** returns non-zero (the data) if the entry is taken */
c_voidp                 d_tableTake             (d_table table);

/** returns non-zero (the data) if the entry is found */
c_voidp                 d_tableFind             (d_table table,
                                                 c_voidp arg );

/** returns non-zero (the data) if the entry is found */
/** returns non-zero (the data) if an entry is found */
c_voidp                 d_tableFirst            (d_table table);

/** returns non-zero (the data) if an entry is taken */
c_bool                  d_tableWalk             (d_table table,
                                                 c_bool ( * action ) (),
                                                 c_voidp userData);

c_ulong                 d_tableSize             (d_table table);

/** Iterator for d_table */
void *                  d_tableIterFirst        (d_table table,
                                                 d_tableIter *iter);

void *                  d_tableIterNext         (d_tableIter *iter);


#if defined (__cplusplus)
}
#endif

#endif /* D__TABLE_H */
