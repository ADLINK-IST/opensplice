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

/* Note: to improve performance all tree algorithms are
 *       implemeted iterative instead of recursive. 
 ***********************************************************************/

#include "os.h"

#include "d_misc.h"
#include "d_avltree.h"

/** @class d_avlNode
    @brief The class implementation.
*/
C_STRUCT(d_avlNode) {
    d_avlNode left;    /* The element located at the left of this tree element */
    d_avlNode right;   /* The element located at the right of this tree element */
    c_short   height;  /* The heigth of this element within the tree */
    c_voidp   data;
};

/**
 * Local definitions:
 * D_AVLTREE_MAXHEIGHT represents the maximum tree height (tunable)
 * heightof(tree) inline function returning the height of the specified
 *          tree node.
 **/
#define D_AVLTREE_MAXHEIGHT     (41)
#define heightof(tree)          (((tree) == NULL) ? 0 : (tree)->height)

/**
 * Tree route definitions
 * used in a route stack, implementing a history of a treewalk.
 **/
#define D_AVLTREE_LEFT  (1)
#define D_AVLTREE_RIGHT (0)


void
avlTreeFreeNode(
    d_avlNode node )
{
    node->left   = NULL;
    node->right  = NULL;
    node->height = 0;
    node->data   = NULL;
    d_free(node);
}

/***********************************************************************
 *
 * Method     : d_avlTreeRebalance
 * Description:
 *    After an insert or remove action this method is called to
 *    rebalance the tree. The insert or remove method will pass the
 *    stack (path to the inserted node).
 * Algorithm  :
 *    1st : rewind stack and for ech step check tree heights.
 *    2nd : if heights differ more the 1 then rebalance this level.
 *
 ***********************************************************************/
static void
d_avlTreeRebalance (
    d_avlNode ** nodeplaces_ptr,
    c_long       count )
{
    d_avlNode * nodeplace;
    d_avlNode   node;
    d_avlNode   nodeleft;
    d_avlNode   noderight;
    c_long      heightleft;
    c_long      heightright;
    d_avlNode   nodeleftleft;
    d_avlNode   nodeleftright;
    c_long      heightleftright;
    d_avlNode   noderightright;
    d_avlNode   noderightleft;
    c_long      heightrightleft;
    c_long      height;

    for ( ; count > 0 ; count--) {
        nodeplace = *--nodeplaces_ptr;
        node = *nodeplace;
        nodeleft = node->left;
        noderight = node->right;
        heightleft = heightof(nodeleft);
        heightright = heightof(noderight);
        if ((heightright + 1) < heightleft) {
            nodeleftleft = nodeleft->left;
            nodeleftright = nodeleft->right;
            heightleftright = heightof(nodeleftright);
            if (heightof(nodeleftleft) >= heightleftright) {
                node->left = nodeleftright;
                nodeleft->right = node;
                nodeleft->height = 1 + (node->height = 1 + heightleftright);
                *nodeplace = nodeleft;
            } else {
                nodeleft->right = nodeleftright->left;
                node->left = nodeleftright->right;
                nodeleftright->left = nodeleft;
                nodeleftright->right = node;
                nodeleft->height = node->height = heightleftright;
                nodeleftright->height = heightleft;
                *nodeplace = nodeleftright;
            }
        } else if ((heightleft + 1) < heightright) {
            noderightright = noderight->right;
            noderightleft = noderight->left;
            heightrightleft = heightof(noderightleft);
            if (heightof(noderightright) >= heightrightleft) {
                node->right = noderightleft;
                noderight->left = node;
                noderight->height = 1 + (node->height = 1 + heightrightleft);
                *nodeplace = noderight;
            } else {
                noderight->left = noderightleft->right;
                node->right = noderightleft->left;
                noderightleft->right = noderight;
                noderightleft->left = node;
                noderight->height = node->height = heightrightleft;
                noderightleft->height = heightright;
                *nodeplace = noderightleft;
            }
        } else {
            if (heightleft<heightright) {
                height = heightright+1;
            } else {
                height = heightleft+1;
            }
            if (height == node->height) {
                break;
            }
            node->height = height;
        }
    }
}

/***********************************************************************
 *
 * Method     : d_avlTreeInsert
 * Algorithm  :
 *    1st : find insertion place by a iterative tree walk.
 *    2nd : insert new node only if not already occupied.
 *    3rd : rebalance tree.
 * If the item is in the tree
 * Then the existing item is returned
 * Else it is added and NULL is returned
 *
 ***********************************************************************/
