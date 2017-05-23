PA_INLINE os_os_uint32 os_ld32(volatile os_os_uint32* x)
{
    return x->v;
}
_INLINE os_os_uint32 os_ld32(volatile os_os_uint32* x)
{
    return x->v;
}
PA_INLINE os_os_int64 os_ld64(volatile os_os_int64* x)
{
    return x->v;
}
PA_INLINE os_uintptr os_ldptr(volatile os_uintptr* x)
{
    return x->v;
}
PA_INLINE void* os_ldvoidp(volatile os_voidp_t* x)
{
    return (void*) os_ldptr(x);
}

PA_INLINE void os_st32(volatile os_os_uint32* x, os_os_uint32 v)
{
    x->v = v;
}
PA_INLINE void os_st64(volatile os_os_int64* x, os_os_int64 v)
{
    x->v = v;
}
PA_INLINE void os_stptr(volatile os_uintptr* x, os_uintptr v)
{
    x->v = v;
}
PA_INLINE void os_stvoidp(volatile os_voidp_t* x, void* v)
{
    os_stptr(x, (os_uintptr) v);
}
PA_INLINE os_os_int64 os_ld64(volatile os_os_int64* x)
{
    return x->v;
}
PA_INLINE os_uintptr os_ldptr(volatile os_uintptr* x)
{
    return x->v;
}
PA_INLINE void* os_ldvoidp(volatile os_voidp_t* x)
{
    return (void*) os_ldptr(x);
}

PA_INLINE void os_st32(volatile os_os_uint32* x, os_os_uint32 v)
{
    x->v = v;
}
PA_INLINE void os_st64(volatile os_os_int64* x, os_os_int64 v)
{
    x->v = v;
}
PA_INLINE void os_stptr(volatile os_uintptr* x, os_uintptr v)
{
    x->v = v;
}
PA_INLINE void os_stvoidp(volatile os_voidp_t* x, void* v)
{
    os_stptr(x, (os_uintptr) v);
}
