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
	struct a_treNode *right;
	int               balanceFactor;
} *a_treNode;



/**
 * \brief
 * Tree Type
 *
 * Abstract Data Type for an AVL-Tree Implementation.
 */
struct a_treTree_s {
	a_treNode root;
};



/***********************************************************
 CREATION AND DESTROY
 ***********************************************************/

/**
 * \brief
 * Creates a new tree and returns a pointer to that tree.
 *
 * This operation creates a new tree and returns a pointer to that
 * tree. If no memory could be allocated, i.e. if malloc failed,
 * NULL is returned.
 *
 * \return
 * Pointer to the newly created tree, or NULL if tree creation
 * failed.
 */
a_treTree
a_treCreateTree()
{
	a_treTree tree = a_memAlloc(sizeof(struct a_treTree_s));
	if (tree) {
		tree->root = NULL;
	}
	return tree;
}



/* Removes nodes recursively, specified by the (sub)tree's root.
 */
static void
a_treRemoveNodeAndBelow(
	a_treNode *rootNode,
	a_treFreeAction freeAction)
{
	if ((*rootNode)->left) {
		a_treRemoveNodeAndBelow(&((*rootNode)->left), freeAction);
	}
	if ((*rootNode)->right) {
		a_treRemoveNodeAndBelow(&((*rootNode)->right), freeAction);
	}
	if (freeAction) {
		(freeAction)(&((*rootNode)->value));
	}
	a_memFree(*rootNode);
}



/**
 * \brief
 * Destroys a tree, and frees its memory.
 *
 * This operation destroys a tree and all its nodes. A call is made
 * to a user defined function to destroy the user defined data,
 * related to that node.\n
 * This user defined function must be of type \a a_treFreeAction.
 *
 * \param tree
 * The tree to destroy. If tree = NULL, this operation will fail.
 *
 * \param freeAction
 * A user defined function that will destroy the user defined data.
 * freeAction may be NULL. In that case, only the tree itself will be
 * destroyed.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_treFreeAction
 */
int
a_treDestroyTree(
	a_treTree tree,
	a_treFreeAction freeAction)
{
	int result = 0;
	if (tree) {
		if (tree->root) {
			a_treRemoveNodeAndBelow(&(tree->root), freeAction);
			tree->root = NULL;
		}
		a_memFree(tree);
		result++;
	}
	return result;
}




/***********************************************************
 INTERNAL AIDS FOR INSERTING
 ***********************************************************/


/* Returns the greater number of two ints
 */
static int
a_treMax(
	int x,
	int y)
{
	return x < y ? y : x;
}



/* Returns the tree depth of a node
 */
static int
a_treNodeDepth(
	a_treNode node)
{
	return node ? 1 + a_treMax(a_treNodeDepth(node->left), a_treNodeDepth(node->right)) : 0;
}



/* Returns the balance factor of a node, which is the difference
 * in tree depth of its subtrees (right minus left)
 */
static int
a_treBalanceFactor(
	a_treNode node)
{
	return a_treNodeDepth(node->right) - a_treNodeDepth(node->left);
}



/* Rotates a tree to the right and updates the balance factors
 */
static void
a_treRotateRight(
	a_treNode *node)
{
	a_treNode nodeX = (*node)->left;
	a_treNode nodeB = nodeX->right;

	nodeX->right = *node;
	(*node)->left = nodeB;
	*node = nodeX;
	
	(*node)->right->balanceFactor = a_treBalanceFactor((*node)->right);
	(*node)->balanceFactor = a_treBalanceFactor(*node);
}

	

/* Rotates a tree to the left and updates the balance factors
 */
static void
a_treRotateLeft(
	a_treNode *node)
{
	a_treNode nodeY = (*node)->right;
	a_treNode nodeB = nodeY->left;
	
	nodeY->left = *node;
	(*node)->right = nodeB;
	*node = nodeY;
	
	(*node)->left->balanceFactor = a_treBalanceFactor((*node)->left);
	(*node)->balanceFactor = a_treBalanceFactor(*node);
}



/* Rebalance a (sub)tree, specified by its root node.
 * Handles 4 different cases for rebalancing:
 *   Node --, left child +  : L(child)  R(node)
 *   Node --, otherwise     :           R(node)
 *   Node ++, right child - : R(child), L(node)
 *   Node ++, otherwise     :           L(node)
 * where:
 *   -         = left heavy
 *   --        = double left heavy
 *   +         = right heavy
 *   ++        = double right heavy
 *   L(node)   = left rotation on node
 *   R(node)   = right rotation on node
 */
