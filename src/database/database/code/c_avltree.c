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
/***********************************************************************
 *
 * Object-name   : avltree
 * Component     : standard
 * 
 * Implementation: All tree algorithms are implemeted iterative instead
 *                 of recursive to improve performance. 
 *
 ***********************************************************************/
#include "c_avltree.h"
#include <assert.h>

/**
 * Local definitions:
 * C_AVLTREE_MAXHEIGHT represents the maximum tree height (tunable)
 * heightof(tree) inline function returning the height of the specified
 *          tree node.
 **/
#define C_AVLTREE_MAXHEIGHT    (41)
#define heightof(tree)    (((tree) == NULL) ? 0 : (tree)->height)

/**
 * Tree route definitions
 * used in a route stack, implenting a history of a treewalk.
 **/
#define C_AVLTREE_LEFT  (1)
#define C_AVLTREE_RIGHT (0)

/***
 * The avltree administration structure
 *
 * mm is a reference to the associated memory management.
 * root is a reference to the first tree element.
 * size represents the number of elements within the tree.
 * This definition must always be memory compatible with
 * spl_extends_avltree, specified in c_avltree.h
 **/

/**
 * The internal treenode definition
 * This definition must always be memory compatible with
 * spl_extends_avltree_element, specified in c_avltree.h
 *
 * left and right are references to left and right elements within the tree.
 * height represents the height of the node seen from the bottom of the tree.
 **/ 

#define TONODE(t,d) ((c_avlNode)(((c_address)(d))+(t)->offset))
#define TODATA(t,n) ((void *)(((c_address)(n))-(t)->offset))

/***********************************************************************
 *
 * Method     : c_avlTreeRebalance
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
c_avlTreeRebalance (
    c_avlNode **nodeplaces_ptr,
    c_long count)
{
    for ( ; count > 0 ; count--) {
        c_avlNode *nodeplace = *--nodeplaces_ptr;
        c_avlNode node = *nodeplace;
        c_avlNode nodeleft = node->left;
        c_avlNode noderight = node->right;
        c_long heightleft = heightof(nodeleft);
        c_long heightright = heightof(noderight);
        if ((heightright + 1) < heightleft) {
            c_avlNode nodeleftleft = nodeleft->left;
            c_avlNode nodeleftright = nodeleft->right;
            c_long heightleftright = heightof(nodeleftright);
            if (heightof(nodeleftleft) >= heightleftright) {
                node->left = nodeleftright; nodeleft->right = node;
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
            c_avlNode noderightright = noderight->right;
            c_avlNode noderightleft = noderight->left;
            c_long heightrightleft = heightof(noderightleft);
            if (heightof(noderightright) >= heightrightleft) {
                node->right = noderightleft; noderight->left = node;
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
            c_long height;
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
 * Method     : c_avlTreeInsert
 * Algorithm  :
 *    1st : find insertion place by a itterative tree walk.
 *    2nd : insert new node only if not already occupied.
 *    3rd : rebalance tree.
 *
 ***********************************************************************/
