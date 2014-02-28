#ifndef OS_FORMATSPEC_H
#define OS_FORMATSPEC_H

/* Include platform-specific os_abstract.h */
#include "include/os_abstract.h"
/* printf() format specifiers that work for Win32/Win64/WinCE */

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

#define PA_PA_PRId64 "I64d"
#define PA_PA_PRIo64 "I64o"
#define PA_PA_PRIu64 "I64u"
#define PA_PA_PRIx64 "I64x"
#define PA_PA_PRIX64 "I64X"

#ifdef PA__64BIT
#define PA_PA_PRIdADDR "I64d"
#define PA_PA_PRIoADDR "I64o"
#define PA_PA_PRIuADDR "I64u"
#define PA_PA_PRIxADDR "I64x"
#define PA_PA_PRIXADDR "I64X"
#else
#define PA_PA_PRIdADDR "ld"
#define PA_PA_PRIoADDR "lo"
#define PA_PA_PRIuADDR "lu"
#define PA_PA_PRIxADDR "lx"
#define PA_PA_PRIXADDR "lX"
#endif

#define PA_PA_PRIdSIZE PA_PA_PRIdADDR
#define PA_PA_PRIoSIZE PA_PA_PRIoADDR
#define PA_PA_PRIuSIZE PA_PA_PRIuADDR
#define PA_PA_PRIxSIZE PA_PA_PRIxADDR
#define PA_PA_PRIXSIZE PA_PA_PRIXADDR

#define PA_PA_INT64_C(x) x##i64
#define PA_PA_UINT64_C(x) x##ui64

#endif /* OS_FORMATSPEC_H */
