/* INC */

PA_INLINE void os_inc32(volatile os_os_uint32* x)
{
    atomic_inc_32(&x->v);
}
PA_INLINE void os_inc64(volatile os_os_uint64* x)
{
    atomic_inc_64(&x->v);
}
PA_INLINE void os_incptr(volatile os_uintptr* x)
{
    atomic_inc_ulong(&x->v);
}
PA_INLINE os_os_uint32 os_inc32_nv(volatile os_os_uint32* x)
{
    return atomic_inc_32_nv(&x->v);
}
PA_INLINE os_os_uint64 os_inc64_nv(volatile os_os_uint64* x)
{
    return atomic_inc_64_nv(&x->v);
}
PA_INLINE os_uintptr os_incptr_nv(volatile os_uintptr* x)
{
    return atomic_inc_ulong_nv(&x->v);
}

/* DEC */

PA_INLINE void os_dec32(volatile os_os_uint32* x)
{
    atomic_dec_32(&x->v);
}
PA_INLINE void os_dec64(volatile os_os_uint64* x)
{
    atomic_dec_64(&x->v);
}
PA_INLINE void os_decptr(volatile os_uintptr* x)
{
    atomic_dec_ulong(&x->v);
}
PA_INLINE os_os_uint32 os_dec32_nv(volatile os_os_uint32* x)
{
    return atomic_dec_32_nv(&x->v);
}
PA_INLINE os_os_uint64 os_dec64_nv(volatile os_os_uint64* x)
{
    return atomic_dec_64_nv(&x->v);
}
PA_INLINE os_uintptr os_decptr_nv(volatile os_uintptr* x)
{
    return atomic_dec_ulong_nv(&x->v);
}

/* ADD */

PA_INLINE void os_add32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_add_32(&x->v, v);
}
PA_INLINE void os_add64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_add_64(&x->v, v);
}
PA_INLINE void os_addptr(volatile os_uintptr* x, os_uintptr v)
{
    atomic_add_ulong(&x->v, v);
}
PA_INLINE void os_addvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    atomic_add_ptr(&x->v, v);
}
PA_INLINE os_os_uint32 os_add32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_add_32(&x->v, v);
}
PA_INLINE os_os_uint64 os_add64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_add_64(&x->v, v);
}
PA_INLINE os_uintptr os_addptr_nv(volatile os_uintptr* x, os_uintptr v)
{
    return atomic_add_ulong(&x->v, v);
}
PA_INLINE void* os_addvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return atomic_add_ptr(&x->v, v);
}

/* SUB */

PA_INLINE void os_sub32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_add_32(&x->v, -v);
}
PA_INLINE void os_sub64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_add_64(&x->v, -v);
}
PA_INLINE void os_subptr(volatile os_uintptr* x, os_uintptr v)
{
    atomic_add_ulong(&x->v, -v);
}
PA_INLINE void os_subvoidp(volatile os_voidp_t* x, ptrdiff_t v)
{
    atomic_add_ptr(&x->v, -v);
}
PA_INLINE os_os_uint32 os_sub32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_add_32_nv(&x->v, -v);
}
PA_INLINE os_os_uint64 os_sub64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_add_64_nv(&x->v, -v);
}
PA_INLINE os_uintptr os_subptr_nv(volatile os_uintptr* x, os_uintptr v)
{
    return atomic_add_ulong_nv(&x->v, -v);
}
PA_INLINE void* os_subvoidp_nv(volatile os_voidp_t* x, ptrdiff_t v)
{
    return atomic_add_ptr_nv(&x->v, -v);
}

/* AND */

PA_INLINE void os_and32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_and_32(&x->v, v);
}
PA_INLINE void os_and64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_and_64(&x->v, v);
}
PA_INLINE void os_andptr(volatile os_uintptr* x, os_uintptr v)
{
    atomic_and_ulong(&x->v, v);
}
PA_INLINE os_os_uint32 os_and32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    os_os_uint32 old, new;
    do
    {
        old = x->v;
        new = old & v;
    }
    while(atomic_cas_32(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_os_uint64 os_and64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    os_os_uint64 old, new;
    do
    {
        old = x->v;
        new = old & v;
    }
    while(atomic_cas_64(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_uintptr os_andptr_ov(volatile os_uintptr* x, os_uintptr v)
{
    os_uintptr old, new;
    do
    {
        old = x->v;
        new = old & v;
    }
    while(atomic_cas_ulong(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_os_uint32 os_and32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_and_32_nv(&x->v, v);
}
PA_INLINE os_os_uint64 os_and64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_and_64_nv(&x->v, v);
}
PA_INLINE os_uintptr os_andptr_nv(volatile os_uintptr* x, os_uintptr v)
{
    return atomic_and_ulong(&x->v, v);
}

/* OR */

PA_INLINE void os_or32(volatile os_os_uint32* x, os_os_uint32 v)
{
    atomic_or_32(&x->v, v);
}
PA_INLINE void os_or64(volatile os_os_uint64* x, os_os_uint64 v)
{
    atomic_or_64(&x->v, v);
}
PA_INLINE void os_orptr(volatile os_uintptr* x, os_uintptr v)
{
    atomic_or_ulong(&x->v, v);
}
PA_INLINE os_os_uint32 os_or32_ov(volatile os_os_uint32* x, os_os_uint32 v)
{
    os_os_uint32 old, new;
    do
    {
        old = x->v;
        new = old | v;
    }
    while(atomic_cas_32(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_os_uint64 os_or64_ov(volatile os_os_uint64* x, os_os_uint64 v)
{
    os_os_uint64 old, new;
    do
    {
        old = x->v;
        new = old | v;
    }
    while(atomic_cas_64(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_uintptr os_orptr_ov(volatile os_uintptr* x, os_uintptr v)
{
    os_uintptr old, new;
    do
    {
        old = x->v;
        new = old | v;
    }
    while(atomic_cas_ulong(&x->v, old, new) != old);
    return old;
}
PA_INLINE os_os_uint32 os_or32_nv(volatile os_os_uint32* x, os_os_uint32 v)
{
    return atomic_or_32_nv(&x->v, v);
}
PA_INLINE os_os_uint64 os_or64_nv(volatile os_os_uint64* x, os_os_uint64 v)
{
    return atomic_or_64_nv(&x->v, v);
}
PA_INLINE os_uintptr os_orptr_nv(volatile os_uintptr* x, os_uintptr v)
{
    return atomic_or_ulong(&x->v, v);
}
