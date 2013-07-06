#ifndef OS_FORMATSPEC_H
#define OS_FORMATSPEC_H

/* printf() format specifiers that work for most ILP32 and I32LP64 platforms --
   pretty much every modern Unix box out there for example.  Essentially a
   clone of the C99 stuff. */

#define PA_PA_PRId16 "hd"
#define PA_PA_PRIo16 "ho"
#define PA_PA_PRIu16 "hu"
#define PA_PA_PRIx16 "hx"
#define PA_PA_PRIX16 "hX"

#define PA_PA_PRId32 "d"
#define PA_PA_PRIo32 "o"
#define PA_PA_PRIu32 "u"
#define PA_PA_PRIx32 "x"
#define PA_PA_PRIX32 "X"

#ifdef PA__64BIT
#define PA_PA_PRId64 "ld"
#define PA_PA_PRIo64 "lo"
#define PA_PA_PRIu64 "lu"
#define PA_PA_PRIx64 "lx"
#define PA_PA_PRIX64 "lX"
#else
#define PA_PA_PRId64 "lld"
#define PA_PA_PRIo64 "llo"
#define PA_PA_PRIu64 "llu"
#define PA_PA_PRIx64 "llx"
#define PA_PA_PRIX64 "llX"
#endif

#define PA_PA_PRIdADDR "ld"
#define PA_PA_PRIoADDR "lo"
#define PA_PA_PRIuADDR "lu"
#define PA_PA_PRIxADDR "lx"
#define PA_PA_PRIXADDR "lX"

#define PA_PA_PRIdSIZE "ld"
#define PA_PA_PRIoSIZE "lo"
#define PA_PA_PRIuSIZE "lu"
#define PA_PA_PRIxSIZE "lx"
#define PA_PA_PRIXSIZE "lX"

/* magic for creating compile-time 64-bit constants */

#ifdef PA__64BIT
#define PA_PA_INT64_C(x) x##l
#define PA_PA_UINT64_C(x) x##ul
#else
#define PA_PA_INT64_C(x) x##ll
#define PA_PA_UINT64_C(x) x##ull
#endif

#endif /* OS_FORMATSPEC_H */
