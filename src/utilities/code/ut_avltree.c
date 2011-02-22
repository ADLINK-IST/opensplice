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
#include <assert.h>

#include "ut__avltree.h"

#include "os.h"

/**
 * Local definitions:
 * UT_AVLTREE_MAXHEIGHT represents the maximum tree height (tunable)
 * heightof(tree) inline function returning the height of the specified
 *          tree node.
 **/
#define UT_AVLTREE_MAXHEIGHT    (40)
#define heightof(tree)          (((tree) == NULL) ? 0 : (tree)->height)

/**
 * Tree route definitions
 * used in a route stack, implenting a history of a treewalk.
 **/
#define UT_AVLTREE_LEFT  (1)
#define UT_AVLTREE_RIGHT (0)

/***
 * The avltree administration structure
 *
 * mm is a reference to the associated memory management.
 * root is a reference to the first tree element.
 * size represents the number of elements within the tree.
 * This definition must always be memory compatible with
 * spl_extends_avltree, specified in ut_avltree.h
 **/

/**
 * The internal treenode definition
 * This definition must always be memory compatible with
 * spl_extends_avltree_element, specified in ut_avltree.h
 *
 * left and right are references to left and right elements within the tree.
 * height represents the height of the node seen from the bottom of the tree.
 **/ 

#define TONODE(t,d) ((ut_avlNode)(((os_address)(d))+(t)->offset))
#define TODATA(t,n) ((void *)(((os_address)(n))-(t)->offset))

/***********************************************************************
 *
 * Method     : ut_avlTreeRebalance
 * Description:
 *    After an insert or remove action this method is called to
 *    rebalance the tree. The insert or remove method will pass the
 *    stack (path to the inserted node).
 * Implementation :
 * History    :
 * Algorithm  :
 *    1st : rewind stack and for ech step check tree heights.
 *    2nd : if heights differ more the 1 then rebalance this level.
 * Bugs       :
 *
 ***********************************************************************/
