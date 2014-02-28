/* -*- c -*- */
#if defined SUPPRESS_SYSDEPS_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

#if defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_MIPS
#include <vxAtomicLib.h>
#endif

/* MISSING IN HW ABSTRACTION LAYER -------------------------------------

   Atomic operations because the two currently present in the
   abstraction layer don't quite cut it.  Roughly the same as the
   solaris atomic operations.  */
NN_C99_INLINE os_uint32 atomic_read_u32 (volatile const os_uint32 *loc)
{
  return *loc;
}

NN_C99_INLINE void atomic_store_u32 (volatile os_uint32 *loc, os_uint32 new)
{
  *loc = new;
}

#if __APPLE__

#include <libkern/OSAtomic.h>
NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return (os_uint32) OSAtomicIncrement32 ((volatile os_int32 *) value);
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return (os_uint32) OSAtomicDecrement32 ((volatile os_int32 *) value);
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  OSAtomicAdd32 ((os_int32) addend, (volatile os_int32 *) value);
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  return (os_uint32) OSAtomicAdd32 ((os_int32) -subtrahend, (volatile os_int32 *) value);
}
NN_C99_INLINE void pa_membar_enter (void)
{
  OSMemoryBarrier ();
}
NN_C99_INLINE void pa_membar_exit (void)
{
  OSMemoryBarrier ();
}
NN_C99_INLINE void pa_membar_producer (void)
{
  OSMemoryBarrier ();
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  OSMemoryBarrier ();
}

#elif defined _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return (os_uint32) InterlockedIncrement ((volatile LONG *) value);
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return (os_uint32) InterlockedDecrement ((volatile LONG *) value);
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  InterlockedExchangeAdd ((volatile os_int32 *) value, (os_int32) addend);
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  os_uint32 old;
  old = (os_uint32) InterlockedExchangeAdd ((volatile os_int32 *) value, (os_int32) -subtrahend);
  return old - subtrahend;
}
/* From MSDN: from Vista & 2k3 onwards, a macro named MemoryBarrier is
   defined, XP needs inline assembly.  Unfortunately, MemoryBarrier()
   is a function on x86 ...

   Definition below is taken from the MSDN page on MemoryBarrier() */
#ifndef MemoryBarrier
#if NTDDI_VERSION >= NTDDI_WS03 && defined _M_IX86
#define MemoryBarrier() do {                    \
    LONG Barrier;                               \
    __asm {                                     \
      xchg Barrier, eax                         \
    }                                           \
  } while (0)
#endif /* x86 */

/* Don't try interworking with thumb - one thing at a time. Do a DMB
   SY if supported, else no need for a memory barrier. (I think.) */
#if defined _M_ARM && ! defined _M_ARMT
#define MemoryBarrierARM __emit (0xf57ff05f) /* 0xf57ff05f or 0x5ff07ff5 */
#if _M_ARM > 7
/* if targetting ARMv7 the dmb instruction is available */
#define MemoryBarrier() MemoryBarrierARM
#else
/* else conditional on actual hardware platform */
extern void (*q_maybe_membar) (void);
#define MemoryBarrier() q_maybe_membar ()
#define NEED_ARM_MEMBAR_SUPPORT 1
#endif /* ARM version */
#endif /* ARM */

#endif /* !def MemoryBarrier */
NN_C99_INLINE void pa_membar_enter (void)
{
  MemoryBarrier ();
}
NN_C99_INLINE void pa_membar_exit (void)
{
  MemoryBarrier ();
}
NN_C99_INLINE void pa_membar_producer (void)
{
  MemoryBarrier ();
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  MemoryBarrier ();
}

#elif (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40100

NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
#if defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_MIPS
  return (os_uint32) vxAtomicInc_inline ((atomic_t *) value) + 1;
#else
  return __sync_add_and_fetch (value, 1);
#endif
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
#if defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_MIPS
  return (os_uint32) vxAtomicDec_inline ((atomic_t *) value) - 1;
#else
  return __sync_sub_and_fetch (value, 1);
#endif
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
#if defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_MIPS
  (void)vxAtomicAdd_inline( (atomic_t *)value, addend );
#else
  __sync_fetch_and_add (value, addend);
#endif
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
#if defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_MIPS
  return( vxAtomicSub_inline( (atomic_t *)value, subtrahend ) - subtrahend );
#else
  return __sync_sub_and_fetch (value, subtrahend);
#endif
}
NN_C99_INLINE void pa_membar_enter (void)
{
  __sync_synchronize ();
}
NN_C99_INLINE void pa_membar_exit (void)
{
  __sync_synchronize ();
}
NN_C99_INLINE void pa_membar_producer (void)
{
  __sync_synchronize ();
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  __sync_synchronize ();
}

#elif defined __sun

#if defined __GNUC__ && defined __sparc__
/* assume we can use os_atomic_add from the PA layer */
#include "../../../abstraction/pa/code/pa_abstract.c"
NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return os_atomic_add (value, 1) + 1;
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return os_atomic_add (value, -1) - 1;
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  os_atomic_add (value, addend);
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  os_uint32 old = os_atomic_add (value, -subtrahend);
  return old - subtrahend;
}
NN_C99_INLINE void pa_membar_enter (void)
{
  __asm__ __volatile__ ("membar #StoreLoad|#StoreStore\n");
}
NN_C99_INLINE void pa_membar_exit (void)
{
  __asm__ __volatile__ ("membar #LoadStore|#StoreStore\n");
}
NN_C99_INLINE void pa_membar_producer (void)
{
  __asm__ __volatile__ ("membar #StoreStore\n");
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  __asm__ __volatile__ ("membar #LoadLoad\n");
}
#else /* assume we're on Solaris 10 or newer */
#include <atomic.h>
NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return atomic_inc_32_nv (value);
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return atomic_dec_32_nv (value);
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  atomic_add_32 (value, addend);
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  return atomic_add_32_nv (value, -subtrahend);
}
NN_C99_INLINE void pa_membar_enter (void)
{
  membar_enter ();
}
NN_C99_INLINE void pa_membar_exit (void)
{
  membar_exit ();
}
NN_C99_INLINE void pa_membar_producer (void)
{
  membar_producer ();
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  membar_consumer ();
}
#endif /* solaris + gcc */