static void
a_treRebalanceNode(
	a_treNode *node)
{
	if ((*node)->balanceFactor < -1) {
		if (0 < (*node)->left->balanceFactor) {
			a_treRotateLeft(&(*node)->left);
		}
		a_treRotateRight(&(*node));
	} else if (1 < (*node)->balanceFactor) {
		if ((*node)->right->balanceFactor < 0) {
			a_treRotateRight(&(*node)->right);
		}
		a_treRotateLeft(&(*node));
	}
}	



/* Creates a new node and returns a pointer to that node.
 * Returns NULL if the malloc failed.
 */
static a_treNode
a_treNewNode(
	void *valuePtr)
{
	a_treNode node = a_memAlloc(sizeof(struct a_treNode));
	if (node) {
		node->value = valuePtr;
		node->left  = NULL;
		node->right = NULL;
		node->balanceFactor = 0;
	}
	return node;
}
	


/***********************************************************
 INTERT, FIND, DELETE
 ***********************************************************/


/* Inserts a new node into a (sub)tree
 * Assumptions:
 *   - root    != NULL, root->value    != NULL
 *   - newNode != NULL, newNode->value != NULL
 *   - sortAction returns -1 if <
 *                         0    ==
 *                         1    >
 */
static int
a_treInsertNodeIntoTree(
	a_treNode *node,
	a_treNode newNode,
	a_treSortAction sortAction,
	int dupesAllowed)
{
	int result = 0;
	int sortResult = (sortAction)((*node)->value, newNode->value);
	if ( (-1 <= sortResult) && (sortResult <= 1) ) {
	
		if ( (sortResult == -1) || ((sortResult == 0) && dupesAllowed) ) {	// insert left
			if ((*node)->left == NULL) {    // insert left
				(*node)->left = newNode;
				result++;
			} else {
				result = a_treInsertNodeIntoTree(&(*node)->left, newNode, sortAction, dupesAllowed);
			}
		} else if (sortResult == 1) {       // insert right
			if ((*node)->right == NULL) {
				(*node)->right = newNode;
				result++;
			} else {
				result = a_treInsertNodeIntoTree(&(*node)->right, newNode, sortAction, dupesAllowed);
			}
		}
		(*node)->balanceFactor = a_treBalanceFactor(*node);
		a_treRebalanceNode(&(*node));

	} else {  // may not occur
		// assert(0);
	}
	return result;
}



/**
 * \brief
 * Creates a new node and inserts that node into the list.
 *
 * This operation creates a new node (internally) and inserts it,
 * with a pointer to the user defined data structure, ordered into
 * the list.
 *
 * \param tree
 * The tree into which the new node must be created. If tree is NULL,
 * the operation will fail.
 *
 * \param valuePtr
 * Pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to determine its sort order.
 *
 * \param dupesAllowed
 * Boolean setting specifying whether duplicate entries are allowed.
 * If a duplicate is detected while this setting is false, the
 * operation will fail. Remember that in that case the user defined
 * data structure will not have a reference from this list. You'll
 * need to store the reference yourself, or destroy the data and free
 * up memory in order to avoid memory leaks.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_treSortAction
 */
int
a_treInsertNode(
	a_treTree tree,
	void *valuePtr,
	a_treSortAction sortAction,
	int dupesAllowed)
{
	int result = 0;
	a_treNode node = a_treNewNode(valuePtr);
	if (!tree->root) {
		tree->root = node;
		result++;
	} else {
		result = a_treInsertNodeIntoTree(&(tree->root), node, sortAction, dupesAllowed);
	}
	return result;
}



/* Recursive search in tree
 */
static a_treNode
a_treFindNode(
	a_treNode node,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode result = NULL;
	if (node) {
		int sortResult = (sortAction)(node->value, valuePtr);
		if (sortResult == 0) {      // match!
			result = node;
		} else if (sortResult < 0) {
			result = a_treFindNode(node->left, valuePtr, sortAction);
		} else if (0 < sortResult) {
			result = a_treFindNode(node->right, valuePtr, sortAction);
		}
	}
	return result;
}



/**
 * \brief
 * Searches for a value in a tree.
 *
 * This operation searches within the tree for a node with a value,
 * that is the same of valuePtr, and returns a pointer to the value
 * that the (internal) node holds.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Returns a pointer to the found value or NULL if not found.
 *
 * \see
 * a_treSortAction
 */