void *
c_avlTreeInsert (
    c_avlTree this,
    void *element,
    c_equality (*compareFunction)(),
    void *compareArgument)
{
    c_avlNode new_node;
    c_avlNode *nodeplace;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_long stack_count = 0;
    c_long comparison;

    assert((this != NULL));
    assert(element != NULL);
    assert(compareFunction != (c_equality(*)())NULL);

    new_node = TONODE(this, element);
    nodeplace = (c_avlNode *)&this->root;

    for (;;) {
        c_avlNode node = *nodeplace;
        if (node == NULL) {
            break;
        }
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        comparison = compareFunction(TODATA(this, node),
                                     TODATA(this, new_node),
                                     compareArgument);
        if (comparison == C_GT) {
            nodeplace = &node->left;
        } else if (comparison == C_LT) {
            nodeplace = &node->right;
        } else {
            return TODATA(this, node);
        }
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 1;
    *nodeplace = new_node;
    c_avlTreeRebalance(stack_ptr,stack_count);
    this->size++;
    return element;
}
/***********************************************************************
 *
 * Method     : c_avlTreeReplace
 * Algorithm  :
 *    1st : find insertion place by a iterative tree walk.
 *    2nd : if an existing node exist evaluate the condition.
 *              if the evaluation is True replace the node with the new node.
 *              if the evaluation is False do nothing.
 *    3rd : if no node exists insert the new node and rebalance the tree.
 *
 ***********************************************************************/
void *
c_avlTreeReplace (
    c_avlTree this,
    void *element,
    c_equality (*compareFunction)(),
    void *compareArgument,
    c_bool (*condition)(),
    void *conditionArgument)
{
    c_avlNode new_node;
    c_avlNode *nodeplace;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_long stack_count = 0;
    c_long comparison;

    assert(this != NULL);
    assert(element != NULL);
    assert(compareFunction != (c_equality(*)())NULL);

    new_node = TONODE(this, element);
    nodeplace = (c_avlNode *)&this->root;

    for (;;) {
        c_avlNode node = *nodeplace;
        if (node == NULL) {
            break;
        }
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        comparison = compareFunction(TODATA(this, node),
                                     TODATA(this, new_node),
                                     compareArgument);
        if (comparison == C_GT) {
            nodeplace = &node->left;
        } else if (comparison == C_LT) {
            nodeplace = &node->right;
        } else {
            if (condition != NULL) {
                if (!condition(TODATA(this, node),element,conditionArgument)) {
                    return element;
                }
            }
            new_node->left = node->left;
            new_node->right = node->right;
            new_node->height = node->height;
            *nodeplace = new_node;
            return TODATA(this, node);
        }
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 1;
    *nodeplace = new_node;
    c_avlTreeRebalance(stack_ptr,stack_count);
    this->size++;
    return NULL;
}

/***********************************************************************
 *
 * Method : c_avlTreeRemove
 * Algorithm  :
 *    1st : find node to be deleted by a iterative tree walk.
 *    2nd : if found then remove node.
 *    3rd : rebalance tree.
 *
 ***********************************************************************/
void *
c_avlTreeRemove (
    c_avlTree this,
    void *template,
    c_equality (*compareFunction)(),
    void *compareArgument,
    c_bool (*condition)(),
    void *conditionArgument)
{
    c_avlNode *nodeplace;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_avlNode *nodeplace_to_delete;
    c_avlNode node_to_delete = NULL;
    c_long stack_count = 0;
    c_equality comparison;

    assert(this != NULL);
    assert(compareFunction != (c_equality(*)())NULL);

    nodeplace = (c_avlNode *)&this->root;

    for (;;) {
        c_avlNode node = *nodeplace;
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     template,
                                     compareArgument);
        if (comparison == C_EQ) {
            node_to_delete = node;
            break;
        }
        if (comparison == C_GT) {
            nodeplace = &node->left;
        } else {
            nodeplace = &node->right;
        }
    }
    assert(node_to_delete != NULL);
    if (condition != NULL) {
       if (!condition(TODATA(this,node_to_delete),
                      template,
                      conditionArgument))
       {
           return NULL;
       }
    }
    nodeplace_to_delete = nodeplace;

    if (node_to_delete->left == NULL) {
        *nodeplace_to_delete = node_to_delete->right;
        stack_ptr--; stack_count--;
    } else {
        c_avlNode **stack_ptr_to_delete = stack_ptr;
        c_avlNode node;

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
    c_avlTreeRebalance(stack_ptr,stack_count);
    this->size--;
    return TODATA(this, node_to_delete);
}

/***********************************************************************
 *
 * Method : c_avlTreeFind
 * Algorithm  :
 *    1st : find node by a iterative tree walk.
 *    2nd : return reference to the found node.
 *
 ***********************************************************************/
void *
c_avlTreeFind (
    c_avlTree this,
    void *template,
    c_equality (*compareFunction)(),
    void *compareArgument)
{
    c_long comparison;
    c_avlNode node;

    assert(this != NULL);
    assert(compareFunction != (c_equality (*)())NULL);

    node = this->root;
    for (;;) {
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     template,
                                     compareArgument);
        if (comparison == C_GT) {
            node = node->left;
        } else if (comparison == C_LT) {
            node = node->right;
        } else {
            return TODATA(this, node);
        }
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeNearest
 * Algorithm  :
 *    1st : find node by a iterative tree walk.
 *    2nd : two possibilities: node is found or not.
 *    node found: if requested must be equal then return node.
 *            if requested must be less then get most right
 *            element of left subtree. If the left subtree is
 *            empty then walk back and get the first smaller
 *            element.
 *            if requested must be greater then get most left
 *            element of right subtree. If the right subtree is
 *            empty then walk back and get the first greater
 *            element.
 *    3rd : return reference to the found node.
 *
 ***********************************************************************/
void *
c_avlTreeNearest (
    c_avlTree this,
    void *template,
    c_equality (*compareFunction)(),
    void *compareArgument,
    c_equality specifier)
{
    c_avlNode *nodeplace;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_avlNode node;
    c_long comparison;

    assert(this != NULL);
    assert(compareFunction != (c_equality(*)())NULL);

    nodeplace = (c_avlNode *)&this->root;

    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     template,
                                     compareArgument);
        if (comparison == C_GT) {
            if (node->left == NULL) {
                switch(specifier) {
                case C_GT :
                case C_GE :
                    return TODATA(this, node);
                case C_LT :
                case C_LE :
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return NULL;
                        }
                        stack_ptr--;
                        if ((**stack_ptr)->right == **(stack_ptr+1)) {
                            return TODATA(this, **stack_ptr);
                        }
                    }
                case C_EQ :
                default:
                    return NULL;
                }
            }
            nodeplace = &node->left;
        } else if (comparison == C_LT) {
            if (node->right == NULL) {
                switch(specifier) {
                case C_LT :
                case C_LE :
                    return TODATA(this, node);
                case C_GT :
                case C_GE :
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return NULL;
                        }
                        stack_ptr--;
                        if ((**stack_ptr)->left == **(stack_ptr+1)) {
                            return TODATA(this, **stack_ptr);
                        }
                    }
                case C_EQ :
                default:
                    return NULL;
                }
            }
            nodeplace = &node->right;
        } else {
            switch(specifier) {
            case C_LT :
                if (node->left != NULL) {
                    node = node->left;
                    for (;;) {
                        if (node->right == NULL) {
                            return TODATA(this, node);
                        }
                        node = node->right;
                    }
                }
                for (;;) {
                    if (stack_ptr == &stack[0]) {
                        return NULL;
                    }
                    stack_ptr--;
                    if ((**stack_ptr)->right == **(stack_ptr+1)) {
                        return TODATA(this, **stack_ptr);
                    }
                }
            case C_GT :
                if (node->right != NULL) {
                    node = node->right;
                    for (;;) {
                        if (node->left == NULL) {
                            return TODATA(this, node);
                        }
                        node = node->left;
                    }
                }
                for (;;) {
                    if (stack_ptr == &stack[0]) {
                        return NULL;
                    }
                    stack_ptr--;
                    if ((**stack_ptr)->left == **(stack_ptr+1)) {
                        return TODATA(this, **stack_ptr);
                    }
                }
            case C_LE :
            case C_EQ :
            case C_GE :
                return TODATA(this, node);
            default:
                return NULL;
            }
        }
        stack_ptr++;
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeWalk
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
c_avlTreeWalk (
    c_avlTree this,
    c_bool (*action) (),
    void *actionArgument,
    c_fixType fix)
{
    c_avlNode *nodeplace;
    c_avlNode node;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_long routestack[C_AVLTREE_MAXHEIGHT];
    c_long stack_count = 0;

    assert(this != NULL);
    assert(action != (c_bool(*)())NULL);

    nodeplace = (c_avlNode *)&this->root;
    routestack[stack_count] = C_AVLTREE_LEFT;

    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) return TRUE;
        if (fix == C_PREFIX) {
            if (!action (TODATA(this, node), actionArgument)) {
                return FALSE;
            }
        }
        if (node->left == NULL) {
            if (fix == C_INFIX) {
                if (!action (TODATA(this, node), actionArgument)) {
                    return FALSE;
                }
            }
            if (node->right == NULL) {
                for (;;) {
                    if (fix == C_POSTFIX) {
                        if (!action (TODATA(this, node), actionArgument)) {
                            return FALSE;
                        }
                    }
                    if (stack_ptr == &stack[0]) {
                        return TRUE;
                    }
                    stack_ptr--;
                    stack_count--;
                    nodeplace = *stack_ptr;
                    node = *nodeplace;
                    if (routestack[stack_count] == C_AVLTREE_LEFT) {
                        if (fix == C_INFIX) {
                            if (!action (TODATA(this, node), actionArgument)) {
                                return FALSE;
                            }
                        }
                        if (node->right != NULL) {
                            break;
                        }
                    }
                }
            }
            routestack[stack_count] = C_AVLTREE_RIGHT;
            nodeplace = &node->right;
        } else {
            nodeplace = &node->left;
        }
        routestack[++stack_count] = C_AVLTREE_LEFT;
        stack_ptr++;
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeRangeWalk
 * Algorithm  :
 *    1st : get the reference of the last node within the range using
 *          c_avlTreeNearest().
 *    2nd : Find the first node within the range and construct a tree
 *          walk stack.
 *    3rd : if the right subtree != NULL then first process right subtree.
 *    4th : start treewalk with the current stack until last node is
 *          encountered.
 *
 ***********************************************************************/
