PA_INLINE void os_fence(void)
{
    membar_enter();
    membar_exit();
}
PA_INLINE void os_fence_acq(void)
{
    membar_enter();
}
PA_INLINE void os_fence_rel(void)
{
    membar_exit();
}