void *
a_treFindValue(
	a_treTree tree,
	void *valuePtr,
	a_treSortAction sortAction)
{
	void *result = NULL;
	if (tree) {
		a_treNode node = a_treFindNode(tree->root, valuePtr, sortAction);
		if (node) {
			result = node->value;
		}
	}
	return result;
}



/* Finds the first node by searching the left branch of a node
 * recursively.  The first node in the list is the far most left
 * leafnode in the tree. Returns NULL if node == NULL.
 */
static a_treNode
a_treFindFirstNode(
	a_treNode node)
{
	a_treNode result;
	if (node) {
		result = node->left ? a_treFindFirstNode(node->left) : node;
	} else {
		result = NULL;
	}
	return result;
}



/**
 * \brief
 * Finds and returns the first entry in the tree, that is defined as
 * the lowest ordered.
 *
 * This operation returns a pointer to the user defined data
 * structure that is the lowest in order, or NULL if the tree is
 * empty. The (sort)order of the tree was determined at every
 * insertion of a value. (Internally, the value of the left most
 * (semi) leaf node is returned.)
 *
 * \param tree
 * The tree in which the first value must be found and returned.
 *
 * \return
 * Pointer to the user defined data structure of the first value, or
 * NULL if the list is empty.
 *
 * \see
 * a_treFindLastValue
 */
void *
a_treFindFirstValue(
	a_treTree tree)
{
	a_treNode node = NULL;
	if (tree) {
		node = a_treFindFirstNode(tree->root);
	}
	return node ? node->value : NULL;
}



/* Finds the last node by searching the right branch of a node
 * recursively.  The last node in the list is the far most right
 * leafnode in the tree. Returns NULL if node == NULL.
 */
static a_treNode
a_treFindLastNode(
	a_treNode node)
{
	a_treNode result;
	if (node) {
		result = node->right ? a_treFindLastNode(node->right) : node;
	} else {
		result = NULL;
	}
	return result;
}



/**
 * \brief
 * Finds and returns the last entry in the tree, that is defined as
 * the highest ordered.
 *
 * This operation returns a pointer to the user defined data
 * structure that is the highest in order, or NULL if the tree is
 * empty. The (sort)order of the tree was determined at every
 * insertion of a value. (Internally, the value of the right most
 * (semi) leaf node is returned.)
 *
 * \param tree
 * The tree in which the last value must be found and returned.
 *
 * \return
 * Pointer to the user defined data structure of the last value, or
 * NULL if the list is empty.
 *
 * \see
 * a_treFindFirstValue
 */
void *
a_treFindLastValue(
	a_treTree tree)
{
	a_treNode node = NULL;
	if (tree) {
		node = a_treFindLastNode(tree->root);
	}
	return node ? node->value : NULL;
}



/*
 */
static a_treNode
a_treFindLowestHigherNode(
	a_treNode node,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode result;
	
	int sortResult = (sortAction)(valuePtr, node->value);
	if ( sortResult == 1 ) {        //  valuePtr < node->value
		
		if (node->left == NULL ) {
			result = node;
		} else if ( (sortAction)(valuePtr, node->left->value) == 1 ) {
			result = a_treFindLowestHigherNode(node->left, valuePtr, sortAction);
		} else if ( node->left->right == NULL ) {
			result = node;
		} else if ( (sortAction)(node->left->right->value, valuePtr) == 1 ) {
			result = node;
		} else {
			result = a_treFindLowestHigherNode(node->left, valuePtr, sortAction);
		}
		
	} else if ( (sortResult == -1) || (sortResult == 0) ) {
	
		if (node->right == NULL) {
			result = NULL;
		} else {
			result = a_treFindLowestHigherNode(node->right, valuePtr, sortAction);
		}
	
	} else {  // invalid sortResult;
		result = NULL;
	}
	
	return result;
}



/**
 * \brief
 * Searches for the nearest, higher user defined data structure in a
 * tree, specified by a pointer to an search instance.
 *
 * This operation searches within a tree for the nearest, higher (in
 * order) data structure, not being equal to the given search pointer.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found, tree is empty or
 * tree = NULL.
 *
 * \see
 * a_treSortAction a_treFindLowestHigherValue
 */
void *
a_treFindLowestHigherValue(
	a_treTree tree,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode node;
	if (tree) {
		node = a_treFindLowestHigherNode(tree->root, valuePtr, sortAction);
	} else {
		node = NULL;
	}
	return node ? node->value : NULL;
}



/*
 */
