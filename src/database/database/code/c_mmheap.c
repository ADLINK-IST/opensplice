/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

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
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "os_defs.h"
#include "os_stdlib.h"
#include "os_signature.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "c_sync.h"

#include "c_mmheap.h"

#if HAVE_VALGRIND
#include "memcheck.h"
#endif

/* A classical eager-coalescing best-fit heap with internal boundary
 * tags.  For 1 and 2 unit-sized free blocks, it maintains free lists;
 * any larger free block is in an AVL-tree sorted on size, with each
 * node a doubly-linked list of free blocks of that size.
 *
 * A heap may consist of multiple regions, the free blocks
 * administration is unified.
 */

#define PRINTF_PTR(p)   ((int) (2 * sizeof (p))), ((os_address) (p))

#define XA(x, a)        (((x) + (a) - 1) & (~ (os_address) ((a) - 1)))
#define ALIGNMENT       XA (sizeof (struct c_mmheap_allocated), 8)
#define HDR_SIZE        (A (sizeof (struct c_mmheap_allocated)))
#define A(x)            XA ((x), ALIGNMENT)

#define SPLIT_CONST     (1 * ALIGNMENT)
#define FREE1_SIZE      (1 * ALIGNMENT)
#define FREE2_SIZE      (2 * ALIGNMENT)
#define MIN_TREE_SIZE   (3 * ALIGNMENT)

#define USED            ((os_address)0x1) /* 0 for free blocks, 1 for used ones */
#define CHECK           ((os_address)0x2) /* 0 during normal operations, temporarily 1 for free blocks during a heap check */
#define ZERO_FLAG       ((os_address)0x4) /* assumed to be 0 */
#define FLAGS           ((os_address)0x7)

#define GET_SIZE(f)     ((f)->size_and_flags & ~FLAGS)
#define GET_USED(f)     ((f)->size_and_flags & USED)
#define GET_CHECK(f)    ((f)->size_and_flags & CHECK)
#define GET_NEXT(b)     ((struct c_mmheap_tree *) ((char *) (b) + HDR_SIZE + GET_SIZE (b)))
#define IS_MARKER(b)    (GET_SIZE (b) == 0) /* for check_heap () */

#define HEIGHTOF(tree)  ((tree) == NULL ? 0 : (tree)->height)

struct c_mmheap_tree {
    os_address size_and_flags;
    struct c_mmheap_tree *prev;
    struct c_mmheap_tree *left;
    struct c_mmheap_tree *right;
    struct c_mmheap_tree *parent;
    struct c_mmheap_tree *list;
    int height;
};

struct c_mmheap_list {
    os_address size_and_flags;
    struct c_mmheap_tree *prev;
    struct c_mmheap_list *left, *right;
};

struct c_mmheap_allocated {
    os_address size_and_flags;
    struct c_mmheap_tree *prev;
};

#if __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

static void c_mmheap_breakpoint (struct c_mmheap *heap UNUSED)
{
    fprintf (stderr, "heap corruption detected by check_heap ()\n");
}

static int invalid_addr (struct c_mmheap *heap, void *addr)
{
    struct c_mmheap_region *hr = &heap->heap_region;
    while (hr) {
        if ((char *) addr >= (char *) hr->base + hr->off &&
            (char *) addr <  (char *) hr->base + hr->size) {
            return 0;
        }
        hr = hr->next;
    }
    return 1;
}

static int check_list (struct c_mmheap *heap, struct c_mmheap_list *node, struct c_mmheap_list *parent, os_address *n, os_address *amount, int indent, int dump_heap)
{
    int r = 0;
    for (; node; node = node->right) {
        if (invalid_addr (heap, node)) {
            fprintf (stderr, "check_list: illegal node address\n");
            return 1;
        }
        if (dump_heap) {
            fprintf (stderr, "%u%*s %0*"PA_PRIxADDR"  %c%c%c  %8"PA_PRIuADDR"<-%0*"PA_PRIxADDR"  l%0*"PA_PRIxADDR"  r%0*"PA_PRIxADDR"\n",
                     heap->heap_check_serial, indent, "", PRINTF_PTR (node),
                     GET_USED (node) ? 'u' : 'f', GET_CHECK (node) ? 'c' : ' ', ' ',
                     GET_SIZE (node),
                     PRINTF_PTR (node->prev), PRINTF_PTR (node->left), PRINTF_PTR (node->right));
        }

        if (GET_SIZE (node) != FREE1_SIZE && GET_SIZE (node) != FREE2_SIZE) {
            fprintf (stderr, "check_list: non-[12]*align byte block in simple list\n");
            r = 1;
        }
        if (parent) {
            if (GET_SIZE (node) != GET_SIZE (parent)) {
                fprintf (stderr, "check_list: list nodes have different sizes\n");
                r = 1;
            }
        }
        if (GET_USED (node)) {
            fprintf (stderr, "check_list: free node not marked as free\n");
            r = 1;
        }
        if (IS_MARKER (node)) {
            fprintf (stderr, "check_list: marker in list\n");
            r = 1;
        }
        if (parent != node->left) {
            fprintf (stderr, "check_list: parents don't match\n");
            r = 1;
        }
        if (GET_CHECK (node)) {
            fprintf (stderr, "check_list: check bit set\n");
            r = 1;
        }

        node->size_and_flags |= CHECK;
        (*n)++;
        *amount += GET_SIZE (node);
        parent = node;
    }
    return r;
}