#elif defined __GNUC__

#ifdef __i386
NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  os_uint32 tmp;
  __asm__ __volatile__ (
          "movl $1, %0; lock; xaddl %0, %2; incl %0\n"
          : "=&r" (tmp), "=m" (*value) : "m" (*value) : "memory");
  return tmp;
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  os_uint32 tmp;
  __asm__ __volatile__ (
          "movl $-1, %0; lock; xaddl %0, %2; decl %0\n"
          : "=&r" (tmp), "=m" (*value) : "m" (*value) : "memory");
  return tmp;
}
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  __asm__ __volatile__ ("lock; xaddl %1, %0\n" : "+m" (*value), "=r" (addend) : "1" (addend) : "memory", "cc");
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  os_uint32 tmp = -subtrahend;
  __asm__ __volatile__ ("lock; xaddl %1, %0\n" : "+m" (*value), "=r" (tmp) : "1" (tmp) : "memory", "cc");
  return tmp - subtrahend;
}
NN_C99_INLINE void pa_membar_enter (void)
{
#if __x86_64__
  __asm__ __volatile__ ("mfence\n");
#else
  __asm__ __volatile__ ("lock; xorl $0, (%esp)\n");
#endif
}
NN_C99_INLINE void pa_membar_exit (void)
{
#if __x86_64__
  __asm__ __volatile__ ("mfence\n");
#else
  __asm__ __volatile__ ("lock; xorl $0, (%esp)\n");
#endif
}
NN_C99_INLINE void pa_membar_producer (void)
{
#if __x86_64__
  __asm__ __volatile__ ("sfence\n");
#else
  __asm__ __volatile__ ("lock; xorl $0, (%esp)\n");
#endif
}
NN_C99_INLINE void pa_membar_consumer (void)
{
#if __x86_64__
  __asm__ __volatile__ ("lfence\n");
#else
  __asm__ __volatile__ ("lock; xorl $0, (%esp)\n");
#endif
}
#else
#if defined __powerpc__ || defined _VX_CPU_FAMILY && _VX_CPU_FAMILY==_VX_PPC || defined _WRS_KERNEL && ! defined _WRS_VXWORKS_MAJOR && CPU==PPC604
NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  register os_uint32 value2 __asm__ ("r4");
  __asm__ __volatile__ (
          "1: lwarx   %0,0,%2\n\t"
          "   add    %0,%3,%0\n\t"
          "   stwcx.  %0,0,%2\n\t"
          "   bne-    1b\n\t"
          "   isync"
          : "=&r" (value2), "=m" (*value)
          : "r" (value), "r" (addend)
          : "memory");
}

NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  register os_uint32 value2 __asm__ ("r4");
  __asm__ __volatile__ (
          "1: lwarx   %0,0,%2\n\t"
          "   subf    %0,%3,%0\n\t"
          "   stwcx.  %0,0,%2\n\t"
          "   bne-    1b\n\t"
          "   isync"
          : "=&r" (value2), "=m" (*value)
          : "r" (value), "r" (subtrahend)
          : "memory");
  return value2;
}
NN_C99_INLINE os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return atomic_sub_u32_nv (value, -1);
}
NN_C99_INLINE os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return atomic_sub_u32_nv (value, 1);
}
NN_C99_INLINE void pa_membar_enter (void)
{
   __asm__ __volatile__ ("sync\n");
}
NN_C99_INLINE void pa_membar_exit (void)
{
   __asm__ __volatile__ ("sync\n");
}
NN_C99_INLINE void pa_membar_producer (void)
{
   __asm__ __volatile__ ("sync\n");
}
NN_C99_INLINE void pa_membar_consumer (void)
{
   __asm__ __volatile__ ("sync\n");
}
#endif /* __powerpc__ */
#endif /* __i386 */

#elif defined AIX

#include <sys/atomic_op.h>

NN_C99_INLINE void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend)
{
  fetch_and_add ((os_int32 *) value, addend);
}
NN_C99_INLINE os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend)
{
  os_uint32 old = fetch_and_add ((os_int32 *) value, -subtrahend);
  return old - subtrahend;
}
NN_C99_INLINE void atomic_inc_u32_nv (volatile os_uint32 *value)
{
  return atomic_sub_u32_nv (value, -1);
}
NN_C99_INLINE void atomic_dec_u32_nv (volatile os_uint32 *value)
{
  return atomic_sub_u32_nv (value, 1);
}
/* these based on: http://www.ibm.com/developerworks/systems/articles/powerpc.html */
NN_C99_INLINE void pa_membar_enter (void)
{
  __sync ();
}
NN_C99_INLINE void pa_membar_exit (void)
{
  __sync ();
}
NN_C99_INLINE void pa_membar_producer (void)
{
  __lwsync ();
}
NN_C99_INLINE void pa_membar_consumer (void)
{
  __isync ();
}

#endif /* AIX */

/* SHA1 not available (unoffical build.) */