c_bool
c_avlTreeRangeWalk (
    c_avlTree this,
    void *startTemplate,
    c_bool startInclude,
    void *endTemplate,
    c_bool endInclude,
    c_equality (*compareFunction)(),
    void *compareArgument,
    c_bool (*action) (),
    void *actionArgument,
    c_fixType fix)
{
    c_avlNode *nodeplace;
    c_avlNode *stack[C_AVLTREE_MAXHEIGHT];
    c_avlNode **stack_ptr = &stack[0];
    c_avlNode node;
    c_avlNode endnode = NULL;
    c_long routestack[C_AVLTREE_MAXHEIGHT];
    c_long stack_count = 0;
    c_equality comparison;
    c_equality endSpec;

    assert(this != NULL);
    assert(compareFunction != (c_equality(*)())NULL);
    assert(action != (c_bool(*)())NULL);

    if (this->size == 0) {
        return TRUE;
    }

    nodeplace = (c_avlNode *)&this->root;
    
    if (endTemplate != NULL) {
        if (endInclude) {
	    endSpec = C_LE;
        } else {
	    endSpec = C_LT;
        }
        endnode = TONODE(this,
                         c_avlTreeNearest (this,
                                           endTemplate,
                                           compareFunction,
                                           compareArgument,
                                           endSpec));
        if (endnode == NULL) {
            return TRUE;
        }
    }
    routestack[stack_count] = C_AVLTREE_LEFT;
    *stack_ptr = nodeplace;
    if (startTemplate != NULL) {
        for (;;) {
            *stack_ptr = nodeplace;
            node = *nodeplace;
            if (node == NULL) {
                return TRUE;
            }
            comparison = compareFunction(TODATA(this, node),
                                         startTemplate,
                                         compareArgument);
            if (comparison == C_GT) {
                if (node->left == NULL) {
                    break;
                }
                nodeplace = &node->left;
            } else if (comparison == C_LT) {
                if (node->right == NULL) {
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return TRUE;
                        }
                        stack_count--;
                        stack_ptr--;
                        if ((**stack_ptr)->left == **(stack_ptr+1)) {
                            break;
                        }
                    }
                    node = **stack_ptr;
                    break;
                } else {
                    routestack[stack_count] = C_AVLTREE_RIGHT;
                    nodeplace = &node->right;
                }
            } else {
	        if (startInclude) break;
                if (node->right != NULL) {
                    node = node->right;
                    for (;;) {
                        if (node->left == NULL) {
                            break;
                        }
                        node = node->left;
                    }
                    break;
                }
                for (;;) {
                    if (stack_ptr == &stack[0]) return TRUE;
                    stack_ptr--;
                    stack_count--;
                    if (routestack[stack_count] == C_AVLTREE_LEFT) {
                        break;
                    }
                }
                node = **stack_ptr;
                break;
            }
            routestack[++stack_count] = C_AVLTREE_LEFT;
            stack_ptr++;
        }
        if (endnode != NULL) {
            comparison = compareFunction(TODATA(this,node),
                                         TODATA(this,endnode),
                                         compareArgument);
            if (comparison == C_GT) {
                return TRUE;
            }
        }
        if (fix == C_INFIX) {
            if (!action (TODATA(this, node), actionArgument)) {
                return FALSE;
            }
        }
        if (node == endnode) {
            return TRUE;
        }
        if (node->right == NULL) {
            for (;;) {
                if (fix == C_POSTFIX) {
                    if (!action (TODATA(this, node), actionArgument)) {
                        return FALSE;
                    }
                }
                if (stack_ptr == &stack[0]) {
                    return TRUE;
                }
                stack_ptr--;
                stack_count--;
                nodeplace = *stack_ptr;
                node = *nodeplace;
                if (routestack[stack_count] == C_AVLTREE_LEFT) {
                    if (fix == C_INFIX) {
                        if (!action (TODATA(this, node), actionArgument)) {
                            return FALSE;
                        }
                    }
                    if (node->right != NULL) {
                        if (node == endnode) {
                            return TRUE;
                        }
                        break;
                    }
                }
            }
        }
        routestack[stack_count] = C_AVLTREE_RIGHT;
        nodeplace = &node->right;
        routestack[++stack_count] = C_AVLTREE_LEFT;
        stack_ptr++;
    }
    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) return TRUE;
        if (fix == C_PREFIX) {
            if (!action (TODATA(this, node), actionArgument)) {
                return FALSE;
            }
        }
        if (node->left == NULL) {
            if (fix == C_INFIX) {
                if (!action (TODATA(this, node), actionArgument)) {
                    return FALSE;
                }
            }
            if (node == endnode) {
                return TRUE;
            }
            if (node->right == NULL) {
                for (;;) {
                    if (fix == C_POSTFIX) {
                        if (!action (TODATA(this, node), actionArgument)) {
                            return FALSE;
                        }
                    }
                    if (stack_ptr == &stack[0]) {
                        return TRUE;
                    }
                    stack_ptr--;
                    stack_count--;
                    nodeplace = *stack_ptr;
                    node = *nodeplace;
                    if (routestack[stack_count] == C_AVLTREE_LEFT) {
                        if (fix == C_INFIX) {
                            if (!action (TODATA(this, node), actionArgument)) {
                                return FALSE;
                            }
                        }
                        if (node->right != NULL) {
                            if (node == endnode) {
                                return TRUE;
                            }
                            break;
                        }
                    }
                }
            }
            routestack[stack_count] = C_AVLTREE_RIGHT;
            nodeplace = &node->right;
        } else {
            nodeplace = &node->left;
        }
        routestack[++stack_count] = C_AVLTREE_LEFT;
        stack_ptr++;
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeFirst
 * Algorithm  :
 *    1st : Go left until the next left pointer = NULL
 *    2nd : Return current node reference.
 *
 ***********************************************************************/