static int check_node_list (struct c_mmheap *heap, struct c_mmheap_tree *node, struct c_mmheap_tree *parent, os_address *n, os_address *amount, int indent, int dump_heap)
{
    int r = 0;
    for (; node; node = node->list) {
        if (invalid_addr (heap, node)) {
            fprintf (stderr, "check_node_list: illegal node address\n");
            return 1;
        }
        if (dump_heap) {
            fprintf (stderr, "%u %*s+%0*"PA_PRIxADDR" %c%c%c  %8"PA_PRIuADDR"  <-%0*"PA_PRIxADDR"  ^%0*"PA_PRIxADDR"  ->%0*"PA_PRIxADDR"\n",
                     heap->heap_check_serial, indent, "",
                     PRINTF_PTR (node), GET_USED (node) ? 'u' : 'f', GET_CHECK (node) ? 'c' : ' ', ' ',
                     GET_SIZE (node),
                     PRINTF_PTR (node->prev), PRINTF_PTR (node->parent), PRINTF_PTR (node->list));
        }

        if (GET_SIZE (node) == FREE1_SIZE || GET_SIZE (node) == FREE2_SIZE) {
            fprintf (stderr, "check_node_list: [12]*align byte block in tree list\n");
            r = 1;
        }
        if (GET_SIZE (node) != GET_SIZE (parent)) {
            fprintf (stderr, "check_node_list: list nodes have different sizes\n");
            r = 1;
        }
        if (GET_USED (node)) {
            fprintf (stderr, "check_node_list: free node not marked as free\n");
            r = 1;
        }
        if (IS_MARKER (node)) {
            fprintf (stderr, "check_node_list: marker in node list\n");
            r = 1;
        }
        if (parent != node->parent) {
            fprintf (stderr, "check_node_list: parents don't match\n");
            r = 1;
        }
        if (GET_CHECK (node)) {
            fprintf (stderr, "check_node_list: check bit set\n");
            r = 1;
        }

        node->size_and_flags |= CHECK;
        (*n)++;
        *amount += GET_SIZE (node);
        parent = node;
    }
    return r;
}

static int check_node (struct c_mmheap *heap, struct c_mmheap_tree *node, struct c_mmheap_tree *parent, os_address *n, os_address *nn, os_address *amount, int indent, int dump_heap)
{
    int r = 0;

    if (node == NULL) {
        return 0;
    }
    if (invalid_addr (heap, node)) {
        fprintf (stderr, "check_node: illegal node address\n");
        return 1;
    }

    if (dump_heap) {
        fprintf (stderr, "%u %*s%0*"PA_PRIxADDR"  %c%c%c  %8"PA_PRIuADDR"  " "<-%0*"PA_PRIxADDR"  l%0*"PA_PRIxADDR"  r%0*"PA_PRIxADDR"  " "^%0*"PA_PRIxADDR"  ->%0*"PA_PRIxADDR"  h%d\n",
                 heap->heap_check_serial, indent, "", PRINTF_PTR (node),
                 GET_USED (node) ? 'u' : 'f', GET_CHECK (node) ? 'c' : ' ', ' ',
                 GET_SIZE (node),
                 PRINTF_PTR (node->prev), PRINTF_PTR (node->left), PRINTF_PTR (node->right),
                 PRINTF_PTR (node->parent), PRINTF_PTR (node->list),
                 node->height);
    }

    if (parent &&
        ((GET_SIZE (node) < GET_SIZE (parent) && node != parent->left) ||
         (GET_SIZE (node) > GET_SIZE (parent) && node != parent->right))) {
        fprintf (stderr, "check_node: on wrong side of parent\n");
        r++;
    }
    if (node->height - 1 != (HEIGHTOF (node->left) > HEIGHTOF (node->right) ?
                             HEIGHTOF (node->left) : HEIGHTOF (node->right))) {
        fprintf (stderr, "check_node: height of node %p wrong wrt children\n", (void *) node);
        r++;
    }
    if (HEIGHTOF (node->left) - 1 > HEIGHTOF (node->right) ||
        HEIGHTOF (node->left) + 1 < HEIGHTOF (node->right)) {
        fprintf (stderr, "check_node: imbalance rooted at node %p\n", (void *) node);
        r++;
    }
    if (GET_SIZE (node) == FREE1_SIZE || GET_SIZE (node) == FREE2_SIZE) {
        fprintf (stderr, "check_node: [12]*align byte block in tree\n");
        r++;
    }
    if (GET_USED (node)) {
        fprintf (stderr, "check_node: free node not marked as free\n");
        r++;
    }
    if (IS_MARKER (node)) {
        fprintf (stderr, "check_node: marker in tree\n");
        r++;
    }
    if (parent != node->parent) {
        fprintf (stderr, "check_node: parents don't match\n");
        r++;
    }
    if (GET_CHECK (node)) {
        fprintf (stderr, "check_node: check bit set\n");
        r++;
    }

    node->size_and_flags |= CHECK;
    (*n)++;
    (*nn)++;
    *amount += GET_SIZE (node);

    r += check_node_list (heap, node->list, node, n, amount, indent, dump_heap);
    r += check_node (heap, node->left, node, n, nn, amount, indent + 2, dump_heap);
    r += check_node (heap, node->right, node, n, nn, amount, indent + 2, dump_heap);
    return r;
}