static void
ut_avlTreeRebalance (
    ut_avlNode **nodeplaces_ptr,
    os_uint32 count)
{
    for ( ; count > 0 ; count--) {
        ut_avlNode *nodeplace = *--nodeplaces_ptr;
        ut_avlNode node = *nodeplace;
        ut_avlNode nodeleft = node->left;
        ut_avlNode noderight = node->right;
        os_uint32 heightleft = heightof(nodeleft);
        os_uint32 heightright = heightof(noderight);
        if ((heightright + 1) < heightleft) {
            ut_avlNode nodeleftleft = nodeleft->left;
            ut_avlNode nodeleftright = nodeleft->right;
            os_uint32 heightleftright = heightof(nodeleftright);
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
            ut_avlNode noderightright = noderight->right;
            ut_avlNode noderightleft = noderight->left;
            os_uint32 heightrightleft = heightof(noderightleft);
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
            os_uint32 height;
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
 * Method     : ut_avlTreeInsert
 * Implementation :
 * History    :
 * Algorithm  :
 *    1st : find insertion place by a itterative tree walk.
 *    2nd : insert new node only if not already occupied.
 *    3rd : rebalance tree.
 * Bugs       :
 *
 ***********************************************************************/
void *
ut_avlTreeInsert (
    ut_avlTree this,
    void *element,
    os_equality (*compareFunction)(),
    void *compareArgument)
{
    ut_avlNode new_node;
    ut_avlNode *nodeplace;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    os_uint32 stack_count = 0;
    os_equality comparison;

    assert((this != NULL));
    assert(element != NULL);
    assert(compareFunction != (os_equality(*)())NULL);

    new_node = TONODE(this, element);
    nodeplace = (ut_avlNode *)&this->root;

    for (;;) {
        ut_avlNode node = *nodeplace;
        if (node == NULL) {
            break;
        }
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        comparison = compareFunction(TODATA(this, node),
                                     TODATA(this, new_node),
                                     compareArgument);
        if (comparison == OS_GT) {
            nodeplace = &node->left;
        } else if (comparison == OS_LT) {
            nodeplace = &node->right;
        } else {
            return TODATA(this, node);
        }
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 1;
    *nodeplace = new_node;
    ut_avlTreeRebalance(stack_ptr,stack_count);
    this->size++;
    return element;
}

/***********************************************************************
 *
 * Method     : ut_avlTreeReplace
 * Implementation :
 * History    :
 * Algorithm  :
 *    1st : find insertion place by a itterative tree walk.
 *    2nd : if an existing node exist evaluate the condition.
 *              if the evaluation is True replace the node with the new node.
 *              if the evaluation is False do nothing.
 *    3rd : if no node exists insert the new node and rebalance the tree.
 * Bugs       :
 *
 ***********************************************************************/
void *
ut_avlTreeReplace (
    ut_avlTree this,
    void *element,
    os_equality (*compareFunction)(),
    void *compareArgument,
    os_int32 (*condition)(),
    void *conditionArgument)
{
    ut_avlNode new_node;
    ut_avlNode *nodeplace;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    os_uint32 stack_count = 0;
    os_equality comparison;

    assert(this != NULL);
    assert(element != NULL);
    assert(compareFunction != (os_equality(*)())NULL);

    new_node = TONODE(this, element);
    nodeplace = (ut_avlNode *)&this->root;

    for (;;) {
        ut_avlNode node = *nodeplace;
        if (node == NULL) {
            break;
        }
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        comparison = compareFunction(TODATA(this, node),
                                     TODATA(this, new_node),
                                     compareArgument);
        if (comparison == OS_GT) {
            nodeplace = &node->left;
        } else if (comparison == OS_LT) {
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
    ut_avlTreeRebalance(stack_ptr,stack_count);
    this->size++;
    return NULL;
}

/***********************************************************************
 *
 * Method : ut_avlTreeRemove
 * Implementation :
 * History :
 * Algorithm  :
 *    1st : find node to be deleted by a itterative tree walk.
 *    2nd : if found then remove node.
 *    3rd : rebalance tree.
 * Bugs :
 *
 ***********************************************************************/
void *
ut_avlTreeRemove (
    ut_avlTree this,
    void *template,
    os_equality (*compareFunction)(),
    void *compareArgument,
    os_int32 (*condition)(),
    void *conditionArgument)
{
    ut_avlNode *nodeplace;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    ut_avlNode *nodeplace_to_delete;
    ut_avlNode node_to_delete = NULL;
    ut_avlNode node_for_template;
    os_uint32 stack_count = 0;
    os_equality comparison;

    assert(this != NULL);
    assert(template != NULL);
    assert(compareFunction != (os_equality(*)())NULL);

    node_for_template = TONODE(this, template);
    nodeplace = (ut_avlNode *)&this->root;

    for (;;) {
        ut_avlNode node = *nodeplace;
        *stack_ptr = nodeplace;
        stack_ptr++;
        stack_count++;
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     TODATA(this, node_for_template),
                                     compareArgument);
        if (comparison == OS_EQ) {
            node_to_delete = node;
            break;
        }
        if (comparison == OS_GT) {
            nodeplace = &node->left;
        } else {
            nodeplace = &node->right;
        }
    }
    assert(node_to_delete != NULL);
    if (condition != NULL) {
       if (!condition(TODATA(this,node_to_delete),
                      TODATA(this,node_for_template),conditionArgument)) {
           return NULL;
       }
    }
    nodeplace_to_delete = nodeplace;

    

    if (node_to_delete->left == NULL) {
        *nodeplace_to_delete = node_to_delete->right;
        stack_ptr--;
        stack_count--;
    } else {
        ut_avlNode **stack_ptr_to_delete = stack_ptr;
        ut_avlNode node;

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
    ut_avlTreeRebalance(stack_ptr,stack_count);
    this->size--;
    return TODATA(this, node_to_delete);
}

/***********************************************************************
 *
 * Method : ut_avlTreeFind
 * Implementation :
 *    This function can be replaced by ut_avlTreeNearest() but
 *    considering the simplicity of the code with respect to performance
 *    this function is preserved.
 * History :
 * Algorithm  :
 *    1st : find node by a itterative tree walk.
 *    2nd : return reference to the found node.
 * Bugs :
 *
 ***********************************************************************/
void *
ut_avlTreeFind (
    ut_avlTree this,
    void *template,
    os_equality (*compareFunction)(),
    void *compareArgument)
{
    os_equality comparison;
    ut_avlNode node;

    assert(this != NULL);
    assert(compareFunction != (os_equality (*)())NULL);

    node = this->root;
    for (;;) {
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     template,
                                     compareArgument);
        if (comparison == OS_GT) {
            node = node->left;
        } else if (comparison == OS_LT) {
            node = node->right;
        } else {
            return TODATA(this, node);
        }
    }
}

/***********************************************************************
 *
 * Method : ut_avlTreeNearest
 * Implementation :
 * History :
 *    This method is added to the original design.
 * Algorithm  :
 *    1st : find node by a itterative tree walk.
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
 * Bugs :
 *
 ***********************************************************************/
void *
ut_avlTreeNearest (
    ut_avlTree this,
    void *template,
    os_equality (*compareFunction)(),
    void *compareArgument,
    os_equality specifier)
{
    ut_avlNode *nodeplace;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    ut_avlNode node;
    os_equality comparison;

    assert(this != NULL);
    assert(template != NULL);
    assert(compareFunction != (os_equality(*)())NULL);

    nodeplace = (ut_avlNode *)&this->root;

    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) {
            return NULL;
        }
        comparison = compareFunction(TODATA(this, node),
                                     template,
                                     compareArgument);
        if (comparison == OS_GT) {
            if (node->left == NULL) {
                switch(specifier) {
                case OS_GT :
                case OS_GE :
                    return TODATA(this, node);
                case OS_LT :
                case OS_LE :
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return NULL;
                        }
                        stack_ptr--;
                        if ((**stack_ptr)->right == **(stack_ptr+1)) {
                            return TODATA(this, **stack_ptr);
                        }
                    }
                case OS_EQ :
                default:
                    return NULL;
                }
            }
            nodeplace = &node->left;
        } else if (comparison == OS_LT) {
            if (node->right == NULL) {
                switch(specifier) {
                case OS_LT :
                case OS_LE :
                    return TODATA(this, node);
                case OS_GT :
                case OS_GE :
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return NULL;
                        }
                        stack_ptr--;
                        if ((**stack_ptr)->left == **(stack_ptr+1)) {
                            return TODATA(this, **stack_ptr);
                        }
                    }
                case OS_EQ :
                default:
                    return NULL;
                }
            }
            nodeplace = &node->right;
        } else {
            switch(specifier) {
            case OS_LT :
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
            case OS_GT :
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
            case OS_LE :
            case OS_EQ :
            case OS_GE :
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
 * Method : ut_avlTreeWalk
 * Implementation :
 *    This function is never designed but is more or less a hack,
 *    nevertheless it can be the best possible. But improvements may
 *    also be possible.
 * History :
 *    This method is added to the original design.
 * Algorithm  :
 *    1st : for each node push information on local stack and go left.
 *          a reference to the current node is put on stack and the
 *          intended direction to go (left or right).
 *    2nd : if left is processed go right.
 *    3rd : if right is processed pop from stack so the previous node
 *          can be processed further.
 * Bugs :
 *
 ***********************************************************************/
os_int32
ut_avlTreeWalk (
    ut_avlTree this,
    os_int32 (*action) (),
    void *actionArgument,
    ut_fixType fix)
{
    ut_avlNode *nodeplace;
    ut_avlNode node;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    os_char routestack[UT_AVLTREE_MAXHEIGHT];
    os_uint32 stack_count = 0;

    assert(this != NULL);
    assert(action != (os_int32(*)())NULL);

    nodeplace = (ut_avlNode *)&this->root;
    routestack[stack_count] = UT_AVLTREE_LEFT;

    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) return 1;
        if (fix == UT_PREFIX) {
            if (!action (TODATA(this, node), actionArgument)) {
                return 0;
            }
        }
        if (node->left == NULL) {
            if (fix == UT_INFIX) {
                if (!action (TODATA(this, node), actionArgument)) {
                    return 0;
                }
            }
            if (node->right == NULL) {
                for (;;) {
                    if (fix == UT_POSTFIX) {
                        if (!action (TODATA(this, node), actionArgument)) {
                            return 0;
                        }
                    }
                    if (stack_ptr == &stack[0]) {
                        return 1;
                    }
                    stack_ptr--;
                    stack_count--;
                    nodeplace = *stack_ptr;
                    node = *nodeplace;
                    if (routestack[stack_count] == UT_AVLTREE_LEFT) {
                        if (fix == UT_INFIX) {
                            if (!action (TODATA(this, node), actionArgument)) {
                                return 0;
                            }
                        }
                        if (node->right != NULL) {
                            break;
                        }
                    }
                }
            }
            routestack[stack_count] = UT_AVLTREE_RIGHT;
            nodeplace = &node->right;
        } else {
            nodeplace = &node->left;
        }
        routestack[++stack_count] = UT_AVLTREE_LEFT;
        stack_ptr++;
    }
}

/***********************************************************************
 *
 * Method : ut_avlTreeRangeWalk
 * Implementation :
 *    This function is never designed but is more or less a hack,
 *    nevertheless it can be the best possible. But improvements may
 *    also be possible.
 * History :
 *    This method is added to the original design.
 * Algorithm  :
 *    1st : get the reference of the last node within the range using
 *          ut_avlTreeNearest().
 *    2nd : Find the first node within the range and construct a tree
 *          walk stack.
 *    3rd : if the right subtree != NULL then first process right subtree.
 *    4th : start treewalk with the current stack until last node is
 *          encountered.
 * Bugs :
 *    This method is not tested for correct behavior on prefix and
 *    postfix actions.
 *
 ***********************************************************************/
os_int32
ut_avlTreeRangeWalk (
    ut_avlTree this,
    void *startTemplate,
    os_int32 startInclude,
    void *endTemplate,
    os_int32 endInclude,
    os_equality (*compareFunction)(),
    void *compareArgument,
    os_int32 (*action) (),
    void *actionArgument,
    ut_fixType fix)
{
    ut_avlNode *nodeplace;
    ut_avlNode *stack[UT_AVLTREE_MAXHEIGHT];
    ut_avlNode **stack_ptr = &stack[0];
    ut_avlNode node;
    ut_avlNode endnode = NULL;
    os_char routestack[UT_AVLTREE_MAXHEIGHT];
    os_uint32 stack_count = 0;
    os_equality comparison;
    os_equality endSpec;

    assert(this != NULL);
    assert(compareFunction != (os_equality(*)())NULL);
    assert(action != (os_int32(*)())NULL);


    if (this->size == 0) {
        return 1;
    }

    nodeplace = (ut_avlNode *)&this->root;
    
    if (endTemplate != NULL) {
        if (endInclude) {
            endSpec = OS_LE;
        } else {
            endSpec = OS_LT;
        }
        endnode = TONODE(this,
                         ut_avlTreeNearest(this,
                                           endTemplate,
                                           compareFunction,
                                           compareArgument,
                                           endSpec));
        if (endnode == NULL) {
            return 1;
        }
    }
    routestack[stack_count] = UT_AVLTREE_LEFT;
    *stack_ptr = nodeplace;
    if (startTemplate != NULL) {
        for (;;) {
            *stack_ptr = nodeplace;
            node = *nodeplace;
            if (node == NULL) {
                return 1;
            }
            comparison = compareFunction(TODATA(this, node),
                                         startTemplate,
                                         compareArgument);
            if (comparison == OS_GT) {
                if (node->left == NULL) {
                    break;
                }
                nodeplace = &node->left;
            } else if (comparison == OS_LT) {
                if (node->right == NULL) {
                    for (;;) {
                        if (stack_ptr == &stack[0]) {
                            return 1;
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
                    routestack[stack_count] = UT_AVLTREE_RIGHT;
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
                    if (stack_ptr == &stack[0]) return 1;
                    stack_ptr--;
                    stack_count--;
                    if (routestack[stack_count] == UT_AVLTREE_LEFT) {
                        break;
                    }
                }
                node = **stack_ptr;
                break;
            }
            routestack[++stack_count] = UT_AVLTREE_LEFT;
            stack_ptr++;
        }
        if (endnode != NULL) {
            comparison = compareFunction(TODATA(this,node),
                                         TODATA(this,endnode),
                                         compareArgument);
            if (comparison == OS_GT) {
                return 1;
            }
        }
        if (fix == UT_INFIX) {
            if (!action (TODATA(this, node), actionArgument)) {
                return 0;
            }
        }
        if (node == endnode) {
            return 1;
        }
        if (node->right == NULL) {
            for (;;) {
                if (fix == UT_POSTFIX) {
                    if (!action (TODATA(this, node), actionArgument)) {
                        return 0;
                    }
                }
                if (stack_ptr == &stack[0]) {
                    return 1;
                }
                stack_ptr--;
                stack_count--;
                nodeplace = *stack_ptr;
                node = *nodeplace;
                if (routestack[stack_count] == UT_AVLTREE_LEFT) {
                    if (fix == UT_INFIX) {
                        if (!action (TODATA(this, node), actionArgument)) {
                            return 0;
                        }
                    }
                    if (node->right != NULL) {
                        if (node == endnode) {
                            return 1;
                        }
                        break;
                    }
                }
            }
        }
        routestack[stack_count] = UT_AVLTREE_RIGHT;
        nodeplace = &node->right;
        routestack[++stack_count] = UT_AVLTREE_LEFT;
        stack_ptr++;
    }
    for (;;) {
        *stack_ptr = nodeplace;
        node = *nodeplace;
        if (node == NULL) return 1;
        if (fix == UT_PREFIX) {
            if (!action(TODATA(this, node), actionArgument)) {
                return 0;
            }
        }
        if (node->left == NULL) {
            if (fix == UT_INFIX) {
                if (!action(TODATA(this, node), actionArgument)) {
                    return 0;
                }
            }
            if (node == endnode) {
                return 1;
            }
            if (node->right == NULL) {
                for (;;) {
                    if (fix == UT_POSTFIX) {
                        if (!action(TODATA(this, node), actionArgument)) {
                            return 0;
                        }
                    }
                    if (stack_ptr == &stack[0]) {
                        return 1;
                    }
                    stack_ptr--;
                    stack_count--;
                    nodeplace = *stack_ptr;
                    node = *nodeplace;
                    if (routestack[stack_count] == UT_AVLTREE_LEFT) {
                        if (fix == UT_INFIX) {
                            if (!action(TODATA(this, node), actionArgument)) {
                                return 0;
                            }
                        }
                        if (node->right != NULL) {
                            if (node == endnode) {
                                return 1;
                            }
                            break;
                        }
                    }
                }
            }
            routestack[stack_count] = UT_AVLTREE_RIGHT;
            nodeplace = &node->right;
        } else {
            nodeplace = &node->left;
        }
        routestack[++stack_count] = UT_AVLTREE_LEFT;
        stack_ptr++;
    }
}

/***********************************************************************
 *
 * Method : ut_avlTreeFirst
 * Implementation :
 * History :
 *    This method is added to the original design.
 * Algorithm  :
 *    1st : Go left until the next left pointer = NULL
 *    2nd : Return current node reference.
 * Bugs :
 *
 ***********************************************************************/
void *
ut_avlTreeFirst (
    ut_avlTree this)
{
    ut_avlNode node;

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
 * Method : ut_avlTreeLast
 * Implementation :
 * History :
 *    This method is added to the original design.
 * Algorithm  :
 *    1st : Go right until the next right pointer = NULL
 *    2nd : Return current node reference.
 * Bugs :
 *
 ***********************************************************************/
void *
ut_avlTreeLast (
    ut_avlTree this)
{
    ut_avlNode node;

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
 * Method : ut_avlTreeCount
 * Implementation :
 * History :
 *    This method is added to the original design.
 * Bugs :
 *
 ***********************************************************************/
os_uint32
ut_avlTreeCount (
    ut_avlTree this)
{
    assert(this != NULL);
    return this->size;
}

/***********************************************************************
 *
 * Method : ut_avlTreeNew
 * Implementation :
 * History :
 *    This method is added to the original design.
 * Bugs :
 *
 ***********************************************************************/
ut_avlTree
ut_avlTreeNew (
    os_uint32 offset)
{
    ut_avlTree tree;

    tree = (ut_avlTree)os_malloc(OS_SIZEOF(ut_avlTree));
    tree->root = NULL;
    tree->offset = offset;
    tree->size = 0;
    return tree;
}

static void
ut_avlNodeFree(
    ut_avlNode node)
{
    if (node != NULL) {
        ut_avlNodeFree(node->left);
        ut_avlNodeFree(node->right);
        os_free(node);
    }
}

/***********************************************************************
 *
 * Method : ut_avlTreeFree
 * Implementation :
 *    This method expects an empty tree, if not empty references can
 *    be lost. The user is responsible for the garbage collection.
 * History :
 *    This method is added to the original design.
 * Bugs :
 *
 ***********************************************************************/
void
ut_avlTreeFree (
    ut_avlTree this)
{
    assert(this != NULL);

    ut_avlNodeFree(this->root);
    os_free(this);
}
