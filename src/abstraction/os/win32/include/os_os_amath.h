/* INC */

PA_INLINE void os_inc32(volatile os_os_uint32* x)
{
    InterlockedIncrement(&x->v) - 1;
}
PA_INLINE void os_inc64(volatile os_os_uint64* x)
{
    InterlockedIncrement64(&x->v) - 1;
}
PA_INLINE void os_incptr(volatile os_uintptr_t* x)
{
    PA_ATOMIC_PTROP(InterlockedIncrement)(&x->v) - 1;
}
PA_INLINE os_os_uint32 os_inc32_nv(volatile os_os_uint32* x)
{
    return InterlockedIncrement(x);
}
PA_INLINE os_os_uint64 os_inc64_nv(volatile os_os_uint64* x)
{
    return InterlockedIncrement64(&x->v);
}
PA_INLINE uintptr_t os_incptr_nv(volatile os_uintptr_t* x)
{
    return PA_ATOMIC_PTROP(InterlockedIncrement)(&x->v);
}

/* DEC */

PA_INLINE void os_dec32(volatile os_os_uint32* x)
{
    InterlockedIncrement(&x->v) + 1;
}
PA_INLINE void os_dec64(volatile os_os_uint64* x)
{
    InterlockedDecrement64(&x->v) + 1;
}
PA_INLINE void os_decptr(volatile os_uintptr_t* x)
{
    PA_ATOMIC_PTROP(InterlockedDecrement)(&x->v) + 1;
}
PA_INLINE os_os_uint32 os_dec32_nv(volatile os_os_uint32* x)
{
    return InterlockedDecrement(&x->v);
}
PA_INLINE os_os_uint64 os_dec64_nv(volatile os_os_uint64* x)
{
    return InterlockedDecrement64(&x->v);
}
PA_INLINE uintptr_t os_decptr_nv(volatile os_uintptr_t* x)
{
    return PA_ATOMIC_PTROP(InterlockedDecrement)(&x->v);
}

/* ADD */

PA_INLINE void os_add32(volatile os_os_uint32* x, os_os_uint32 v)
{
    InterlockedExchangeAdd(&x->v, v);
}
PA_INLINE void os_add64(volatile os_os_uint64* x, os_os_uint64 v)
{
    InterlockedExchangeAdd64(&x->v, v);
}
PA_INLINE void os_addptr(volatile os_uintptr_t* x, uintptr_t v)
{
    PA_ATOMIC_PTROP(InterlockedExchangeAdd)(&x->v, v);
}
PA_INLINE void os_addvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    os_addptr_nv(x, (uintptr_t) v);
}
PA_INLINE os_os_uint32 os_add32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedExchangeAdd(&x->v, v) + v;
}
PA_INLINE os_os_uint64 os_add64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedExchangeAdd64(&x->v, v) + v;
}
PA_INLINE uintptr_t os_addptr_nv(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedExchangeAdd)(&x->v, v) + v;
}
PA_INLINE void* os_addvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return (void*) os_addptr_nv(x, (uintptr_t) v);
}

/* SUB */

PA_INLINE void os_sub32(volatile os_os_uint32* x, os_os_uint32 v)
{
    InterlockedExchangeAdd(&x->v, -v);
}
PA_INLINE void os_sub64(volatile os_os_uint64* x, os_os_uint64 v)
{
    InterlockedExchangeAdd64(&x->v, -v);
}
PA_INLINE void os_subptr(volatile os_uintptr_t* x, uintptr_t v)
{
    PA_ATOMIC_PTROP(InterlockedExchangeAdd)(&x->v, -v);
}
PA_INLINE void os_subvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    os_subptr_nv(x, (uintptr_t) v);
}
PA_INLINE os_os_uint32 os_sub32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedExchangeAdd(&x->v, -v) - v;
}
PA_INLINE os_os_uint64 os_sub64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedExchangeAdd64(&x->v, -v) - v;
}
PA_INLINE uintptr_t os_subptr_nv(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedExchangeAdd)(&x->v, -v) - v;
}
PA_INLINE void* os_subvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return (void*) os_subptr_nv(x, (uintptr_t) v);
}

/* AND */

PA_INLINE void os_and32(volatile os_os_uint32* x, os_os_uint32 v)
{
    InterlockedAnd(&x->v, v);
}
PA_INLINE void os_and64(volatile os_os_uint64* x, os_os_uint64 v)
{
    InterlockedAnd64(&x->v, v);
}
PA_INLINE void os_andptr(volatile os_uintptr_t* x, uintptr_t v)
{
    PA_ATOMIC_PTROP(InterlockedAnd)(&x->v, v);
}
PA_INLINE os_os_uint32 os_and32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedAnd(&x->v, v);
}
PA_INLINE os_os_uint64 os_and64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedAnd64(&x->v, v);
}
PA_INLINE uintptr_t os_andptr_ov(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedAnd)(&x->v, v);
}
PA_INLINE os_os_uint32 os_and32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedAnd(&x->v, v) & v;
}
PA_INLINE os_os_uint64 os_and64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedAnd64(&x->v, v) & v;
}
PA_INLINE uintptr_t os_andptr_nv(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedAnd)(&x->v, v) & v;
}

/* OR */

PA_INLINE void os_or32(volatile os_os_uint32* x, os_os_uint32 v)
{
    InterlockedOr(&x->v, v);
}
PA_INLINE void os_or64(volatile os_os_uint64* x, os_os_uint64 v)
{
    InterlockedOr64(&x->v, v);
}
PA_INLINE void os_orptr(volatile os_uintptr_t* x, uintptr_t v)
{
    PA_ATOMIC_PTROP(InterlockedOr)(&x->v, v);
}
PA_INLINE os_os_uint32 os_or32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedOr(&x->v, v);
}
PA_INLINE os_os_uint64 os_or64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedOr64(&x->v, v);
}
PA_INLINE uintptr_t os_orptr_ov(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedOr)(&x->v, v);
}
PA_INLINE os_os_uint32 os_or32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return InterlockedOr(&x->v, v) | v;
}
PA_INLINE os_os_uint64 os_or64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return InterlockedOr64(&x->v, v) | v;
}
PA_INLINE uintptr_t os_orptr_nv(volatile os_uintptr_t* x, uintptr_t v)
{
    return PA_ATOMIC_PTROP(InterlockedOr)(&x->v, v) | v;
}