static int check_heap (struct c_mmheap *heap, char *call, os_address p0, os_address p1, int lock, int dump_heap)
{
    int r = 0;
    os_address nfree;
    os_address afree;
    struct c_mmheap_allocated *hdr, *phdr;
    struct c_mmheap_region *hr;

    if (lock) {
        os_mutexLock (&heap->lock);
    }

    heap->heap_check_serial++;

    if (dump_heap && call) {
        fprintf (stderr, "%u ---- %s (%"PA_PRIuADDR" 0x%"PA_PRIxADDR", %"PA_PRIuADDR" 0x%"PA_PRIxADDR") start of heap\n",
                 heap->heap_check_serial, call, p0, p0, p1, p1);
    }

    nfree = 0;
    afree = 0;

    if (dump_heap) {
        fprintf (stderr, "%u ---- free %"PA_PRIdADDR" byte blocks list:\n", heap->heap_check_serial, FREE1_SIZE);
    }
    if (heap == NULL || (heap->free1 && invalid_addr (heap, heap->free1))) {
        fprintf (stderr, "check_heap: free %"PA_PRIdADDR" byte blocks list has illegal address\n", FREE1_SIZE);
        r++;
    } else {
        r += check_list (heap, heap->free1, 0, &nfree, &afree, 0, dump_heap);
    }

    if (dump_heap) {
        fprintf (stderr, "%u ---- free %"PA_PRIdADDR" byte blocks list:\n", heap->heap_check_serial, FREE2_SIZE);
    }
    if (heap == NULL || (heap->free2 && invalid_addr (heap, heap->free2))) {
        fprintf (stderr, "check_heap: free %"PA_PRIdADDR" byte blocks list has illegal address\n", FREE2_SIZE);
        r++;
    } else {
        r += check_list (heap, heap->free2, 0, &nfree, &afree, 0, dump_heap);
    }

    if (dump_heap) {
        fprintf (stderr, "%u ---- tree:\n", heap->heap_check_serial);
    }
    if (heap == NULL || (heap->free && invalid_addr (heap, heap->free))) {
        fprintf (stderr, "check_heap: free blocks tree has illegal address\n");
        r++;
    } else {
        os_address tree_n, node_n;

        tree_n = (os_address) (-(os_saddress)nfree);
        node_n = 0;
        r += check_node (heap, heap->free, 0, &nfree, &node_n, &afree, 0, dump_heap);
        tree_n += nfree;

        if (dump_heap) {
            fprintf (stderr, "%u ---- free: tree: blocks: %"PA_PRIdADDR" block%s (%"PA_PRIdADDR" key%s)\n",
                     heap->heap_check_serial, tree_n, tree_n != 1 ? "s" : "", node_n, node_n != 1 ? "s" : "");
            fprintf (stderr, "%u ---- free: blocks adm: %"PA_PRIdADDR" blocks, heap adm: %"PA_PRIdADDR" blocks\n",
                     heap->heap_check_serial, nfree, heap->n_free_blocks);
            fprintf (stderr, "%u ---- free: blocks adm: %"PA_PRIdADDR" bytes, heap adm: %"PA_PRIdADDR" bytes\n",
                     heap->heap_check_serial, afree, heap->n_free_bytes);
        }
        if (nfree != heap->n_free_blocks) {
            fprintf (stderr, "check_heap: heap->n_free_blocks is incorrect\n");
            r++;
        }
        if (afree != heap->n_free_bytes) {
            fprintf (stderr, "check_heap: heap->n_free_bytes is incorrect\n");
            r++;
        }
    }

    if (dump_heap > 1) {
        fprintf (stderr, "%u ---- linear:\n", heap->heap_check_serial);
    }

    for (hr = &heap->heap_region; hr; hr = hr->next) {
        phdr = NULL;
        hdr = (struct c_mmheap_allocated *) ((char *) hr->base + hr->off);
        while ((char *) hdr < ((char *) hr->base + hr->size)) {
            if (hdr <  (struct c_mmheap_allocated *) ((char *) hr->base + hr->off) ||
                hdr >= (struct c_mmheap_allocated *) ((char *) hr->base + hr->size)) {
                fprintf (stderr, "check_heap: block address illegal\n");
                if (lock) {
                    os_mutexUnlock (&heap->lock);
                }
                return 1;
            }

            if (dump_heap > 1) {
                fprintf (stderr, "%u %0*"PA_PRIxADDR"  %c%c%c  %8"PA_PRIuADDR"  <-%0*"PA_PRIxADDR"\n",
                         heap->heap_check_serial,
                         PRINTF_PTR (hdr),
                         GET_USED (hdr) ? 'u' : 'f',
                         GET_CHECK (hdr) ? 'c' : ' ',
                         ' ',
                         GET_SIZE (hdr),
                         PRINTF_PTR (hdr->prev));
            }

            if (!GET_USED (hdr)) {
                nfree--;
            }

            if (!GET_USED (hdr) && !GET_CHECK (hdr)) {
                fprintf (stderr, "check_heap: %p marked as free but not in tree\n", (void *) hdr);
                r++;
            }
            if (!GET_USED (hdr) && IS_MARKER (hdr)) {
                fprintf (stderr, "check_heap: %p marker has free bit set\n", (void *) hdr);
                r++;
            }
            if ((IS_MARKER (hdr) != 0) !=
                (((char *) hdr == (char *) hr->base + hr->off) ||
                 ((char *) hdr + HDR_SIZE == (char *) hr->base + hr->size))) {
                if (IS_MARKER (hdr)) {
                    fprintf (stderr, "check_heap: %p marker not at heap boundary\n", (void *) hdr);
                } else {
                    fprintf (stderr, "check_heap: %p non-marker at heap boundary\n", (void *) hdr);
                }
                r++;
            }
            if ((char *) hdr + HDR_SIZE + GET_SIZE (hdr) > (char *) hr->base + hr->size) {
                fprintf (stderr, "check_heap: %p block ends beyond over heap boundary\n", (void *) hdr);
                r++;
            }
            if (hdr->prev != (struct c_mmheap_tree *) phdr) {
                fprintf (stderr, "check_heap: %p prev field wrong (%p expected)\n", (void *) hdr, (void *) phdr);
                r++;
            }
            if (!GET_USED (hdr) && hdr->prev && !GET_USED (hdr->prev)) {
                fprintf (stderr, "check_heap: %p free blocks (%p & %p) not merged\n", (void *) hdr, (void *) phdr, (void *) hdr);
                r++;
            }
            if (hdr->size_and_flags & ZERO_FLAG) {
                fprintf (stderr, "check_heap: %p bit 2 is set\n", (void *) hdr);
                r++;
            }

            hdr->size_and_flags &= ~CHECK;

            phdr = hdr;
            hdr = (struct c_mmheap_allocated *) GET_NEXT (hdr);
        }
    }
    if (nfree != 0) {
        fprintf (stderr, "check_heap: free count doesn't match " "number of nodes (%"PA_PRIuADDR")\n", nfree);
        r++;
    }

    if (dump_heap) {
        fprintf (stderr, "%u ---- end of heap\n", heap->heap_check_serial);
    }

    if (lock) {
        os_mutexUnlock (&heap->lock);
    }

    return r;
}

