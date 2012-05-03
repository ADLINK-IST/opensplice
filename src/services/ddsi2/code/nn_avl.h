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
#ifndef NN_AVL_H
#define NN_AVL_H

#include "os_defs.h"

typedef struct { char *addr; } avlparent_t;

#define AVLWALK_CONTINUE 0
#define AVLWALK_ABORT 1
#define AVLWALK_DELETE 2

typedef int (*avlcompare_fun_t) (const void *a, const void *b);
typedef void (*avlaugment_fun_t) (void *node);
typedef void (*nodefree_fun_t) (void *node);
typedef int (*avlwalk_fun_t) (void *node, void *arg);

#define STRUCT_AVLNODE(ntname, type) struct ntname {    \
    type left;                                  \
    type right;                                 \
    type parent;                                \
    int height;                                 \
  }

#define STRUCT_AVLTREE(ntname, type) struct ntname {    \
    type root;                                          \
    os_size_t avlnodeoffset;                            \
    os_size_t keyoffset;                                        \
    avlcompare_fun_t comparekk;                         \
    avlaugment_fun_t augment;                           \
    nodefree_fun_t nodefree;                            \
  }

void avl_init (void *vtree, os_size_t avlnodeoffset, os_size_t keyoffset, avlcompare_fun_t comparekk, avlaugment_fun_t augment, nodefree_fun_t nodefree);
void avl_init_indkey (void *vtree, os_size_t avlnodeoffset, os_size_t keyoffset, avlcompare_fun_t comparekk, avlaugment_fun_t augment, nodefree_fun_t nodefree);
void avl_free (void *vtree);
void *avl_lookup (const void *vtree, const void *key, avlparent_t *parent);
void *avl_lookup_predeq (const void *vtree, const void *key);
void *avl_lookup_succeq (const void *vtree, const void *key);
void avl_init_node (void *vnode, avlparent_t parent);
void avl_insert (void *vtree, void *vnode);
void avl_delete (void *vtree, void *vnode);
int avl_empty (const void *vtree);
void *avl_findmin (void *vtree);
void *avl_findmax (void *vtree);
void *avl_findpred (void *vtree, void *vnode);
void *avl_findsucc (void *vtree, void *vnode);
void avl_walk (void *vtree, avlwalk_fun_t f, void *a);
void avl_walkrange (void *vtree, const void *min, const void *max, avlwalk_fun_t f, void *a);
void avl_augment_update (void *vtree, void *vnode);
void avl_parent_from_node (const void *tree, const void *node, avlparent_t *parent);

#endif /* NN_AVL_H */