c_voidp
d_avlTreeInsert (
    d_avlNode * rootNodePntr,
    c_voidp     data,
    int (*      compareFunction)() )
{
    d_avlNode    node;
    d_avlNode *  nodeplace;
    d_avlNode *  stack[D_AVLTREE_MAXHEIGHT];
    d_avlNode ** stackPtr = &stack[0];
    c_long       stack_count = 0;
    c_long       comparison;

    assert(rootNodePntr != NULL);
    assert(data != NULL);
    assert(compareFunction != (int(*)())NULL);

    nodeplace = rootNodePntr;

    for (;;) {
        node = *nodeplace;
        if (node == NULL) {
            break;
        }
        *stackPtr = nodeplace;
        stackPtr++;
        stack_count++;
        comparison = compareFunction(node->data, data);
        if (comparison > 0) {
            nodeplace = &node->left;
        } else if (comparison < 0) {
            nodeplace = &node->right;
        } else {
            return node->data;
        }
    }
    node = (d_avlNode)d_malloc((os_uint32)C_SIZEOF(d_avlNode), "TreeNode");
    if (node == NULL) {
        return data;    /* cannot insert it */
    } else {
        node->left = NULL;
        node->right = NULL;
        node->height = 1;
        node->data = data;
        *nodeplace = node;
        d_avlTreeRebalance(stackPtr,stack_count);
    }
    return NULL;
}

/***********************************************************************
 *
 * Method : d_avlTreeRemove
 * Algorithm  :
 *    1st : find node to be deleted by a iterative tree walk.
 *    2nd : if found then remove node.
 *    3rd : rebalance tree.
 * Returns data or zero if not found
 ***********************************************************************/
c_voidp
d_avlTreeRemove (
    d_avlNode * rootNodePntr,
    c_voidp     data,
    int (*      compareFunction)() )
{
    d_avlNode *  nodeplace;
    d_avlNode *  stack[D_AVLTREE_MAXHEIGHT];
    d_avlNode ** stack_ptr;
    d_avlNode *  nodeplace_to_delete;
    d_avlNode    node_to_delete;
    d_avlNode    node;
    d_avlNode ** stack_ptr_to_delete;
    c_long       stack_count;
    int          comparison;
    c_voidp      removedData;

    assert(rootNodePntr != NULL);
    assert(*rootNodePntr != NULL);
    assert(data != NULL);
    assert(compareFunction != (int(*)())NULL);

    stack_ptr = &stack[0];
    node_to_delete = NULL;
    stack_count = 0;
    nodeplace = rootNodePntr;

    for (;;) {
        node = *nodeplace;
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(data, node->data);
        if (comparison == 0) {
            node_to_delete = node;
            break;
        }
        if (comparison < 0) {
            nodeplace = &node->left;
        } else {
            nodeplace = &node->right;
        }
    }
    assert(node_to_delete != NULL);
    nodeplace_to_delete = nodeplace;

    if (node_to_delete->left == NULL) {
        *nodeplace_to_delete = node_to_delete->right;
        stack_ptr--;
        stack_count--;
    } else {
        stack_ptr_to_delete = stack_ptr;

        nodeplace = &node_to_delete->left;
        for (;;) {
            node = *nodeplace;
            if (node->right == NULL) {
                break;
            }
            *stack_ptr = nodeplace;
            stack_ptr++;
            stack_count++;
            nodeplace = &node->right;
        }
        *nodeplace = node->left;
        node->left = node_to_delete->left;
        node->right = node_to_delete->right;
        node->height = node_to_delete->height;
        *nodeplace_to_delete = node;
        *stack_ptr_to_delete = &node->left;
    }
    d_avlTreeRebalance(stack_ptr, stack_count);

    removedData = node_to_delete->data;
    avlTreeFreeNode(node_to_delete);
    return removedData;
}

/***********************************************************************
 *
 * Method : d_avlTreeTake
 * Algorithm  :
 *    1st : find a node to be deleted by a iterative tree walk.
 *    2nd : if found then remove node.
 *    3rd : rebalance tree.
 * Returns data or zero if not found
 ***********************************************************************/
c_voidp
d_avlTreeTake (
    d_avlNode * rootNodePntr )
{
    d_avlNode *  nodeplace;
    d_avlNode *  stack[D_AVLTREE_MAXHEIGHT];
    d_avlNode ** stack_ptr;
    d_avlNode *  nodeplace_to_delete;
    d_avlNode    node_to_delete;
    d_avlNode    node;
    d_avlNode ** stack_ptr_to_delete;
    c_long       stack_count;
    c_voidp      removedData;

    assert(rootNodePntr != NULL);
    assert(*rootNodePntr != NULL);

    stack_ptr = &stack[0];
    node_to_delete = NULL;
    stack_count = 0;
    nodeplace = rootNodePntr;

    node = *nodeplace;
    *stack_ptr = nodeplace;
    stack_ptr++;
    stack_count++;
    if (node == NULL) {
        return NULL;
    }
    node_to_delete = node;

    assert(node_to_delete != NULL);
    nodeplace_to_delete = nodeplace;

    if (node_to_delete->left == NULL) {
        *nodeplace_to_delete = node_to_delete->right;
        stack_ptr--;
        stack_count--;
    } else {
        stack_ptr_to_delete = stack_ptr;

        nodeplace = &node_to_delete->left;
        for (;;) {
            node = *nodeplace;
            if (node->right == NULL) {
                break;
            }
            *stack_ptr = nodeplace;
            stack_ptr++;
            stack_count++;
            nodeplace = &node->right;
        }
        *nodeplace = node->left;
        node->left = node_to_delete->left;
        node->right = node_to_delete->right;
        node->height = node_to_delete->height;
        *nodeplace_to_delete = node;
        *stack_ptr_to_delete = &node->left;
    }
    d_avlTreeRebalance(stack_ptr, stack_count);

    removedData = node_to_delete->data;
    avlTreeFreeNode(node_to_delete);
    return removedData;
}