static struct c_mmheap_tree *tree_rebalance_one (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    struct c_mmheap_tree *noderight = node->right;
    struct c_mmheap_tree *nodeleft = node->left;
    os_int heightright = HEIGHTOF (noderight);
    os_int heightleft = HEIGHTOF (nodeleft);

    if (heightright + 1 < heightleft) {
        struct c_mmheap_tree *nodeleftleft, *nodeleftright;
        int heightleftright;

        assert(nodeleft);
        nodeleftleft = nodeleft->left;
        nodeleftright = nodeleft->right;
        heightleftright = HEIGHTOF (nodeleftright);

        if (HEIGHTOF (nodeleftleft) >= heightleftright) {
            node->left = nodeleft->right;
            nodeleft->parent = node->parent;
            node->parent = nodeleft;
            if (nodeleftright) {
                nodeleftright->parent = node;
            }
            nodeleft->right = node;
            nodeleft->height = 1 + (node->height = 1 + heightleftright);
            if (nodeleft->parent == NULL) {
                heap->free = nodeleft;
            } else if (node == nodeleft->parent->left) {
                nodeleft->parent->left = nodeleft;
            } else {
                nodeleft->parent->right = nodeleft;
            }
            return nodeleft->parent;
        } else {
            assert(nodeleftright);
            nodeleftright->parent = node->parent;

            nodeleft->right = nodeleftright->left;
            if (nodeleft->right) {
                nodeleft->right->parent = nodeleft;
            }
            node->left = nodeleftright->right;
            if (node->left) {
                node->left->parent = node;
            }
            nodeleftright->left = nodeleft;
            nodeleft->parent = nodeleftright;

            nodeleftright->right = node;
            node->parent = nodeleftright;

            nodeleft->height = node->height = heightleftright;
            nodeleftright->height = heightleft;

            if (nodeleftright->parent == NULL) {
                heap->free = nodeleftright;
            } else if (node == nodeleftright->parent->left) {
                nodeleftright->parent->left = nodeleftright;
            } else {
                nodeleftright->parent->right = nodeleftright;
            }
            return nodeleftright->parent;
        }
    } else if (heightleft + 1 < heightright) {
        struct c_mmheap_tree *noderightright, *noderightleft;
        int heightrightleft;

        assert(noderight);
        noderightright = noderight->right;
        noderightleft = noderight->left;
        heightrightleft = HEIGHTOF (noderightleft);

        if (HEIGHTOF (noderightright) >= heightrightleft) {
            node->right = noderight->left;
            noderight->parent = node->parent;
            node->parent = noderight;
            if (noderightleft) {
                noderightleft->parent = node;
            }
            noderight->left = node;
            noderight->height = 1 + (node->height = 1 + heightrightleft);

            if (noderight->parent == NULL) {
                heap->free = noderight;
            } else if (node == noderight->parent->left) {
                noderight->parent->left = noderight;
            } else {
                noderight->parent->right = noderight;
            }
            return noderight->parent;
        } else {
            assert(noderightleft);
            noderightleft->parent = node->parent;

            noderight->left = noderightleft->right;
            if (noderight->left) {
                noderight->left->parent = noderight;
            }
            node->right = noderightleft->left;
            if (node->right) {
                node->right->parent = node;
            }
            noderightleft->right = noderight;
            noderight->parent = noderightleft;

            noderightleft->left = node;
            node->parent = noderightleft;

            noderight->height = node->height = heightrightleft;
            noderightleft->height = heightright;

            if (noderightleft->parent == NULL) {
                heap->free = noderightleft;
            } else if (node == noderightleft->parent->left) {
                noderightleft->parent->left = noderightleft;
            } else {
                noderightleft->parent->right = noderightleft;
            }
            return noderightleft->parent;
        }
    } else {
        int height = (heightleft < heightright ? heightright : heightleft) + 1;
        if (height == node->height) {
            return NULL;
        } else {
            node->height = height;
            return node->parent;
        }
    }
}

static void tree_rebalance (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    while (node) {
        node = tree_rebalance_one (heap, node);
    }
}

static void tree_insert (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    os_address node_size;
    struct c_mmheap_tree *tmp, *prev;

    if (heap->free == NULL) {
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->list = NULL;
        node->height = 1;
        heap->free = node;
        return;
    }

    tmp = heap->free;
    prev = NULL;
    node_size = node->size_and_flags;
    while (tmp && node_size != tmp->size_and_flags) {
        prev = tmp;
        if (node_size < tmp->size_and_flags) {
            tmp = tmp->left;
        } else {
            tmp = tmp->right;
        }
    }

    if (tmp) {
        /* add to list */
        node->parent = tmp;
        node->list = tmp->list;
        tmp->list = node;
        if (node->list) {
            /* already a list, update (3rd node)->parent */
            node->list->parent = node;
        }
    } else {
        /* add the node to the tree */
        node->left = NULL;
        node->right = NULL;
        node->list = NULL;
        node->height = 1;
        if (node_size < prev->size_and_flags) {
            prev->left = node;
        } else {
            prev->right = node;
        }
        node->parent = prev;
        tree_rebalance (heap, prev);
    }
}

static void tree_delete (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    struct c_mmheap_tree *parent = node->parent, *largest, *rebalance_start;

    if (node->left == NULL) {
        struct c_mmheap_tree *right = node->right;
        if (parent == NULL) {
            heap->free = right;
        } else if (node == parent->left) {
            parent->left = right;
        } else {
            parent->right = right;
        }
        if (right) {
            right->parent = node->parent;
        }
        if (parent) {
            tree_rebalance (heap, parent);
        }
        return;
    }

    if (node->right == NULL) {
        struct c_mmheap_tree *left = node->left;
        if (parent == NULL) {
            heap->free = left;
        } else if (node == parent->left) {
            parent->left = left;
        } else {
            parent->right = left;
        }
        left->parent = node->parent;
        if (parent) {
            tree_rebalance (heap, parent);
        }
        return;
    }

    /* Search the left orphaned subtree for the largest value it
     * contains. This will necessarily be smaller than the deleted
     * node, and all the nodes in the right subtree.
     */
    largest = node->left;
    if (largest->right == NULL) {
        largest->parent = node->parent;
        largest->right = node->right;
        largest->height = node->height;
        if (largest->right)
            largest->right->parent = largest;

        rebalance_start = largest;
    } else {
        while (largest->right) {
            largest = largest->right;
        }
        rebalance_start = largest->parent;
        rebalance_start->right = largest->left;
        if (largest->left) {
            largest->left->parent = rebalance_start;
        }
        largest->right = node->right;
        largest->left = node->left;
        largest->parent = node->parent;
        largest->height = node->height;
        node->left->parent = largest;
        node->right->parent = largest;
    }

    if (parent == NULL) {
        heap->free = largest;
    } else if (node == parent->left) {
        parent->left = largest;
    } else {
        parent->right = largest;
    }

    tree_rebalance (heap, rebalance_start);
}

