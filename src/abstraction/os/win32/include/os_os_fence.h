PA_INLINE void os_fence(void)
{
    volatile LONG tmp = 0;
    InterlockedExchange(&tmp, 0);
}
PA_INLINE void os_fence_acq(void)
{
    os_fence();
}
PA_INLINE void os_fence_rel(void)
{
    os_fence();
}