static a_treNode
a_treFindHighestLowerNode(
	a_treNode node,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode result;
	
	int sortResult = (sortAction)(node->value, valuePtr);
	if ( sortResult == 1 ) {        //  node->value < valuePtr
		
		if (node->right == NULL ) {
			result = node;
		} else if ( (sortAction)(node->right->value, valuePtr) == 1 ) {
			result = a_treFindHighestLowerNode(node->right, valuePtr, sortAction);
		} else if ( node->right->left == NULL ) {
			result = node;
		} else if ( (sortAction)(valuePtr, node->right->value) == 1 ) {
			result = node;
		} else {
			result = a_treFindHighestLowerNode(node->right, valuePtr, sortAction);
		}
		
	} else if ( (sortResult == -1) || (sortResult == 0) ) {
	
		if (node->left == NULL) {
			result = NULL;
		} else {
			result = a_treFindHighestLowerNode(node->left, valuePtr, sortAction);
		}
	
	} else {  // invalid sortResult;
		result = NULL;
	}
	
	return result;
}



/**
 * \brief
 * Searches for the nearest, lower user defined data structure in a
 * tree, specified by a pointer to an search instance.
 *
 * This operation searches within a tree for the nearest, lower (in
 * order) data structure, not being equal to the given search pointer.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found, tree is empty or
 * tree = NULL.
 *
 * \see
 * a_treSortAction a_treFindLowestHigherValue
 */
void *
a_treFindHighestLowerValue(
	a_treTree tree,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode node;
	if (tree) {
		node = a_treFindHighestLowerNode(tree->root, valuePtr, sortAction);
	} else {
		node = NULL;
	}
	return node ? node->value : NULL;
}



/* Searches for a node, specified by the user defined pointer
 * value, and, if found, removes the node from the tree,
 * rebalancing afterwards.
 */
static a_treNode
a_treFindAndRemoveNode(
	a_treNode *parent,
	a_treNode node,
	void *valuePtr,
	a_treSortAction sortAction)
{
	a_treNode result = NULL;
	if (node) {
		int sortResult = (sortAction)(node->value, valuePtr);
		if (sortResult == 0) {      // match!
			result = node;
			if (!node->right) {                   // case#1 node has no right child
				*parent = node->left;
			} else if (!node->right->left) {      // case#2 node's right child has no left child
				node->right->left = node->left;
				*parent = node->right;
			} else {                              // case#3 node's right child has a left child
				a_treNode inorderSuccessorParent = node->right;
				a_treNode inorderSuccessor;
				while (inorderSuccessorParent->left->left) {
					inorderSuccessorParent = inorderSuccessorParent->left;
				}
				inorderSuccessor = inorderSuccessorParent->left;
				inorderSuccessor->left = node->left;
				inorderSuccessorParent->left = inorderSuccessor->right;   // might be NULL
				inorderSuccessor->right = node->right;
				*parent = inorderSuccessor;
			}
			result->left = NULL;
			result->right = NULL;
		} else if (sortResult < 0) {
			result = a_treFindAndRemoveNode((a_treNode *)&(node->left), node->left, valuePtr, sortAction);
		} else if (0 < sortResult) {
			result = a_treFindAndRemoveNode((a_treNode *)&(node->right), node->right, valuePtr, sortAction);
		}
		if (result) {
			if (*parent) {
				(*parent)->balanceFactor = a_treBalanceFactor(*parent);
				a_treRebalanceNode(&(*parent));
			}
		}
	}
	return result;
}



/**
 * \brief
 * Searches for a value in a tree and, if found, removes the
 * corresponding node that held a pointer to that value.
 *
 * This operation searches within the tree for a node with a value,
 * that is the same of valuePtr, and returns a pointer to the found
 * user defined data structure. The (internal) node that held this
 * pointer will be removed from the tree and its memory will be
 * freed. If needed, the tree will be rebalanced.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \note
 * Remember to destroy the user defined data structure yourself, as
 * after this point the tree holds no reference to it anymore.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found.
 *
 * \see
 * a_treSortAction
 */
void *
a_treFindAndRemoveValue(
	a_treTree tree,
	void *valuePtr,
	a_treSortAction sortAction)
{
	void *result = NULL;
	if (tree) {
		a_treNode node = a_treFindAndRemoveNode((a_treNode *)&(tree->root), tree->root, valuePtr, sortAction);
		if (node) {
			result = node->value;
			a_memFree(node);
		}
	}
	return result;
}



