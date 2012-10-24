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
#ifndef UT_AVLTREE_H
#define UT_AVLTREE_H

#include "os_defs.h"
#include "os_classbase.h"
#include "os_iterator.h"

#define ut_avlTree(tree) ((ut_avlTree)(tree))

typedef enum ut_fixType {
    UT_PREFIX,
    UT_INFIX,
    UT_POSTFIX
} ut_fixType;

OS_CLASS(ut_avlTree);

ut_avlTree ut_avlTreeNew   (os_uint32 offset);
void       ut_avlTreeFree  (ut_avlTree tree);
os_uint32  ut_avlTreeCount (ut_avlTree tree);
void      *ut_avlTreeFirst (ut_avlTree tree);
void      *ut_avlTreeLast  (ut_avlTree tree);
 
void      *ut_avlTreeInsert(ut_avlTree   tree,
                            void         *node,
                            os_equality (*compareFunction)(),
                            void         *compareArgument);

void     *ut_avlTreeReplace (
               ut_avlTree    tree,
               void         *node,
               os_equality (*compareFunction)(),
               void         *compareArgument,
               os_int32    (*condition)(),
               void         *conditionArgument);

void     *ut_avlTreeRemove (
               ut_avlTree    tree,
               void         *template,
               os_equality (*compareFunction)(),
               void         *compareArgument,
               os_int32    (*condition)(),
               void         *conditionArgument);

void     *ut_avlTreeFind(
               ut_avlTree    tree,
               void         *template,
               os_equality (*compareFunction)(),
               void         *compareArgument);

void     *ut_avlTreeNearest (
               ut_avlTree    tree,
               void         *template,
               os_equality (*compareFunction)(),
               void         *compareArgument,
               os_equality   specifier);

os_int32  ut_avlTreeWalk(
               ut_avlTree   tree,
               os_int32   (*action) (),
               void        *actionArgument,
               ut_fixType   fix);

os_int32  ut_avlTreeRangeWalk(
               ut_avlTree    tree,
               void         *startTemplate,
               os_int32      startInclude,
               void         *endTemplate,
               os_int32      endInclude,
               os_equality (*compareFunction)(),
               void         *compareArgument,
               os_int32    (*action) (),
               void         *actionArgument,
               ut_fixType    fix);

#endif /* UT_AVLTREE_H */