static void node_insert (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    if (node->size_and_flags >= MIN_TREE_SIZE) {
        tree_insert (heap, node);
    } else if (node->size_and_flags >= FREE2_SIZE) {
        struct c_mmheap_list *lnode = (struct c_mmheap_list *) node;
        lnode->left = NULL;
        lnode->right = heap->free2;
        if (heap->free2) {
            heap->free2->left = lnode;
        }
        heap->free2 = lnode;
    } else {
        struct c_mmheap_list *lnode = (struct c_mmheap_list *) node;
        lnode->left = NULL;
        lnode->right = heap->free1;
        if (heap->free1) {
            heap->free1->left = lnode;
        }
        heap->free1 = lnode;
    }
}

static void node_delete (struct c_mmheap *heap, struct c_mmheap_tree *node)
{
    if (node->size_and_flags < FREE2_SIZE) {
        struct c_mmheap_list *n = (struct c_mmheap_list *) node;
        if (n->right) {
            n->right->left = n->left;
        } if (n->left) {
            n->left->right = n->right;
        } else {
            heap->free1 = n->right;
        }
    } else if (node->size_and_flags < MIN_TREE_SIZE) {
        struct c_mmheap_list *n = (struct c_mmheap_list *) node;
        if (n->right) {
            n->right->left = n->left;
        } if (n->left)  {
            n->left->right = n->right;
        } else {
            heap->free2 = n->right;
        }
    }
    else if (node->list != NULL /* for all nodes but the last in a list */ ||
             (node->list == NULL && node->parent != NULL /* for the last node in a list */ &&
              node->parent->list == node)) {
        /* removing a node that is part of the list doesn't really
         * change the structure of the tree: the list will be
         * shortened but the key won't be removed.
         */
        if (node->parent == NULL || node->parent->list != node) {
            /* removing head of list (i.e. second node in list now
             * becomes the head)
             */
            struct c_mmheap_tree *next = node->list;

            next->parent = node->parent;
            next->left = node->left;
            next->right = node->right;
            next->height = node->height;
            if (node->left) {
                node->left->parent = node->list;
            }
            if (node->right) {
                node->right->parent = node->list;
            }
            if (node->parent) {
                struct c_mmheap_tree *parent = node->parent;
                if (node == parent->left) {
                    parent->left = node->list;
                } else {
                    parent->right = node->list;
                }
            } else {
                heap->free = next;
            }
        } else {
            /* removing an element somewhere in the list */
            struct c_mmheap_tree *prev = node->parent;
            prev->list = node->list;
            if (node->list) {
                node->list->parent = node->parent;
            }
        }
    } else {
        tree_delete (heap, node);
    }
}

static struct c_mmheap_tree *block_from_list (struct c_mmheap_tree *block)
{
    /* block is a tree node, and the head of a list blocks of this
     * size, remove the second to speed things up a bit more.  (note
     * that this doesn't alter the tree's structure as far as
     * balancing is concerned.)
     */
    struct c_mmheap_tree *b = block->list;
    block->list = b->list;
    if (b->list) {
        b->list->parent = block;
    }
    return b;
}

static struct c_mmheap_tree *split_block (struct c_mmheap *heap, struct c_mmheap_tree *block, os_address size)
{
    struct c_mmheap_tree *new = (struct c_mmheap_tree *) ((char *) block + HDR_SIZE + size);
    new->size_and_flags = block->size_and_flags - size - HDR_SIZE;
    new->prev = block;
    GET_NEXT (block)->prev = new;
    block->size_and_flags = size | USED;
    heap->n_free_bytes -= size + HDR_SIZE;
    return new;
}

static void *tree_alloc_unsplittable (struct c_mmheap *heap, struct c_mmheap_tree *block)
{
    if (block->list == NULL) {
        tree_delete (heap, block);
    } else {
        block = block_from_list (block);
    }
    heap->n_free_bytes -= block->size_and_flags;
    heap->n_free_blocks--;
    block->size_and_flags |= USED;
    return block;
}

static void *mmheap_malloc_lockheld (struct c_mmheap *heap, os_address size)
{
    struct c_mmheap_tree *root, *block;
    struct c_mmheap_tree *pred;
    void *result;

    if (size == 0) {
        return NULL;
    }

    size = A (size);

    if (heap->check > 2 && check_heap (heap, "pre  c_mmheap_malloc", size, 0, 0, heap->dump)) {
        c_mmheap_breakpoint (heap);
    }

    if (size <= FREE1_SIZE && heap->free1 != NULL) {
        struct c_mmheap_list *b = heap->free1;
        if (b->right) b->right->left = NULL;
        heap->free1 = b->right;
        heap->n_free_bytes -= b->size_and_flags;
        heap->n_free_blocks--;
        b->size_and_flags |= USED;
        result = b;
        goto success;
    }

    if (size <= FREE2_SIZE && heap->free2 != NULL) {
        struct c_mmheap_list *b = heap->free2;
        if (b->right) b->right->left = NULL;
        heap->free2 = b->right;
        heap->n_free_bytes -= b->size_and_flags;
        heap->n_free_blocks--;
        b->size_and_flags |= USED;
        result = b;
        goto success;
    }

    root = heap->free; /* used to walk the tree.  */
    block = NULL; /* best match so far.  */
    pred = NULL; /* cand. largest smaller than block */
    while (root) {
        if (root->size_and_flags > size) { /* larger than necessary */
            block = root, root = root->left;
        } else if (root->size_and_flags < size) { /* too small */
            pred = root, root = root->right;
        } else { /* exact fit: don't think, just do */
            result = tree_alloc_unsplittable (heap, root);
            goto success;
        }
    }

    if (block == NULL) {
        /* bail out on memory exhaustion. */
        goto failure;
    } else if (block->size_and_flags < size + HDR_SIZE + SPLIT_CONST) {
        /* unsplittable blocks are short circuited */
        result = tree_alloc_unsplittable (heap, block);
    } else if (block->list != NULL) {
        /* remove the block from the list, split & reinsert */
        struct c_mmheap_tree *new;
        block = block_from_list (block);
        new = split_block (heap, block, size);
        node_insert (heap, new);
        result = block;
    } else if (block->size_and_flags - size - HDR_SIZE < MIN_TREE_SIZE) {
        /* can't stay in the tree, so remove it, split it and insert the
         * remainer in list1 or list2 depending on its size
         */
        struct c_mmheap_tree *new;
        tree_delete (heap, block);
        new = split_block (heap, block, size);
        node_insert (heap, new);
        result = block;
    } else {
        struct c_mmheap_tree *new =
            (struct c_mmheap_tree *) ((char *) block + HDR_SIZE + size);
        os_address new_size = block->size_and_flags - size - HDR_SIZE;

        /* Note: this modifies size & prev in block, but it doesn't touch
         * the pointers used by tree_delete() which means that postponing
         * removal from the tree is actually possible.
         */
        GET_NEXT (block)->prev = new;
        block->size_and_flags = size | USED;
        heap->n_free_bytes -= size + HDR_SIZE;

        /* This is merely an excellent approximation of the predecessor */
        if (block->left == NULL && (pred == NULL || pred->size_and_flags < new_size)) {
            /* Wow!  In-place substitution is possible - beware though
             * that the fields must be copied in the reverse order of
             * their definition to cope with a allocation of 8 or 16
             * bytes, in which case `new' overlaps part of the tree fields
             * in `block'
             */
            new->height = block->height;
            new->list = NULL;
            new->parent = block->parent;
            new->right = block->right;
            new->left = block->left;
            new->prev = block;
            new->size_and_flags = new_size;
            if (new->left) {
                new->left->parent = new;
            }
            if (new->right) {
                new->right->parent = new;
            }
            if (new->parent == NULL) {
                heap->free = new;
            } else if (new->parent->left == block) {
                new->parent->left = new;
            } else {
                new->parent->right = new;
            }
        } else {
            /* Brute force needed...  We simply don't know where new is going to end up. */
            tree_delete (heap, block);
            new->size_and_flags = new_size;
            new->prev = block;
            tree_insert (heap, new);
        }
        result = block;
    }

success:
    heap->n_allocated_blocks++;
    if (heap->check > 1 && check_heap (heap, "post c_mmheap_malloc", size, 0, 0, heap->dump)) {
        c_mmheap_breakpoint (heap);
    }
    return ((struct c_mmheap_allocated *) result + 1);
failure:
    heap->n_failed_allocations++;
    if (heap->check > 1 && check_heap (heap, "post failed c_mmheap_malloc", size, 0, 0, heap->dump)) {
        c_mmheap_breakpoint (heap);
    }
    return NULL;
}

