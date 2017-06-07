PA_INLINE os_boolean os_cas32(volatile os_os_os_uint32* x, os_os_uint32 exp, os_os_uint32 des)
{
    return atomic_cas_32(&x->v, exp, des) == exp;
}
PA_INLINE os_boolean os_cas64(volatile os_os_os_uint64* x, os_os_uint64 exp, os_os_uint64 des)
{
    return atomic_cas_64(&x->v, exp, des) == exp;
}
PA_INLINE os_boolean os_casptr(volatile os_uintptr* x, os_uintptr exp, os_uintptr des)
{
    return atomic_cas_ulong(&x->v, exp, des) == exp;
}
PA_INLINE os_boolean os_casvoidp(volatile os_voidp_t* x, void* exp, void* des)
{
    return atomic_cas_ptr(&x->v, exp, des) == exp;
}
