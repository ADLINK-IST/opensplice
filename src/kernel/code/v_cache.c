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


#include "v_cache.h"
#include "os_report.h"

#ifdef NDEBUG
#define v_cacheNodeCheck(c)
#else
void
v_cacheNodeCheck(
    v_cacheNode node)
{
    if (node->connections.next) {
        assert(v_cacheNode(node->connections.next)->connections.prev == node);
    }
    if (node->connections.prev) {
        assert(v_cacheNode(node->connections.prev)->connections.next == node);
    }
    if (node->targets.next) {
        assert(v_cacheNode(node->targets.next)->targets.prev == node);
    }
    if (node->targets.prev) {
        assert(v_cacheNode(node->targets.prev)->targets.next == node);
    }
}
#endif

static void
v_cacheNodeInit(
    v_cacheNode node)
{
    if (node) {
        node->connections.next = NULL;
        node->connections.prev = NULL;
        node->targets.next = NULL;
        node->targets.prev = NULL;
    }
}

v_cacheNode
v_cacheNodeNew (
    v_cache cache)
{
    v_cacheNode node;

    assert(C_TYPECHECK(cache,v_cache));

    node = c_new(cache->itemType);
    if (node) {
        v_cacheNodeInit(node);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_cacheNode::v_cacheNodeNew",0,
                  "Failed to allocate v_cacheNode object.");
        assert(FALSE);
    }
    return node;
}

v_cache
v_cacheNew (
    v_kernel kernel,
    c_type type,
    v_cacheKind kind)
{
    c_base base = NULL;
    v_cache cache = NULL;

    assert(C_TYPECHECK(cache,v_cache));
    assert(C_TYPECHECK(type,c_type));

    if (type) {
        base = c_getBase(type);
        if (base) {
            cache = c_new(v_kernelType(kernel,K_CACHE));
            if (cache) {
                cache->kind = kind;
                cache->itemType = c_keep(type);
                v_cacheNodeInit(v_cacheNode(cache));
            } else {
                OS_REPORT(OS_ERROR,
                          "v_cacheNode::v_cacheNew",0,
                          "Failed to allocate v_cache object.");
                assert(FALSE);
            }
        }
    }
    return cache;
}

void
v_cacheDeinit (
    v_cache cache)
{
    v_cacheNode node, next;

    assert(C_TYPECHECK(cache,v_cache));

    v_cacheNodeCheck(v_cacheNode(cache));
    switch (cache->kind) {
    case V_CACHE_CONNECTION:
        node = v_cacheNode(cache)->connections.next;
        while (node != NULL) {
            v_cacheNodeCheck(node);
            assert(C_TYPECHECK(node,v_cacheNode));
            next = node->connections.next;
            v_cacheNodeRemove(node,V_CACHE_ANY); /* also frees node. */
            node = v_cacheNode(cache)->connections.next;
            assert(next == node);
            v_cacheNodeCheck(v_cacheNode(cache));
        }
    break;
    case V_CACHE_TARGETS:
        node = v_cacheNode(cache)->targets.next;
        while (node != NULL) {
            v_cacheNodeCheck(node);
            assert(C_TYPECHECK(node,v_cacheNode));
            next = node->targets.next;
            v_cacheNodeRemove(node,V_CACHE_ANY); /* also frees node. */
            node = v_cacheNode(cache)->targets.next;
            assert(next == node);
            v_cacheNodeCheck(v_cacheNode(cache));
        }
    break;
    default:
        node = NULL;
    }
    v_cacheNodeCheck(v_cacheNode(cache));
}