static void mmheap_free_lockheld (struct c_mmheap *heap, void *ptr)
{
    struct c_mmheap_tree *next, *block;

    if (ptr == NULL) {
        return;
    }

    block = (struct c_mmheap_tree *) ((struct c_mmheap_allocated *) ptr - 1);

    if (heap->check > 2 && check_heap (heap, "pre  c_mmheap_free", (os_address) block, 0, 0, heap->dump)) {
        c_mmheap_breakpoint (heap);
    }
    if (heap->check > 0 && (block->size_and_flags & USED) == 0) {
        fprintf (stderr, "c_mmheap_free (%p, %p): block already freed\n", (void *) heap, (void *) ptr);
    }

    block->size_and_flags &= ~USED;

    next = GET_NEXT (block);
    heap->n_free_bytes += block->size_and_flags;
    heap->n_allocated_blocks--;

    if (! GET_USED ((struct c_mmheap_allocated *) block->prev)) {
        struct c_mmheap_tree *prev = block->prev;
        if (! GET_USED (next)) {
            /* combine prev, block, next */
            node_delete (heap, prev);
            node_delete (heap, next);
            prev->size_and_flags +=
                HDR_SIZE + block->size_and_flags +
                HDR_SIZE + next->size_and_flags;
            GET_NEXT (next)->prev = prev;

            /* two headers reclaimed; block, next disappear, block wasn't
               counted yet.  */
            heap->n_free_bytes += 2 * HDR_SIZE;
            heap->n_free_blocks--;
        } else {
            /* combine prev, block */
            node_delete (heap, prev);
            prev->size_and_flags += HDR_SIZE + block->size_and_flags;
            next->prev = prev;

            /* one header reclaimed; no free blocks added or removed.  */
            heap->n_free_bytes += HDR_SIZE;
        }
        /* prev is now at least size (prev) + sizeof (header) + size
         * (block) which amounts to (>= align) + align + (>= align) = (>=
         * 2*align) bytes large, any block at least 3*align bytes large is
         * stored in the tree and we can thus use tree_insert () without
         * bothering to check whether to block is at least 3*align bytes
         * large or not :-).
         */
        tree_insert (heap, prev);
    } else if (! GET_USED (next)) {
        /* combine block, next */
        node_delete (heap, next);
        block->size_and_flags += HDR_SIZE + next->size_and_flags;
        GET_NEXT (next)->prev = block;
        /* tree_insert () is used instead of node_insert () for the
         * reasons mentioned above in the `prev' case.
         */
        tree_insert (heap, block);

        /* one header reclaimed; no free blocks added or removed.  */
        heap->n_free_bytes += HDR_SIZE;
    } else {
        node_insert (heap, block);

        /* block added, no blocks removed; so increment n_free.  */
        heap->n_free_blocks++;
    }

    if (heap->check > 1 && check_heap (heap, "post c_mmheap_free", (os_address) block, 0, 0, heap->dump)) {
        c_mmheap_breakpoint (heap);
    }
}

static int mmheap_addregion_lockheld (struct c_mmheap *heap, void *block, os_address size) __nonnull_all__;

