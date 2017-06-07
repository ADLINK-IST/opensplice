inline os_boolean os_cas32(volatile os_os_uint32* x, os_os_uint32 exp, os_os_uint32 des)
{
    return atomic_compare_exchange_strong(&x->v, &exp, des);
}
#if PA_ATOMIC64_SUPPORT
inline os_boolean os_cas64(volatile os_os_int64* x, os_os_int64 exp, os_os_int64 des)
{
    return atomic_compare_exchange_strong(&x->v, &exp, des);
}
#endif
inline os_boolean os_casptr(volatile os_uintptr* x, os_uintptr exp, os_uintptr des)
{
    return atomic_compare_exchange_strong(&x->v, &exp, des);
}
inline os_boolean os_casvoidp(volatile os_voidp_t* x, void* exp, void* des)
{
    return os_casptr(x, (os_uintptr) exp, (os_uintptr) des);
}
