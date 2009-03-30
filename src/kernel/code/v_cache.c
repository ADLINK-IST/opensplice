

#include "v_cache.h"

#ifdef NDEBUG
#define v_cacheNodeCheck(c)
#else
void
v_cacheNodeCheck(
    v_cacheNode node)
{
    if (node->owner.next) {
        assert(v_cacheNode(node->owner.next)->owner.prev == node);
    }
    if (node->owner.prev) {
        assert(v_cacheNode(node->owner.prev)->owner.next == node);
    }
    if (node->instance.next) {
        assert(v_cacheNode(node->instance.next)->instance.prev == node);
    }
    if (node->instance.prev) {
        assert(v_cacheNode(node->instance.prev)->instance.next == node);
    }
}
#endif

void
v_cacheNodeInit (
    v_cacheNode node)
{
    assert(C_TYPECHECK(node,v_cacheNode));

    node->owner.next = NULL;
    node->owner.prev = NULL;
    node->instance.next = NULL;
    node->instance.prev = NULL;
}

void
v_cacheInit (
    v_cache cache,
    v_cacheKind kind)
{
    assert(C_TYPECHECK(cache,v_cache));

    cache->kind = kind;
    v_cacheNodeInit(v_cacheNode(cache));
}

void
ownerCheck(
    v_cacheNode node)
{
    v_cacheNode next,prev;
    assert(node);
    assert(C_TYPECHECK(node,v_cacheNode));
   
    next = node->owner.next;
    prev = node;
    while (next != NULL) { 
        assert(next->owner.prev == prev);
        prev = next;
        next = next->owner.next;
    }
    prev = node->owner.prev;
    next = node;
    while (prev != NULL) { 
        assert(prev->owner.next == next);
        next = prev;
        prev = prev->owner.prev;
    }
}

void
instanceCheck (
    v_cacheNode node)
{
    v_cacheNode next,prev;
    assert(node);
    assert(C_TYPECHECK(node,v_cacheNode));
   
    next = node->instance.next;
    prev = node;
    while (next != NULL) { 
        assert(next->instance.prev == prev);
        prev = next;
        next = next->instance.next;
    }
    prev = node->instance.prev;
    next = node;
    while (prev != NULL) { 
        assert(prev->instance.next == next);
        next = prev;
        prev = prev->instance.prev;
    }
}

void
v_cacheDeinit (
    v_cache cache)
{
    v_cacheNode node;

    assert(C_TYPECHECK(cache,v_cache));

    v_cacheNodeCheck(v_cacheNode(cache));
    switch (cache->kind) {
    case V_CACHE_OWNER:
        node = v_cacheNode(cache)->owner.next;
    break;
    case V_CACHE_INSTANCE:
        node = v_cacheNode(cache)->instance.next;
    break;
    default:
        node = NULL;
    }
    while (node != NULL) {
        assert(C_TYPECHECK(node,v_cacheNode));
        v_cacheNodeRemove(node,V_CACHE_ANY); /* also frees node. */
        switch (cache->kind) {
        case V_CACHE_OWNER:
            node = v_cacheNode(cache)->owner.next;
        break;
        case V_CACHE_INSTANCE:
            node = v_cacheNode(cache)->instance.next;
        break;
        default:
            node = NULL;
        }
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

    assert(cache != NULL);
    assert(node != NULL);
    assert(C_TYPECHECK(cache,v_cache));
    assert(C_TYPECHECK(node,v_cacheNode));

    v_cacheNodeCheck(v_cacheNode(cache));
    v_cacheNodeCheck(node);
    switch (cache->kind) {
    case V_CACHE_OWNER:
        nodeLink = &node->owner;
        cacheLink = &v_cacheNode(cache)->owner;
        if (cacheLink->next != NULL) {
            headLink = &cacheLink->next->owner;
            headLink->prev = node;
        }
    break;
    case V_CACHE_INSTANCE:
        nodeLink = &node->instance;
        cacheLink = &v_cacheNode(cache)->instance;
        if (cacheLink->next != NULL) {
            headLink = &cacheLink->next->instance;
            headLink->prev = node;
        }
    break;
    default:
        assert(FALSE);
        nodeLink = NULL;
        cacheLink = NULL;
    break;
    }
    assert(nodeLink->next == NULL);
    assert(nodeLink->prev == NULL);
    nodeLink->next = cacheLink->next;
    cacheLink->next = c_keep(node);
    nodeLink->prev = v_cacheNode(cache);
    v_cacheNodeCheck(v_cacheNode(cache));
    v_cacheNodeCheck(node);
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
    case V_CACHE_OWNER:
        node = v_cacheNode(cache)->owner.next;
        while ((node != NULL) && (proceed)) {
            assert(C_TYPECHECK(node,v_cacheNode));
            proceed = action(node,arg);
            node = node->owner.next;
        }
    break;
    case V_CACHE_INSTANCE:
        node = v_cacheNode(cache)->instance.next;
        while ((node != NULL) && (proceed)) {
            assert(C_TYPECHECK(node,v_cacheNode));
            proceed = action(node,arg);
            node = node->instance.next;
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
    case V_CACHE_OWNER:
        nodeLink = &node->owner;
        if (nodeLink->next != NULL) {
            v_cacheNode(nodeLink->next)->owner.prev = nodeLink->prev;
        }
        if (nodeLink->prev != NULL) {
            v_cacheNode(nodeLink->prev)->owner.next = nodeLink->next;
            
        }
        nodeLink->next = NULL;
        nodeLink->prev = NULL;
        c_free(node);
    break;
    case V_CACHE_INSTANCE:
        nodeLink = &node->instance;
        if (nodeLink->next != NULL) {
            v_cacheNode(nodeLink->next)->instance.prev = nodeLink->prev;
        }
        if (nodeLink->prev != NULL) {
            v_cacheNode(nodeLink->prev)->instance.next = nodeLink->next;
        }
        nodeLink->next = NULL;
        nodeLink->prev = NULL;
        c_free(node);
    break;
    case V_CACHE_ANY:
        v_cacheNodeRemove(node,V_CACHE_INSTANCE);
        v_cacheNodeRemove(node,V_CACHE_OWNER);
    break;
    default:
    break;
    }
    v_cacheNodeCheck(node);
}