/***********************************************************************
 *
 * Method : d_avlTreeFind
 * Algorithm  :
 *    1st : find node by a iterative tree walk.
 *    2nd : return reference to the found node.
 *
 ***********************************************************************/
c_voidp
d_avlTreeFind (
    d_avlNode rootNode,
    c_voidp   data,
    int (*    compareFunction)() )
{
    int       comparison;
    d_avlNode node;

    assert(rootNode != NULL);
    assert(compareFunction != (int (*)())NULL);

    node = rootNode;
    for (;;) {
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(data, node->data);
        if (comparison < 0) {
            node = node->left;
        } else if (comparison > 0) {
            node = node->right;
        } else {
            return node->data;
        }
    }
    return NULL;
}

/***********************************************************************
 *
 * Method : d_avlTreeFirst
 * Algorithm  :
 *    1st : Go left until the next left pointer == NULL
 *    2nd : Return current node reference.
 * PRE: there is at least one node
 ***********************************************************************/
c_voidp
d_avlTreeFirst (
    d_avlNode rootNode )
{
    d_avlNode node = rootNode;

    assert(node != NULL);

    while (node->left) {
        node = node->left;
    }
    return node->data;
}

/***********************************************************************
 *
 * Method : d_avlTreeWalk
 * Implementation :
 *    This function is never designed but is more or less a hack,
 *    nevertheless it can be the best possible. But improvements may
 *    also be possible.
 * Algorithm  :
 *    1st : for each node push information on local stack and go left.
 *          a reference to the current node is put on stack and the
 *          intended direction to go (left or right).
 *    2nd : if left is processed go right.
 *    3rd : if right is processed pop from stack so the previous node
 *          can be processed further.
 *
 ***********************************************************************/
c_bool
d_avlTreeWalk (
    d_avlNode * root,
    c_bool (*   action) (),
    c_voidp     actionArgument )
{
    d_avlNode *  nodeplace;
    d_avlNode    node;
    d_avlNode *  stack[D_AVLTREE_MAXHEIGHT];
    d_avlNode ** stack_ptr = &stack[0];
    c_long       routestack[D_AVLTREE_MAXHEIGHT];
    c_long       stack_count = 0;

    assert(root != NULL);
    assert(*root != NULL);
    assert(action != (c_bool(*)())NULL);

    nodeplace = root;
    routestack[stack_count] = D_AVLTREE_LEFT;

    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) return TRUE;
        if (node->left) {
            nodeplace = &node->left;
        } else {
            if (!action (node->data, actionArgument)) {
                return FALSE;
            }
            if (node->right == NULL) {
                for (;;) {
                    if (stack_ptr == &stack[0]) {
                        return TRUE;
                    }
                    stack_ptr--;
                    stack_count--;
                    nodeplace = *stack_ptr;
                    node = *nodeplace;
                    if (routestack[stack_count] == D_AVLTREE_LEFT) {
                        if (!action (node->data, actionArgument)) {
                            return FALSE;
                        }
                        if (node->right != NULL) {
                            break;
                        }
                    }
                }
            }
            routestack[stack_count] = D_AVLTREE_RIGHT;
            nodeplace = &node->right;
        }
        routestack[++stack_count] = D_AVLTREE_LEFT;
        stack_ptr++;
    }
    return TRUE;
}

/***********************************************************************
 *
 * Method : d_avlTreeFree
 * Implementation :
 *    This method expects an empty tree, if not empty references can
 *    be lost. The user is responsible for the garbage collection.
 *
 ***********************************************************************/

static void
avlTreeFree (
    d_avlNode node )
{
    if (node) {
        avlTreeFree(node->left);
        avlTreeFree(node->right);
        avlTreeFreeNode(node);
    }
}

static void
avlTreeCleanAndFree (
    d_avlNode node,
    void (*   cleanAction)() )
{
    if (node) {
        avlTreeCleanAndFree(node->left, cleanAction);
        avlTreeCleanAndFree(node->right, cleanAction);
        cleanAction(node->data);
        avlTreeFreeNode(node);
    }
}

void
d_avlTreeFree (
    d_avlNode node,
    void (*   cleanAction)() )
{
    if (cleanAction == NULL) {
        avlTreeFree(node);
    } else {
        avlTreeCleanAndFree(node, cleanAction);
    }
}
