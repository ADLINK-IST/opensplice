/* INC */

inline void os_inc32(volatile os_os_uint32* x)
{
    atomic_fetch_add(&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
inline void os_inc64(volatile os_os_uint64* x)
{
    atomic_fetch_add(&x->v, 1);
}
#endif
inline void os_incptr(volatile os_uintptr* x)
{
    atomic_fetch_add(&x->v, 1);
}
inline os_os_uint32 os_inc32_nv(volatile os_os_uint32* x)
{
    return atomic_fetch_add(&x->v, 1) + 1;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_inc64_nv(volatile os_os_uint64* x)
{
    return atomic_fetch_add(&x->v, 1) + 1;
}
#endif
inline uintptr_t os_incptr_nv(volatile os_uintptr* x)
{
    return atomic_fetch_add(&x->v, 1) + 1;
}

/* DEC */

inline void os_dec32(volatile os_os_uint32* x)
{
    atomic_fetch_sub(&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
inline void os_dec64(volatile os_os_uint64* x)
{
    atomic_fetch_sub(&x->v, 1);
}
#endif
inline void os_decptr(volatile os_uintptr* x)
{
    atomic_fetch_sub(&x->v, 1);
}
inline os_os_uint32 os_dec32_nv(volatile os_os_uint32* x)
{
    return atomic_fetch_sub(&x->v, 1) - 1;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_dec64_nv(volatile os_os_uint64* x)
{
    return atomic_fetch_sub(&x->v, 1) - 1;
}
#endif
inline uintptr_t os_decptr_nv(volatile os_uintptr* x)
{
    return atomic_fetch_sub(&x->v, 1) - 1;
}

/* ADD */

inline void os_add32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_fetch_add(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline void os_add64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_fetch_add(&x->v, v);
}
#endif
inline void os_addptr(volatile os_uintptr* x, uintptr_t v)
{
    atomic_fetch_add(&x->v, v);
}
inline void os_addvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    os_addptr(x, (uintptr_t) v);
}
inline os_os_uint32 os_add32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_add(&x->v, v) + v;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_add64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_add(&x->v, v) + v;
}
#endif
inline uintptr_t os_addptr_nv(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_add(&x->v, v) + v;
}
inline void* os_addvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return (void*) os_addptr_nv(x, (uintptr_t) v);
}

/* SUB */

inline void os_sub32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_fetch_sub(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline void os_sub64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_fetch_sub(&x->v, v);
}
#endif
inline void os_subptr(volatile os_uintptr* x, uintptr_t v)
{
    atomic_fetch_sub(&x->v, v);
}
inline void os_subvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    os_subptr(x, (uintptr_t) v);
}
inline os_os_uint32 os_sub32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_sub(&x->v, v) - v;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_sub64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_sub(&x->v, v) - v;
}
#endif
inline uintptr_t os_subptr_nv(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_sub(&x->v, v) - v;
}
inline void* os_subvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return (void*) os_subptr_nv(x, (uintptr_t) v);
}

/* AND */

inline void os_and32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_fetch_and(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline void os_and64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_fetch_and(&x->v, v);
}
#endif
inline void os_andptr(volatile os_uintptr* x, uintptr_t v)
{
    atomic_fetch_and(&x->v, v);
}
inline os_os_uint32 os_and32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_and(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_and64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_and(&x->v, v);
}
#endif
inline uintptr_t os_andptr_ov(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_and(&x->v, v);
}
inline os_os_uint32 os_and32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_and(&x->v, v) & v;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_and64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_and(&x->v, v) & v;
}
#endif
inline uintptr_t os_andptr_nv(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_and(&x->v, v) & v;
}

/* OR */

inline void os_or32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_fetch_or(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline void os_or64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_fetch_or(&x->v, v);
}
#endif
inline void os_orptr(volatile os_uintptr* x, uintptr_t v)
{
    atomic_fetch_or(&x->v, v);
}
inline os_os_uint32 os_or32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_or(&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_or64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_or(&x->v, v);
}
#endif
inline uintptr_t os_orptr_ov(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_or(&x->v, v);
}
inline os_os_uint32 os_or32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_fetch_or(&x->v, v) | v;
}
#if PA_ATOMIC64_SUPPORT
inline os_os_uint64 os_or64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_fetch_or(&x->v, v) | v;
}
#endif
inline uintptr_t os_orptr_nv(volatile os_uintptr* x, uintptr_t v)
{
    return atomic_fetch_or(&x->v, v) | v;
}
