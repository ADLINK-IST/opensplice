#ifndef C__LIST_TMPL_H
#define C__LIST_TMPL_H

#define C__LIST_TYPES_TMPL(prefix_, elemT_, extension_, batch_) \
struct prefix_##Node_s {                        \
    struct prefix_##Node_s *next;               \
    os_uint32 first, lastp1;                    \
    elemT_ ary[(batch_)];                       \
};                                              \
                                                \
struct prefix_##_s {                            \
    struct prefix_##Node_s *head;               \
    struct prefix_##Node_s *tail;               \
    os_uint32 count;                            \
    extension_                                  \
};                                              \
                                                \
struct prefix_##Iter_s {                        \
    struct prefix_##Node_s *node;               \
    os_uint32 idx;                              \
};                                              \
                                                \
struct prefix_##IterD_s {                       \
    struct prefix_##_s *list;                   \
    struct prefix_##Node_s *node;               \
    struct prefix_##Node_s *prev;               \
    os_uint32 idx;                              \
};

#ifndef NDEBUG
#define C__LIST_TMPL_POISON(x) do { x = (void *)1; } while (0)
#else
#define C__LIST_TMPL_POISON(x) do {} while (0)
#endif

#define C__LIST_DECLS_TMPL(linkage_, prefix_, elemT_, attrs_) \
linkage_ void prefix_##Init (struct prefix_##_s *list); \
linkage_ void prefix_##Free (struct prefix_##_s *list) attrs_; \
linkage_ elemT_ prefix_##Insert (struct prefix_##_s *list, elemT_ o) attrs_; \
linkage_ elemT_ prefix_##Append (struct prefix_##_s *list, elemT_ o) attrs_; \
linkage_ elemT_ prefix_##IterFirst (const struct prefix_##_s *list, struct prefix_##Iter_s *iter) attrs_; \
linkage_ elemT_ prefix_##IterNext (struct prefix_##Iter_s *iter) attrs_; \
linkage_ elemT_ *prefix_##IterElemAddress (struct prefix_##Iter_s *iter) attrs_; \
linkage_ elemT_ prefix_##IterDFirst (struct prefix_##_s *list, struct prefix_##IterD_s *iter) attrs_; \
linkage_ elemT_ prefix_##IterDNext (struct prefix_##IterD_s *iter) attrs_; \
linkage_ void prefix_##IterDRemove (struct prefix_##IterD_s *iter) attrs_; \
linkage_ elemT_ prefix_##Remove (struct prefix_##_s *list, elemT_ o) attrs_; \
linkage_ elemT_ prefix_##TakeFirst (struct prefix_##_s *list) attrs_; \
linkage_ elemT_ prefix_##TakeLast (struct prefix_##_s *list) attrs_; \
linkage_ os_uint32 prefix_##Count (const struct prefix_##_s *list) attrs_; \
linkage_ void prefix_##AppendList (struct prefix_##_s *list, struct prefix_##_s *b) attrs_; \
linkage_ elemT_ prefix_##Index (struct prefix_##_s *list, os_uint32 index) attrs_; \
linkage_ elemT_ prefix_##Index (struct prefix_##_s *list, os_uint32 index) attrs_;

#define C__LIST_CODE_TMPL(linkage_, prefix_, elemT_, null_, equals_, malloc_, free_) \
linkage_ void prefix_##Init (struct prefix_##_s *list)                  \
{                                                                       \
    list->head = NULL;                                                  \
    list->tail = NULL;                                                  \
    list->count = 0;                                                    \
}                                                                       \
                                                                        \