void *
c_avlTreeFirst (
    c_avlTree this)
{
    c_avlNode node;

    assert(this != NULL);

    node = this->root;
    if (node == NULL) {
        return NULL;
    }
    for (;;) {
        if (node->left == NULL) {
            return TODATA(this, node);
        }
        node = node->left;
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeLast
 * Algorithm  :
 *    1st : Go right until the next right pointer = NULL
 *    2nd : Return current node reference.
 *
 ***********************************************************************/
void *
c_avlTreeLast (
    c_avlTree this)
{
    c_avlNode node;

    assert(this != NULL);

    node = this->root;
    if (node == NULL) {
        return NULL;
    }
    for (;;) {
        if (node->right == NULL) {
            return TODATA(this, node);
        }
        node = node->right;
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeCount
 *
 ***********************************************************************/
c_long
c_avlTreeCount (
    c_avlTree this)
{
    assert(this != NULL);
    return this->size;
}

/***********************************************************************
 *
 * Method : c_avlTreeNew
 *
 ***********************************************************************/
c_avlTree
c_avlTreeNew (
    c_mm mm,
    c_long offset)
{
    c_avlTree tree;

    assert(mm != NULL);

    tree = (c_avlTree)c_mmMalloc(mm, C_SIZEOF(c_avlTree));
    tree->root = NULL;
    tree->offset = offset;
    tree->size = 0;
    tree->mm = mm;
    return tree;
}

static void
c_avlNodeFree(
    c_mm mm,
    c_avlNode node)
{
    if (node != NULL) {
        c_avlNodeFree(mm,node->left);
        c_avlNodeFree(mm,node->right);
        c_mmFree(mm,node);
    }
}

/***********************************************************************
 *
 * Method : c_avlTreeFree
 * Implementation :
 *    This method expects an empty tree, if not empty references can
 *    be lost. The user is responsible for the garbage collection.
 *
 ***********************************************************************/
void
c_avlTreeFree (
    c_avlTree this)
{
    assert(this != NULL);
    assert(this->root == NULL); /* if this fail then comment it out (and write ticket), causes a tiny memory leak */
    c_mmFree(this->mm, this);
}