static int mmheap_addregion_lockheld (struct c_mmheap *heap, void *block, os_address size)
{
    struct c_mmheap_allocated *node, *endm;
    struct c_mmheap_region *hr;
    os_address off;

    off = A ((os_address) block + sizeof (struct c_mmheap_region)) - (os_address) block;
    size = (((os_address) block + size) & (os_address)(-(os_saddress)ALIGNMENT)) - (os_address) block;

    if (size < off ||
        size - off < (HDR_SIZE +  /* begin marker */
                      HDR_SIZE + 3 * HDR_SIZE + /* one tree node */
                      HDR_SIZE)) { /* end marker */
        /* size is too small for a heap */
        return -1;
    }

    /* Two modes: (1) block immediately follows an existing heap
     * block, and (2) it does not.  In case (1), we grow the heap
     * block, in case (2) we add a heap block.  We don't attempt to
     * merge three blocks if we happen to fill a hole between two.
     *
     * Note that in case (1) we don't really have the size & offset
     * requirements that we've already checked, but those amount to a
     * few bytes and one shouldn't be attempting to grow the heap by
     * such small amounts anyway.
     */
    assert (heap != NULL);
    for (hr = &heap->heap_region; hr; hr = hr->next) {
        if ((char *) hr->base + hr->size == (char *) block) {
            break;
        }
    }

    if (hr != NULL) { /* case (1) */
        /* Write an end marker at the end of the new region and
         * rewrite the size of the old end marker so that it becomes a
         * block that occupies all of the space in the new region.
         * Then all we need to do is free the old end marker.
         */
        node = (struct c_mmheap_allocated *) ((char *) hr->base + hr->size - HDR_SIZE);
        assert (node->size_and_flags == (0 | USED));
        node->size_and_flags += size - HDR_SIZE;
        hr->size += size;
    } else { /* case (2) */
        /* Make block look like: HEAPBLOCK BEGM NODE ENDM, where BEGM
         * and ENDM are begin and end markers, and where NODE is an
         * _allocated_ block spanning the bytes between BEGM and
         * ENDM.
         */
        struct c_mmheap_allocated *begm;
        hr = block;
        hr->base = block;
        hr->off = off;
        hr->size = size;
        hr->next = heap->heap_region.next;
        heap->heap_region.next = hr;

        begm = (struct c_mmheap_allocated *) ((char *) block + off);
        begm->size_and_flags = 0 | USED;
        begm->prev = NULL;

        node = (struct c_mmheap_allocated *) ((char *) block + off + HDR_SIZE);
        node->size_and_flags = (size - off - 3 * HDR_SIZE) | USED;
        node->prev = (struct c_mmheap_tree *) begm;
    }

    /* Always need an end marker */
    endm = (struct c_mmheap_allocated *) ((char *) block + size - HDR_SIZE);
    endm->size_and_flags = 0 | USED;
    endm->prev = (struct c_mmheap_tree *) node;

    /* Integrate new block into heap by freeing the node that
     * describes the new region as-if it were a allocated memory
     */
    heap->n_allocated_blocks++;
    mmheap_free_lockheld (heap, (struct c_mmheap_allocated *) node + 1);
    return 0;
}

static void *mmheap_check_pointer_lockheld (struct c_mmheap *heap, void *ptr)
{
    struct c_mmheap_region *hr;
    struct c_mmheap_allocated *hdr;
    /* find the heap block containing the address, excluding the begin
     * and end markers because they could easily be confused for
     * allocated blocks
     */
    for (hr = &heap->heap_region; hr; hr = hr->next) {
        if ((char *) ptr >= (char *) hr->base + hr->off + HDR_SIZE &&
            (char *) ptr < (char *) hr->base + hr->size - HDR_SIZE) {
            break;
        }
    }
    if (hr == NULL) {
        /* address is not contained in any heap block */
        return NULL;
    }
    /* do a linear scan of "hr", ptr is known to be between the begin
     * & end markers, so we'll definitely find the block
     */
    hdr = (struct c_mmheap_allocated *) ((char *) hr->base + hr->off + HDR_SIZE);
    while ((char *) ptr >= (char *) GET_NEXT (hdr)) {
        hdr = (struct c_mmheap_allocated *) GET_NEXT (hdr);
    }
    assert ((char *) ptr >= (char *) hdr);
    assert ((char *) ptr < (char *) GET_NEXT (hdr));
    if ((char *) ptr < (char *) hdr + HDR_SIZE) {
        /* naughty: ptr points into the block header */
        return NULL;
    } else if (! GET_USED (hdr)) {
        /* sloppy: ptr points into unallocated memory */
        return NULL;
    } else {
        return hdr + 1;
    }
}

static void mmheap_dropregion_lockheld (struct c_mmheap *heap, os_address minfree, os_address minsize, os_address align, void (*dropped_cb) (void *arg, void *addr, os_address size), void *arg) __nonnull((1,5));

static void mmheap_dropregion_lockheld (struct c_mmheap *heap, os_address minfree, os_address minsize, os_address align, void (*dropped_cb) (void *arg, void *addr, os_address size), void *arg)
{
    /* For each heap region, removes memory from the top of a heap.
     * Minimum size removed is MINSIZE, start address will be a
     * multiple of ALIGN.  Will not free anything if less than minsize
     * remains.  Returns new top-of-heap address.
     */
    struct c_mmheap_region *hr;
    /* ALIGN must be a power of 2, and must be at least ALIGNMENT */
    assert ((align & (os_address)(-(os_saddress)align)) == align);
    assert (align >= ALIGNMENT);
    assert (heap != NULL);
    /* willing to return free memory, but want to retain at least
     * SPLIT_CONST + ALIGNMENT) so that we only ever have to shrink
     * the last free block.
     */
    if (minfree < SPLIT_CONST + ALIGNMENT) {
        minfree = SPLIT_CONST + ALIGNMENT;
    }
    /* find the heap block containing the address, excluding the begin
     * and end markers because they could easily be confused for
     * allocated blocks
     */
    for (hr = &heap->heap_region; hr; hr = hr->next) {
        struct c_mmheap_allocated *endm;
        struct c_mmheap_tree *lastblock;
        os_address addr, size, remain;
        /* Find header of last block before end marker.  If it's in
         * use there is nothing we can do
         */
        endm = (struct c_mmheap_allocated *) ((char *) hr->base + hr->size - HDR_SIZE);
        lastblock = endm->prev;
        if (GET_USED (lastblock)) {
            continue;
        }
        /* Align up the address of the payload (we need space for the
         * header for the end marker), and see if it would result in
         * freeing at least minsize
         */
        if (minfree + align - 1 > (os_address) endm - (os_address) lastblock) {
            /* minsize + alignment (potentially) pushes address beyond
             * heap region
             */
            continue;
        }
        addr = ((os_address) lastblock + HDR_SIZE + minfree + align - 1) & (os_address)(-(os_saddress)align);
        assert (addr >= (os_address) lastblock + HDR_SIZE + minfree);
        assert (addr <= (os_address) hr->base + hr->size);
        size = (os_address) hr->base + hr->size - addr;
        if (size < minsize) {
            /* not worth the bother */
            continue;
        }
        remain = addr - ((os_address) lastblock + HDR_SIZE);
        assert (remain >= minfree);
        assert ((remain & (ALIGNMENT-1)) == 0);
        /* state: ... [lb hdr] [... lb space 0 ...] ADDR [... lb space 1 ...] [endm]
         *
         * where lb space 0 >= minfree and lb space 1 >= minsize and
         * ADDR is aligned to a multiple of align.
         *
         * remain is (sizeof (lb space 0))
         * size is (sizeof (lb space 1) + sizeof (endm))
         *
         * Allocate lastblock, update its size, rewrite region, write
         * new end marker, free lastblock
         */
        node_delete (heap, lastblock);
        heap->n_free_bytes -= lastblock->size_and_flags;
        heap->n_free_blocks--;
        lastblock->size_and_flags = (remain - HDR_SIZE) | USED;
        hr->size = addr - (os_address) hr->base;
        endm = (struct c_mmheap_allocated *) (addr - HDR_SIZE);
        endm->prev = lastblock;
        endm->size_and_flags = (0 | USED);

        heap->n_allocated_blocks++;
        mmheap_free_lockheld (heap, (struct c_mmheap_allocated *) lastblock + 1);
        /* inform caller */
        dropped_cb (arg, (void *) addr, size);
    }
    check_heap (heap, 0, 0, 0, 0, heap->dump);
}