/***********************************************************
 COUNT
 ***********************************************************/


/* Counts the nodes of a (sub)tree recursively, specified
 * by its root node.
 */
static int
a_treCountNodes(
	a_treNode node)
{
	int result = 0;
	if (node != NULL) {
		result = a_treCountNodes(node->left) + 1 + a_treCountNodes(node->right);
	}
	return result;
}



/**
 * \brief
 * Returns the number of elements in a tree.
 *
 * This operation counts the nodes in a tree (recursively) and
 * returns the result.
 *
 * \param tree
 * The tree to count its nodes of.
 *
 * \return
 * The number of elements in a tree, 0 if tree is empty (duh!) or -1
 * if tree = NULL.
 */
int
a_treCount(
	a_treTree tree)
{
	int result;
	if (tree) {
		result = a_treCountNodes(tree->root);
	} else {
		result = -1;
	}
	return result;
}



/***********************************************************
 WALK
 ***********************************************************/


/* Walks recursively a (sub)tree by calling treeAction with
 * the left subtree, the node's value and the right subtree.
 * Is used by a_treTreeWalk.
 */
static int
a_treWalkNode(
	a_treNode node,
	a_treWalkAction walkAction,
	void *arg)
{
	int result;
	if (walkAction) {
		if (node) {
			result = 1;
			if (node->left) {
				result = a_treWalkNode(node->left, walkAction, arg);
			}
			if (result) {
				result = (walkAction)(node->value, arg);
			}
			if (result && (node->right)) {
				result = a_treWalkNode(node->right, walkAction, arg);
			}
		} else {
			result = 0;
		}
	} else {
		result = -1;
	}
	return result;
}



/**
 * \brief
 * Performs a walk over all nodes and calls a user defined function
 * with a pointer to the user defined data structure that the node
 * holds.
 *
 * This operation walks over all nodes, in a sorted order (from low
 * to high), and calls a user defined function for every node,
 * passing on a pointer to the user defined data structure that the
 * node holds, along with a pointer to a user defined context.
 *
 * \param tree
 * The tree to walk over.
 *
 * \param walkAction
 * Pointer to a user defined function to call every node's value
 * pointer with, along with the pointer to the user defined context
 * \a arg.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along to the
 * user defined call back function, without any modification. May be
 * NULL.
 *
 * \return
 * Returns true (1) if the walk was successful, false (0) if the walk
 * was aborted or -1 if tree or walkAction NULL.
 *
 * \see
 * a_treWalkAction
 */
int
a_treWalk(
	a_treTree tree,
	a_treWalkAction walkAction,
	void *arg)
{
	return tree ? a_treWalkNode(tree->root, walkAction, arg) : -1;
}



/* Walks recursively a (sub)tree in reverse order by calling treeAction
 * with the left subtree, the node's value and the right subtree.
 * Is used by a_treTreeWalkReverse.
 * For test purposes only.
 */
static int
a_treWalkReverseNode(
	a_treNode node,
	a_treWalkReverseAction walkAction,
	void *arg,
	int depth)
{
	int result;
	if (walkAction) {
		if (node) {
			a_treWalkReverseNode(node->right, walkAction, arg, depth + 1);
			result = (walkAction)(node->value, arg, depth);
			a_treWalkReverseNode(node->left, walkAction, arg, depth + 1);
		} else {
			result = 1;
		}
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Performs a walk over all nodes in reverse order and calls a user
 * defined function with a pointer to the user defined data structure
 * that the node holds.
 *
 * This operation walks over all nodes, in a reverse order (from high
 * to low), and calls a user defined function for every node, passing
 * on a pointer to the user defined data structure that the node
 * holds, along with a pointer to a user defined context.
 *
 * \param tree
 * The tree to walk over.
 *
 * \param walkAction
 * Pointer to a user defined function to call every node's value
 * pointer with, along with the pointer to the user defined context
 * \a arg.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along to the
 * user defined call back function, without any modification. May be
 * NULL.
 *
 * \return
 * Returns true (1) if the walk was successful, false (0) if the walk
 * was aborted or -1 if tree or walkAction is NULL.
 *
 * \see
 * a_treWalkreverseAction
 */
int
a_treWalkReverse(
	a_treTree tree,
	a_treWalkReverseAction walkAction,
	void *arg)
{
	int result;
	if (tree) {
		result = a_treWalkReverseNode(tree->root, walkAction, arg, 0);
	} else {
		result = -1;
	}
	return result;
}



//END a_tre.c
