PA_INLINE os_boolean os_cas32(volatile os_os_os_uint32* x, os_os_uint32 exp, os_os_uint32 des)
{
    return InterlockedCompareExchange(&x->v, des, exp) == exp;
}
PA_INLINE os_boolean os_cas64(volatile os_os_os_uint64* x, os_os_uint64 exp, os_os_uint64 des)
{
    return InterlockedCompareExchange64(&x->v, des, exp) == exp;
}
PA_INLINE os_boolean os_casptr(volatile os_uintptr* x, uintptr exp, uintptr des)
{
    return PA_ATOMIC_PTROP(InterlockedCompareExchange)(&x->v, des, exp) == exp;
}
PA_INLINE os_boolean os_casvoidp(volatile os_voidp_t* x, void* exp, void* des)
{
    return os_casptr(x, (uintptr) exp, (uintptr) des);
}