static os_address mmheap_largest_available_lockheld (struct c_mmheap *heap)
{
    if (heap->free != NULL) {
        struct c_mmheap_tree *block = heap->free;
        os_address maxsize = 0;
        do {
            maxsize = GET_SIZE (block);
            block = block->right;
        } while (block);
        return maxsize;
    } else if (heap->free2) {
        return FREE2_SIZE;
    } else if (heap->free1) {
        return FREE1_SIZE;
    } else {
        return 0;
    }
}

int c_mmheapInit (struct c_mmheap *heap, os_address off, os_address size, unsigned flags)
{
    struct c_mmheap_tree *node;
    struct c_mmheap_allocated *begm, *endm;
    os_mutexAttr mattr;

    if (flags & ~(C_MMHEAP_SHARED)) {
        /* undefined flags specified */
        return -1;
    }

    if (off == 0) {
        off = sizeof (struct c_mmheap);
    }
    off = A ((os_address) heap + off) - (os_address) heap;
    size = (((os_address) heap + size) & (os_address)(-(os_saddress)ALIGNMENT)) - (os_address) heap;
    if (size < off ||
        size - off < (HDR_SIZE +  /* begin marker */
                      HDR_SIZE + 3 * HDR_SIZE + /* one tree node */
                      HDR_SIZE)) { /* end marker */
        /* size - off is too small for a heap */
        return -1;
    }

    begm = (struct c_mmheap_allocated *) ((char *) heap + off);
    node = (struct c_mmheap_tree *) ((char *) heap + off + HDR_SIZE);
    endm = (struct c_mmheap_allocated *) ((char *) heap + size - HDR_SIZE);

    begm->size_and_flags = 0 | USED;
    begm->prev = NULL;
    node->size_and_flags = size - off - 3 * HDR_SIZE;
    node->prev = (struct c_mmheap_tree *) begm;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->list = NULL;
    node->height = 1;
    endm->size_and_flags = 0 | USED;
    endm->prev = node;

    os_mutexAttrInit (&mattr);
    mattr.scopeAttr = (flags & C_MMHEAP_SHARED) ? OS_SCOPE_SHARED : OS_SCOPE_PRIVATE;
    if (os_mutexInit (&heap->lock, &mattr) != os_resultSuccess) {
        return -1;
    }

    heap->free = node;
    heap->free1 = NULL;
    heap->free2 = NULL;
    heap->flags = flags;
    heap->n_free_bytes = GET_SIZE (node);
    heap->n_free_blocks = 1;
    heap->n_allocated_blocks = 0;
    heap->n_failed_allocations = 0;
    heap->dump = 0;
    heap->check = 0;
    heap->heap_check_serial = 0;
    heap->heap_region.base = heap;
    heap->heap_region.off = off;
    heap->heap_region.size = size;
    heap->heap_region.next = NULL;

    if (check_heap (heap, 0, 0, 0, 1, heap->dump))
    {
        os_mutexDestroy (&heap->lock);
        return -1;
    }
    return 0;
}

void c_mmheapFini (struct c_mmheap *heap)
{
    os_mutexDestroy (&heap->lock);
}

void *c_mmheapMalloc (struct c_mmheap *heap, os_address size)
{
    void *ptr;
    os_mutexLock (&heap->lock);
    ptr = mmheap_malloc_lockheld (heap, size);
    os_mutexUnlock (&heap->lock);
    return ptr;
}

void c_mmheapFree (struct c_mmheap *heap, void *b)
{
    os_mutexLock (&heap->lock);
    mmheap_free_lockheld (heap, b);
    os_mutexUnlock (&heap->lock);
}

int c_mmheapAddRegion (struct c_mmheap *heap, void *block, os_address size)
{
    int r;
    os_mutexLock (&heap->lock);
    r = mmheap_addregion_lockheld (heap, block, size);
    os_mutexUnlock (&heap->lock);
    return r;
}

void c_mmheapDropRegion (struct c_mmheap *heap, os_address minfree, os_address minsize, os_address align, void (*dropped_cb) (void *arg, void *addr, os_address size), void *arg)
{
    os_mutexLock (&heap->lock);
    mmheap_dropregion_lockheld (heap, minfree, minsize, align, dropped_cb, arg);
    os_mutexUnlock (&heap->lock);
}

void c_mmheapStats (struct c_mmheap *heap, struct c_mmheapStats *st)
{
    os_mutexLock (&heap->lock);
    st->nused = heap->n_allocated_blocks;
    st->totfree = heap->n_free_bytes;
    st->nfails = heap->n_failed_allocations;
    os_mutexUnlock (&heap->lock);
}

void *c_mmheapCheckPtr (struct c_mmheap *heap, void *ptr)
{
    void *result;
    os_mutexLock (&heap->lock);
    result = mmheap_check_pointer_lockheld (heap, ptr);
    os_mutexUnlock (&heap->lock);
    return result;
}

os_address c_mmheapLargestAvailable (struct c_mmheap *heap)
{
    os_address res;
    os_mutexLock (&heap->lock);
    res = mmheap_largest_available_lockheld (heap);
    os_mutexUnlock (&heap->lock);
    return res;
}
