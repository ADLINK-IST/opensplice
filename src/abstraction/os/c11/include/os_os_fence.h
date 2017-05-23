inline void os_fence(void)
{
    atomic_thread_fence(memory_order_seq_cst);
}
inline void os_fence_acq(void)
{
    atomic_thread_fence(memory_order_acquire);
}
inline void os_fence_rel(void)
{
    atomic_thread_fence(memory_order_release);
}
