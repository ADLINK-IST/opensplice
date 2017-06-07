inline os_os_uint32 os_ld32 (volatile os_os_uint32 *x)
{
    return atomic_load (&x->v);
}

#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_ld64 (volatile os_os_uint64 *x)
{
    return atomic_load (&x->v);
}
#endif

inline uintptr_t os_ldptr (volatile os_uintptr_t *x)
{
    return atomic_load (&x->v);
}

inline void *os_ldvoidp (volatile os_voidp_t *x)
{
    return (void *) os_ldptr (x);
}

inline void os_st32 (volatile os_os_uint32 *x, os_os_uint32 v)
{
    atomic_store (&x->v, v);
}

#if PA_ATOMIC64_SUPPORT
inline void os_st64 (volatile os_os_uint64 *x, os_os_uint64 v)
{
    atomic_store (&x->v, v);
}
#endif

inline void os_stptr (volatile os_uintptr_t *x, uintptr_t v)
{
    atomic_store (&x->v, v);
}

inline void os_stvoidp (volatile os_voidp_t *x, void *v)
{
    os_stptr (x, (uintptr_t) v);
}