void
v_cacheInsert (
    v_cache cache,
    v_cacheNode node)
{
    struct v_cacheLink *nodeLink;
    struct v_cacheLink *cacheLink;
    struct v_cacheLink *headLink;
    c_bool error = FALSE;

    assert(cache != NULL);
    assert(node != NULL);
    assert(C_TYPECHECK(cache,v_cache));
    assert(C_TYPECHECK(node,v_cacheNode));

    v_cacheNodeCheck(v_cacheNode(cache));
    v_cacheNodeCheck(node);
    switch (cache->kind) {
    case V_CACHE_CONNECTION:
        nodeLink = &node->connections;
        cacheLink = &v_cacheNode(cache)->connections;
        if (cacheLink->next != NULL) {
            headLink = &v_cacheNode(cacheLink->next)->connections;
            headLink->prev = node;
        }
    break;
    case V_CACHE_TARGETS:
        nodeLink = &node->targets;
        cacheLink = &v_cacheNode(cache)->targets;
        if (cacheLink->next != NULL) {
            headLink = &v_cacheNode(cacheLink->next)->targets;
            headLink->prev = node;
        }
    break;
    default:
        assert(FALSE);
        OS_REPORT_1(OS_ERROR, "v_cacheInsert", 0,
                    "Illegal value of cache->kind detected. (%d)",
                    cache->kind);
        error = TRUE;
    break;
    }
    if (!error) {
        assert(nodeLink->next == NULL);
        assert(nodeLink->prev == NULL);
        nodeLink->next = cacheLink->next;
        cacheLink->next = node;
        nodeLink->prev = v_cacheNode(cache);
        v_cacheNodeCheck(v_cacheNode(cache));
        v_cacheNodeCheck(node);
        c_keep(node);
    }
}

c_bool
v_cacheWalk (
    v_cache cache,
    v_cacheWalkAction action,
    c_voidp arg)
{
    v_cacheNode node;
    c_bool proceed;

    assert(C_TYPECHECK(cache,v_cache));

    v_cacheNodeCheck(v_cacheNode(cache));
    proceed = TRUE;
    switch (cache->kind) {
    case V_CACHE_CONNECTION:
        node = v_cacheNode(cache)->connections.next;
        while ((node != NULL) && (proceed)) {
            v_cacheNodeCheck(node);
            assert(C_TYPECHECK(node,v_cacheNode));
            proceed = action(node,arg);
            node = node->connections.next;
        }
    break;
    case V_CACHE_TARGETS:
        node = v_cacheNode(cache)->targets.next;
        while ((node != NULL) && (proceed)) {
            v_cacheNodeCheck(node);
            assert(C_TYPECHECK(node,v_cacheNode));
            proceed = action(node,arg);
            node = node->targets.next;
        }
    break;
    default:
    break;
    }
    v_cacheNodeCheck(v_cacheNode(cache));
    return proceed;
}

void
v_cacheNodeRemove (
    v_cacheNode node,
    v_cacheKind kind)
{
    struct v_cacheLink *nodeLink;

    assert(C_TYPECHECK(node,v_cacheNode));

    v_cacheNodeCheck(node);

    switch (kind) {
    case V_CACHE_CONNECTION:
        nodeLink = &node->connections;
        if (nodeLink->next != NULL) {
            v_cacheNode(nodeLink->next)->connections.prev = nodeLink->prev;
        }
        if (nodeLink->prev != NULL) {
            v_cacheNode(nodeLink->prev)->connections.next = nodeLink->next;
        }
        nodeLink->next = NULL;
        nodeLink->prev = NULL;
        v_cacheNodeCheck(node);
        c_free(node);
    break;
    case V_CACHE_TARGETS:
        nodeLink = &node->targets;
        if (nodeLink->next != NULL) {
            v_cacheNode(nodeLink->next)->targets.prev = nodeLink->prev;
        }
        if (nodeLink->prev != NULL) {
            v_cacheNode(nodeLink->prev)->targets.next = nodeLink->next;
        }
        nodeLink->next = NULL;
        nodeLink->prev = NULL;
        v_cacheNodeCheck(node);
        c_free(node);
    break;
    case V_CACHE_ANY:
        assert(c_refCount(node) > 1);
        v_cacheNodeRemove(node,V_CACHE_CONNECTION);
        v_cacheNodeRemove(node,V_CACHE_TARGETS);
    break;
    default:
    break;
    }
}