linkage_ void prefix_##Free (struct prefix_##_s *list)                  \
{                                                                       \
    /* Note: just free, not re-init */                                  \
    struct prefix_##Node_s *n;                                          \
    while ((n = list->head) != NULL) {                                  \
        list->head = n->next;                                           \
        os_free (n);                                                    \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##Insert (struct prefix_##_s *list, elemT_ o)    \
{                                                                       \
    struct prefix_##Node_s *n;                                          \
    const os_uint32 bs = (os_uint32) (sizeof (n->ary) / sizeof (n->ary[0])); \
    if (list->head != NULL && list->head->first > 0) {                  \
        n = list->head;                                                 \
    } else {                                                            \
        if ((n = (malloc_ (sizeof (struct prefix_##Node_s)))) == NULL) { \
            return null_;                                               \
        }                                                               \
        n->next = list->head;                                           \
        n->first = n->lastp1 = bs;                                      \
        if (list->head == NULL) {                                       \
            list->tail = n;                                             \
        }                                                               \
        list->head = n;                                                 \
    }                                                                   \
    n->ary[--n->first] = o;                                             \
    list->count++;                                                      \
    return o;                                                           \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##Append (struct prefix_##_s *list, elemT_ o)    \
{                                                                       \
    struct prefix_##Node_s *n;                                          \
    const os_uint32 bs = (os_uint32) (sizeof (n->ary) / sizeof (n->ary[0])); \
    if (list->head != NULL && list->tail->lastp1 < bs) {                \
        n = list->tail;                                                 \
    } else {                                                            \
        if ((n = (malloc_ (sizeof (struct prefix_##Node_s)))) == NULL) { \
            return null_;                                               \
        }                                                               \
        n->next = NULL;                                                 \
        n->first = n->lastp1 = 0;                                       \
        if (list->head == NULL) {                                       \
            list->head = n;                                             \
        } else {                                                        \
            list->tail->next = n;                                       \
        }                                                               \
        list->tail = n;                                                 \
    }                                                                   \
    n->ary[n->lastp1++] = o;                                            \
    list->count++;                                                      \
    return o;                                                           \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##IterFirst (const struct prefix_##_s *list, struct prefix_##Iter_s *iter) \
{                                                                       \
    iter->node = list->head;                                            \
    if (iter->node == NULL) {                                           \
        iter->idx = 0;                                                  \
        return null_;                                                   \
    } else {                                                            \
        iter->idx = iter->node->first;                                  \
        if (iter->node->first < iter->node->lastp1) {                   \
            return iter->node->ary[iter->idx];                          \
        } else {                                                        \
            return null_;                                               \
        }                                                               \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##IterNext (struct prefix_##Iter_s *iter)        \
{                                                                       \
    /* You MAY NOT call ut_listIterNext after having received a null    \
     * pointer from IterFirst or IterNext */                            \
    assert (iter->node != NULL);                                        \
    if (iter->idx+1 < iter->node->lastp1) {                             \
        return iter->node->ary[++iter->idx];                            \
    } else if (iter->node->next == NULL) {                              \
        return null_;                                                   \
    } else {                                                            \
        iter->node = iter->node->next;                                  \
        iter->idx = iter->node->first;                                  \
        return iter->node->ary[iter->idx];                              \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ *prefix_##IterElemAddress (struct prefix_##Iter_s *iter) \
{                                                                       \
    assert (iter->node != NULL);                                        \
    return &iter->node->ary[iter->idx];                                 \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##IterDFirst (struct prefix_##_s *list, struct prefix_##IterD_s *iter) \
{                                                                       \
    iter->list = list;                                                  \
    iter->node = list->head;                                            \
    iter->prev = NULL;                                                  \
    if (iter->node == NULL) {                                           \
        iter->idx = 0;                                                  \
        return null_;                                                   \
    } else {                                                            \
        iter->idx = iter->node->first;                                  \
        if (iter->node->first < iter->node->lastp1) {                   \
            return iter->node->ary[iter->idx];                          \
        } else {                                                        \
            return null_;                                               \
        }                                                               \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##IterDNext (struct prefix_##IterD_s *iter)      \
{                                                                       \
    /* You MAY NOT call ut_listIterDNext after having received a null   \
     * pointer from IterDFirst or IterDNext */                          \
    if (iter->node == NULL) {                                           \
        return prefix_##IterDFirst (iter->list, iter);                  \
    } else if (iter->idx+1 < iter->node->lastp1) {                      \
        return iter->node->ary[++iter->idx];                            \
    } else if (iter->node->next == NULL) {                              \
        return null_;                                                   \
    } else {                                                            \
        iter->prev = iter->node;                                        \
        iter->node = iter->node->next;                                  \
        iter->idx = iter->node->first;                                  \
        return iter->node->ary[iter->idx];                              \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ void prefix_##IterDRemove (struct prefix_##IterD_s *iter)      \
{                                                                       \
    struct prefix_##_s * const list = iter->list;                       \
    struct prefix_##Node_s * const n = iter->node;                      \
    os_uint32 j;                                                        \
    list->count--;                                                      \
    for (j = iter->idx; j > n->first; j--) {                            \
        n->ary[j] = n->ary[j-1];                                        \
    }                                                                   \
    n->first++;                                                         \
    if (n->first == n->lastp1) {                                        \
        if (n == list->tail) {                                          \
            list->tail = iter->prev;                                    \
        }                                                               \
        if (iter->prev) {                                               \
            iter->prev->next = n->next;                                 \
            iter->node = iter->prev;                                    \
            iter->idx = iter->prev->lastp1;                             \
            C__LIST_TMPL_POISON(iter->prev);                            \
        } else {                                                        \
            list->head = n->next;                                       \
            iter->node = NULL; /* removed first entry, restart */       \
        }                                                               \
        free_ (n);                                                      \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##Remove (struct prefix_##_s *list, elemT_ o)    \
{                                                                       \
    struct prefix_##IterD_s iter;                                       \
    elemT_ obj;                                                         \
    for (obj = prefix_##IterDFirst (list, &iter); !(equals_ (obj, null_)); obj = prefix_##IterDNext (&iter)) { \
        if (equals_ (obj, o)) {                                         \
            prefix_##IterDRemove (&iter);                               \
            return obj;                                                 \
        }                                                               \
    }                                                                   \
    return null_;                                                       \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##TakeFirst (struct prefix_##_s *list)           \
{                                                                       \
    if (list->count == 0) {                                             \
        return null_;                                                   \
    } else {                                                            \
        struct prefix_##IterD_s iter;                                   \
        elemT_ obj = prefix_##IterDFirst (list, &iter);                 \
        prefix_##IterDRemove (&iter);                                   \
        return obj;                                                     \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##TakeLast (struct prefix_##_s *list)            \
{                                                                       \
    if (list->count == 0) {                                             \
        return null_;                                                   \
    } else {                                                            \
        struct prefix_##IterD_s iter;                                   \
        os_uint32 i;                                                    \
        elemT_ obj;                                                     \
        obj = prefix_##IterDFirst (list, &iter);                        \
        for (i = 0; i < list->count - 1; i++) {                         \
            obj = prefix_##IterDNext (&iter);                           \
        }                                                               \
        prefix_##IterDRemove (&iter);                                   \
        return obj;                                                     \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ os_uint32 prefix_##Count (const struct prefix_##_s *list)      \
{                                                                       \
    return list->count;                                                 \
}                                                                       \
                                                                        \
linkage_ void prefix_##AppendList (struct prefix_##_s *list, struct prefix_##_s *b) \
{                                                                       \
    if (list->head == NULL) {                                           \
        *list = *b;                                                     \
    } else if (b->head != NULL) {                                       \
        list->tail->next = b->head;                                     \
        list->tail = b->tail;                                           \
        list->count += b->count;                                        \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ *prefix_##IndexAddress (struct prefix_##_s *list, os_uint32 index) \
{                                                                       \
    struct prefix_##Node_s *n;                                          \
    os_uint32 pos = 0;                                                  \
    if (index >= list->count) {                                         \
        return NULL;                                                    \
    } else if (index == list->count - 1) {                              \
        n = list->tail;                                                 \
        pos = list->count - (n->lastp1 - n->first);                     \
    } else {                                                            \
        for (n = list->head; n; n = n->next) {                          \
            const os_uint32 c = n->lastp1 - n->first;                   \
            if (pos + c > index) {                                      \
                break;                                                  \
            } else {                                                    \
                pos += c;                                               \
            }                                                           \
        }                                                               \
    }                                                                   \
    if (n == NULL) {                                                    \
        return NULL;                                                    \
    } else {                                                            \
        assert (pos <= index && index < pos + n->lastp1 - n->first);    \
        return &n->ary[n->first + (index - pos)];                       \
    }                                                                   \
}                                                                       \
                                                                        \
linkage_ elemT_ prefix_##Index (struct prefix_##_s *list, os_uint32 index) \
{                                                                       \
    elemT_ *p = prefix_##IndexAddress (list, index);                    \
    if (p == NULL) {                                                    \
        return null_;                                                   \
    } else {                                                            \
        return *p;                                                      \
    }                                                                   \
}

#endif
