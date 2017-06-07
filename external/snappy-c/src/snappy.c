/*
 * C port of the snappy compressor from Google.
 * This is a very fast compressor with comparable compression to lzo.
 * Works best on 64bit little-endian, but should be good on others too.
 * Ported by Andi Kleen.
 * Based on snappy 1.0.3 plus some selected changes from SVN.
 */

/*
 * Copyright 2005 Google Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <limits.h>
#include "os_defs.h"
#include "snappy.h"
#include "compat.h"

#define CRASH_UNLESS(x) BUG_ON(!(x))
#define CHECK(cond) CRASH_UNLESS(cond)
#define CHECK_LE(a, b) CRASH_UNLESS((a) <= (b))
#define CHECK_GE(a, b) CRASH_UNLESS((a) >= (b))
#define CHECK_EQ(a, b) CRASH_UNLESS((a) == (b))
#define CHECK_NE(a, b) CRASH_UNLESS((a) != (b))
#define CHECK_LT(a, b) CRASH_UNLESS((a) < (b))
#define CHECK_GT(a, b) CRASH_UNLESS((a) > (b))

#define UNALIGNED_LOAD16(_p) get_unaligned((u16 *)(_p))
#define UNALIGNED_LOAD32(_p) get_unaligned((u32 *)(_p))
#define UNALIGNED_LOAD64(_p) get_unaligned((u64 *)(_p))

#define UNALIGNED_STORE16(_p, _val) put_unaligned(_val, (u16 *)(_p))
#define UNALIGNED_STORE32(_p, _val) put_unaligned(_val, (u32 *)(_p))
#define UNALIGNED_STORE64(_p, _val) put_unaligned(_val, (u64 *)(_p))

#ifdef NDEBUG

#define DCHECK(cond) do {} while(0)
#define DCHECK_LE(a, b) do {} while(0)
#define DCHECK_GE(a, b) do {} while(0)
#define DCHECK_EQ(a, b) do {} while(0)
#define DCHECK_NE(a, b) do {} while(0)
#define DCHECK_LT(a, b) do {} while(0)
#define DCHECK_GT(a, b) do {} while(0)

#else

#define DCHECK(cond) CHECK(cond)
#define DCHECK_LE(a, b) CHECK_LE(a, b)
#define DCHECK_GE(a, b) CHECK_GE(a, b)
#define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#define DCHECK_NE(a, b) CHECK_NE(a, b)
#define DCHECK_LT(a, b) CHECK_LT(a, b)
#define DCHECK_GT(a, b) CHECK_GT(a, b)

#endif

#if defined (WIN32) || defined (WIN64)
#define OSPL_INLINE __inline
#define ENOMEM 12
#define EIO 5
#else
#define OSPL_INLINE inline
#include <errno.h>
#endif

static OSPL_INLINE int is_little_endian(void)
{
#ifdef PA_LITTLE_ENDIAN
    return 1;
#else
    return 0;
#endif
}

#if _GNUC_ >= 4 || (_GNUC_ == 3 && _GNUC_MINOR_ >= 4)
static inline int log2_floor(u32 n)
{
    return n == 0 ? -1 : 31 ^ __builtin_clz(n);
}

static inline int find_lsb_set_non_zero(u32 n)
{
    return __builtin_ctz(n);
}

static inline int find_lsb_set_non_zero64(u64 n)
{
    return __builtin_ctzll(n);
}
#else
static OSPL_INLINE int log2_floor(u32 n)
{
        int result = -1;
        while (n != 0)
        {
                result++;
                n >>= 1;
        }
        return result;
}
static OSPL_INLINE int find_lsb_set_non_zero(u32 n)
{
        int result = 0;
        if (n == 0) return 0;  /* undefined in gcc but this is what I get */
        while ((n & 1U) == 0)
        {
                result++;
                n >>= 1;
        }
        return result;
}
static OSPL_INLINE int find_lsb_set_non_zero64(u64 n)
{
        int result = 0;
        if (n == 0) return 32;  /* undefined in gcc but this is what I get */
        while ((n & 1U) == 0)
        {
                result++;
                n >>= 1;
        }
        return result;
}
#endif
#define kmax32 5

/*
 * Attempts to parse a varint32 from a prefix of the bytes in [ptr,limit-1].
 *  Never reads a character at or beyond limit.  If a valid/terminated varint32
 * was found in the range, stores it in *OUTPUT and returns a pointer just
 * past the last byte of the varint32. Else returns NULL.  On success,
 * "result <= limit".
 */
static OSPL_INLINE const char *varint_parse32_with_limit(const char *p,
                            const char *l,
                            u32 * OUTPUT)
{
    const unsigned char *ptr = (const unsigned char *)(p);
    const unsigned char *limit = (const unsigned char *)(l);
    u32 b, result;

    if (ptr >= limit)
        return NULL;
    b = *(ptr++);
    result = b & 127;
    if (b < 128)
        goto done;
    if (ptr >= limit)
        return NULL;
    b = *(ptr++);
    result |= (b & 127) << 7;
    if (b < 128)
        goto done;
    if (ptr >= limit)
        return NULL;
    b = *(ptr++);
    result |= (b & 127) << 14;
    if (b < 128)
        goto done;
    if (ptr >= limit)
        return NULL;
    b = *(ptr++);
    result |= (b & 127) << 21;
    if (b < 128)
        goto done;
    if (ptr >= limit)
        return NULL;
    b = *(ptr++);
    result |= (b & 127) << 28;
    if (b < 16)
        goto done;
    return NULL;        /* Value is too long to be a varint32 */
done:
    *OUTPUT = result;
    return (const char *)(ptr);
}

/*
 * REQUIRES   "ptr" points to a buffer of length sufficient to hold "v".
 *  EFFECTS    Encodes "v" into "ptr" and returns a pointer to the
 *            byte just past the last encoded byte.
 */
static OSPL_INLINE char *varint_encode32(char *sptr, u32 v)
{
    /* Operate on characters as unsigneds */
    unsigned char *ptr = (unsigned char *)(sptr);
    static const int B = 128;

    if (v < (1 << 7)) {
        *(ptr++) = v;
    } else if (v < (1 << 14)) {
        *(ptr++) = v | B;
        *(ptr++) = v >> 7;
    } else if (v < (1 << 21)) {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = v >> 14;
    } else if (v < (1 << 28)) {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = (v >> 14) | B;
        *(ptr++) = v >> 21;
    } else {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = (v >> 14) | B;
        *(ptr++) = (v >> 21) | B;
        *(ptr++) = v >> 28;
    }
    return (char *)(ptr);
}

#ifdef SG

struct source {
    struct iovec *iov;
    int iovlen;
    int curvec;
    int curoff;
    size_t total;
};

/* Only valid at beginning when nothing is consumed */
static OSPL_INLINE int available(struct source *s)
{
    return s->total;
}

static OSPL_INLINE const char *peek(struct source *s, size_t *len)
{
    if (likely(s->curvec < s->iovlen)) {
        struct iovec *iv = &s->iov[s->curvec];
        if (s->curoff < iv->iov_len) { 
            *len = iv->iov_len - s->curoff;
            return iv->iov_base + s->curoff;
        }
    }
    *len = 0;
    return NULL;
}

static OSPL_INLINE void skip(struct source *s, size_t n)
{
    struct iovec *iv = &s->iov[s->curvec];
    s->curoff += n;
    DCHECK_LE(s->curoff, iv->iov_len);
    if (s->curoff >= iv->iov_len && s->curvec + 1 < s->iovlen) {
        s->curoff = 0;
        s->curvec++;
    }
}

struct sink {
    struct iovec *iov;
    int iovlen;
    unsigned curvec;
    unsigned curoff;
    unsigned written;
};

static OSPL_INLINE void append(struct sink *s, const char *data, size_t n)
{
    struct iovec *iov = &s->iov[s->curvec];
    char *dst = iov->iov_base + s->curoff;
    size_t nlen = min_t(size_t, iov->iov_len - s->curoff, n);
    if (data != dst)
        memcpy(dst, data, nlen);
    s->written += n;
    s->curoff += nlen;
    while ((n -= nlen) > 0) {
        data += nlen;
        s->curvec++;
        DCHECK_LT(s->curvec, s->iovlen);
        iov++;
        nlen = min_t(size_t, iov->iov_len, n);
        memcpy(iov->iov_base, data, nlen);
        s->curoff = nlen;
    }
}

static OSPL_INLINE void *sink_peek(struct sink *s, size_t n)
{
    struct iovec *iov = &s->iov[s->curvec];
    if (s->curvec < iov->iov_len && iov->iov_len - s->curoff >= n)
        return iov->iov_base + s->curoff;
    return NULL;
}

#else

struct source {
    const char *ptr;
    size_t left;
};

static OSPL_INLINE int available(struct source *s)
{
    return s->left;
}

static OSPL_INLINE const char *peek(struct source *s, size_t * len)
{
    *len = s->left;
    return s->ptr;
}

static OSPL_INLINE void skip(struct source *s, size_t n)
{
    s->left -= n;
    s->ptr += n;
}

struct sink {
    char *dest;
};

static OSPL_INLINE void append(struct sink *s, const char *data, size_t n)
{
    if (data != s->dest)
        memcpy(s->dest, data, n);
    s->dest += n;
}

static OSPL_INLINE void *sink_peek(struct sink *s, size_t n)
{
    OS_UNUSED_ARG(n);
    return s->dest;
}

#endif

struct writer {
    char *base;
    char *op;
    char *op_limit;
};

/* Called before decompression */
static OSPL_INLINE void writer_set_expected_length(struct writer *w, size_t len)
{
    w->op_limit = w->op + len;
}

/* Called after decompression */
static OSPL_INLINE int writer_check_length(struct writer *w)
{
    return w->op == w->op_limit;
}

/*
 * Copy "len" bytes from "src" to "op", one byte at a time.  Used for
 *  handling COPY operations where the input and output regions may
 * overlap.  For example, suppose:
 *    src    == "ab"
 *    op     == src + 2
 *    len    == 20
 * After IncrementalCopy(src, op, len), the result will have
 * eleven copies of "ab"
 *    ababababababababababab
 * Note that this does not match the semantics of either memcpy()
 * or memmove().
 */
static OSPL_INLINE void incremental_copy(const char *src, char *op, int len)
{
    DCHECK_GT(len, 0);
    do {
        *op++ = *src++;
    } while (--len > 0);
}

/*
 * Equivalent to IncrementalCopy except that it can write up to ten extra
 *  bytes after the end of the copy, and that it is faster.
 *
 * The main part of this loop is a simple copy of eight bytes at a time until
 * we've copied (at least) the requested amount of bytes.  However, if op and
 * src are less than eight bytes apart (indicating a repeating pattern of
 * length < 8), we first need to expand the pattern in order to get the correct
 * results. For instance, if the buffer looks like this, with the eight-byte
 * <src> and <op> patterns marked as intervals:
 *
 *    abxxxxxxxxxxxx
 *    [------]           src
 *      [------]         op
 *
 * a single eight-byte copy from <src> to <op> will repeat the pattern once,
 * after which we can move <op> two bytes without moving <src>:
 *
 *    ababxxxxxxxxxx
 *    [------]           src
 *        [------]       op
 *
 * and repeat the exercise until the two no longer overlap.
 *
 * This allows us to do very well in the special case of one single byte
 * repeated many times, without taking a big hit for more general cases.
 *
 * The worst case of extra writing past the end of the match occurs when
 * op - src == 1 and len == 1; the last copy will read from byte positions
 * [0..7] and write to [4..11], whereas it was only supposed to write to
 * position 1. Thus, ten excess bytes.
 */

#define kmax_increment_copy_overflow  10

static OSPL_INLINE void incremental_copy_fast_path(const char *src, char *op,
                          int len)
{
    while (op - src < 8) {
        UNALIGNED_STORE64(op, UNALIGNED_LOAD64(src));
        len -= op - src;
        op += op - src;
    }
    while (len > 0) {
        UNALIGNED_STORE64(op, UNALIGNED_LOAD64(src));
        src += 8;
        op += 8;
        len -= 8;
    }
}

static OSPL_INLINE int writer_append_from_self(struct writer *w, u32 offset,
                       u32 len)
{
    char *op = w->op;
    const int space_left = w->op_limit - op;

    if (op - w->base <= offset - 1u)    /* -1u catches offset==0 */
        return 0;
    if (len <= 16 && offset >= 8 && space_left >= 16) {
        /* Fast path, used for the majority (70-80%) of dynamic
         * invocations. */
        UNALIGNED_STORE64(op, UNALIGNED_LOAD64(op - offset));
        UNALIGNED_STORE64(op + 8, UNALIGNED_LOAD64(op - offset + 8));
    } else {
        if (space_left >= len + kmax_increment_copy_overflow) {
            incremental_copy_fast_path(op - offset, op, len);
        } else {
            if (space_left < len) {
                return 0;
            }
            incremental_copy(op - offset, op, len);
        }
    }

    w->op = op + len;
    return 1;
}

static OSPL_INLINE int writer_append(struct writer *w, const char *ip, u32 len)
{
    char *op = w->op;
    const int space_left = w->op_limit - op;
    if (space_left < len)
        return 0;
    memcpy(op, ip, len);
    w->op = op + len;
    return 1;
}

static OSPL_INLINE int writer_try_fast_append(struct writer *w, const char *ip, 
                      u32 available, u32 len)
{
    char *op = w->op;
    const int space_left = w->op_limit - op;
    if (len <= 16 && available >= 16 && space_left >= 16) {
        /* Fast path, used for the majority (~95%) of invocations */
        UNALIGNED_STORE64(op, UNALIGNED_LOAD64(ip));
        UNALIGNED_STORE64(op + 8, UNALIGNED_LOAD64(ip + 8));
        w->op = op + len;
        return 1;
    }
    return 0;
}

/*
 * Any hash function will produce a valid compressed bitstream, but a good
 * hash function reduces the number of collisions and thus yields better
 * compression for compressible input, and more speed for incompressible
 * input. Of course, it doesn't hurt if the hash function is reasonably fast
 * either, as it gets called a lot.
 */
static OSPL_INLINE u32 hash_bytes(u32 bytes, int shift)
{
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) >> shift;
}

static OSPL_INLINE u32 hash(const char *p, int shift)
{
    return hash_bytes(UNALIGNED_LOAD32(p), shift);
}

/*
 * Compressed data can be defined as:
 *    compressed := item* literal*
 *    item       := literal* copy
 *
 * The trailing literal sequence has a space blowup of at most 62/60
 * since a literal of length 60 needs one tag byte + one extra byte
 * for length information.
 *
 * Item blowup is trickier to measure.  Suppose the "copy" op copies
 * 4 bytes of data.  Because of a special check in the encoding code,
 * we produce a 4-byte copy only if the offset is < 65536.  Therefore
 * the copy op takes 3 bytes to encode, and this type of item leads
 * to at most the 62/60 blowup for representing literals.
 *
 * Suppose the "copy" op copies 5 bytes of data.  If the offset is big
 * enough, it will take 5 bytes to encode the copy op.  Therefore the
 * worst case here is a one-byte literal followed by a five-byte copy.
 * I.e., 6 bytes of input turn into 7 bytes of "compressed" data.
 *
 * This last factor dominates the blowup, so the final estimate is:
 */
size_t snappy_max_compressed_length(size_t source_len)
{
    return 32 + source_len + source_len / 6;
}
EXPORT_SYMBOL(snappy_max_compressed_length)

enum {
    LITERAL = 0,
    COPY_1_BYTE_OFFSET = 1,    /* 3 bit length + 3 bits of offset in opcode */
    COPY_2_BYTE_OFFSET = 2,
    COPY_4_BYTE_OFFSET = 3
};

static OSPL_INLINE char *emit_literal(char *op,
                 const char *literal,
                 int len, int allow_fast_path)
{
    int n = len - 1;    /* Zero-length literals are disallowed */

    if (n < 60) {
        /* Fits in tag byte */
        *op++ = LITERAL | (n << 2);

/*
 * The vast majority of copies are below 16 bytes, for which a
 * call to memcpy is overkill. This fast path can sometimes
 * copy up to 15 bytes too much, but that is okay in the
 * main loop, since we have a bit to go on for both sides:
 *
 *   - The input will always have kInputMarginBytes = 15 extra
 *     available bytes, as long as we're in the main loop, and
 *     if not, allow_fast_path = false.
 *   - The output will always have 32 spare bytes (see
 *     MaxCompressedLength).
 */
        if (allow_fast_path && len <= 16) {
            UNALIGNED_STORE64(op, UNALIGNED_LOAD64(literal));
            UNALIGNED_STORE64(op + 8,
                      UNALIGNED_LOAD64(literal + 8));
            return op + len;
        }
    } else {
        /* Encode in upcoming bytes */
        char *base = op;
        int count = 0;
        op++;
        while (n > 0) {
            *op++ = n & 0xff;
            n >>= 8;
            count++;
        }
        DCHECK(count >= 1);
        DCHECK(count <= 4);
        *base = LITERAL | ((59 + count) << 2);
    }
    memcpy(op, literal, len);
    return op + len;
}

static OSPL_INLINE char *emit_copy_less_than64(char *op, int offset, int len)
{
    DCHECK_LE(len, 64);
    DCHECK_GE(len, 4);
    DCHECK_LT(offset, 65536);

    if ((len < 12) && (offset < 2048)) {
        int len_minus_4 = len - 4;
        DCHECK(len_minus_4 < 8);    /* Must fit in 3 bits */
        *op++ =
            COPY_1_BYTE_OFFSET | ((len_minus_4) << 2) | ((offset >> 8)
                                 << 5);
        *op++ = offset & 0xff;
    } else {
        *op++ = COPY_2_BYTE_OFFSET | ((len - 1) << 2);
        put_unaligned_le16(offset, op);
        op += 2;
    }
    return op;
}

static OSPL_INLINE char *emit_copy(char *op, int offset, int len)
{
    /*
     * Emit 64 byte copies but make sure to keep at least four bytes
     * reserved
     */
    while (len >= 68) {
        op = emit_copy_less_than64(op, offset, 64);
        len -= 64;
    }

    /*
     * Emit an extra 60 byte copy if have too much data to fit in
     * one copy
     */
    if (len > 64) {
        op = emit_copy_less_than64(op, offset, 60);
        len -= 60;
    }

    /* Emit remainder */
    op = emit_copy_less_than64(op, offset, len);
    return op;
}

/**
 * snappy_uncompressed_length - return length of uncompressed output.
 * @start: compressed buffer
 * @n: length of compressed buffer.
 * @result: Write the length of the uncompressed output here.
 *
 * Returns true when successfull, otherwise false.
 */
int snappy_uncompressed_length(const char *start, size_t n, size_t * result)
{
    u32 v = 0;
    const char *limit = start + n;
    if (varint_parse32_with_limit(start, limit, &v) != NULL) {
        *result = v;
        return 1;
    } else {
        return 0;
    }
}
EXPORT_SYMBOL(snappy_uncompressed_length)

#define kblock_log 15
#define kblock_size (1 << kblock_log)

/* 
 * This value could be halfed or quartered to save memory 
 * at the cost of slightly worse compression.
 */
#define kmax_hash_table_bits 14
#define kmax_hash_table_size (1 << kmax_hash_table_bits)

/*
 * Use smaller hash table when input.size() is smaller, since we
 * fill the table, incurring O(hash table size) overhead for
 * compression, and if the input is short, we won't need that
 * many hash table entries anyway.
 */
static u16 *get_hash_table(struct snappy_env *env, size_t input_size,
                  int *table_size)
{
    int htsize = 256;
    u16 *table;

    DCHECK(kmax_hash_table_size >= 256);
    while (htsize < kmax_hash_table_size && htsize < input_size)
        htsize <<= 1;
    CHECK_EQ(0, htsize & (htsize - 1));
    CHECK_LE(htsize, kmax_hash_table_size);

    table = env->hash_table;

    *table_size = htsize;
    memset(table, 0, htsize * sizeof(*table));
    return table;
}

/*
 * Return the largest n such that
 *
 *   s1[0,n-1] == s2[0,n-1]
 *   and n <= (s2_limit - s2).
 *
 * Does not read *s2_limit or beyond.
 * Does not read *(s1 + (s2_limit - s2)) or beyond.
 * Requires that s2_limit >= s2.
 *
 * Separate implementation for x86_64, for speed.  Uses the fact that
 * x86_64 is little endian.
 */
#if defined(PA_LITTLE_ENDIAN) && BITS_PER_LONG == 64
static OSPL_INLINE int find_match_length(const char *s1,
                    const char *s2, const char *s2_limit)
{
    int matched = 0;

    DCHECK_GE(s2_limit, s2);
    /*
     * Find out how long the match is. We loop over the data 64 bits at a
     * time until we find a 64-bit block that doesn't match; then we find
     * the first non-matching bit and use that to calculate the total
     * length of the match.
     */
    while (likely(s2 <= s2_limit - 8)) {
        if (unlikely
            (UNALIGNED_LOAD64(s2) == UNALIGNED_LOAD64(s1 + matched))) {
            s2 += 8;
            matched += 8;
        } else {
            /*
             * On current (mid-2008) Opteron models there
             * is a 3% more efficient code sequence to
             * find the first non-matching byte.  However,
             * what follows is ~10% better on Intel Core 2
             * and newer, and we expect AMD's bsf
             * instruction to improve.
             */
            u64 x =
                UNALIGNED_LOAD64(s2) ^ UNALIGNED_LOAD64(s1 +
                                    matched);
            int matching_bits = find_lsb_set_non_zero64(x);
            matched += matching_bits >> 3;
            return matched;
        }
    }
    while (likely(s2 < s2_limit)) {
        if (likely(s1[matched] == *s2)) {
            ++s2;
            ++matched;
        } else {
            return matched;
        }
    }
    return matched;
}
#else
static OSPL_INLINE int find_match_length(const char *s1,
                    const char *s2, const char *s2_limit)
{
    /* Implementation based on the x86-64 version, above. */
    int matched = 0;
    DCHECK_GE(s2_limit, s2);

    while (s2 <= s2_limit - 4 &&
           UNALIGNED_LOAD32(s2) == UNALIGNED_LOAD32(s1 + matched)) {
        s2 += 4;
        matched += 4;
    }
    if (is_little_endian() && s2 <= s2_limit - 4) {
        u32 x =
            UNALIGNED_LOAD32(s2) ^ UNALIGNED_LOAD32(s1 + matched);
        int matching_bits = find_lsb_set_non_zero(x);
        matched += matching_bits >> 3;
    } else {
        while ((s2 < s2_limit) && (s1[matched] == *s2)) {
            ++s2;
            ++matched;
        }
    }
    return matched;
}
#endif

/*
 * For 0 <= offset <= 4, GetU32AtOffset(UNALIGNED_LOAD64(p), offset) will
 *  equal UNALIGNED_LOAD32(p + offset).  Motivation: On x86-64 hardware we have
 * empirically found that overlapping loads such as
 *  UNALIGNED_LOAD32(p) ... UNALIGNED_LOAD32(p+1) ... UNALIGNED_LOAD32(p+2)
 * are slower than UNALIGNED_LOAD64(p) followed by shifts and casts to u32.
 */
static OSPL_INLINE u32 get_u32_at_offset(u64 v, int offset)
{
    DCHECK(0 <= offset && offset <= 4);
    return v >> (is_little_endian()? 8 * offset : 32 - 8 * offset);
}

/*
 * Flat array compression that does not emit the "uncompressed length"
 *  prefix. Compresses "input" string to the "*op" buffer.
 *
 * REQUIRES: "input" is at most "kBlockSize" bytes long.
 * REQUIRES: "op" points to an array of memory that is at least
 * "MaxCompressedLength(input.size())" in size.
 * REQUIRES: All elements in "table[0..table_size-1]" are initialized to zero.
 * REQUIRES: "table_size" is a power of two
 *
 * Returns an "end" pointer into "op" buffer.
 * "end - op" is the compressed size of "input".
 */

static char *compress_fragment(const char *const input,
                   const size_t input_size,
                   char *op, u16 * table, const int table_size)
{
    /* "ip" is the input pointer, and "op" is the output pointer. */
    const char *ip = input;
    const char *baseip = input;
    const char *next_emit = input;
    int shift;
    const char *ip_end;
    const int kinput_margin_bytes = 15;
    u32 skip;
    const char *next_ip;
    const char *candidate;
        u32 hval;
        u32 bytes_between_hash_lookups;
        u64 input_bytes = 0;
        u32 candidate_bytes = 0;
    u32 next_hash;
        int offset;
        const char *base;
        int matched;
        const char *insert_tail;
        u32 prev_hash;
        u32 cur_hash;

    CHECK_LE(input_size, kblock_size);
    CHECK_EQ(table_size & (table_size - 1), 0);
    shift = 32 - log2_floor(table_size);
    DCHECK_EQ(UINT_MAX >> shift, table_size - 1);
    /*
     * Bytes in [next_emit, ip) will be emitted as literal bytes.  Or
     *  [next_emit, ip_end) after the main loop.
     */
    ip_end = input + input_size;

    if (likely(input_size >= kinput_margin_bytes)) {
        const char *ip_limit = input + input_size -
            kinput_margin_bytes;

        for (next_hash = hash(++ip, shift);;) {
            DCHECK_LT(next_emit, ip);
/*
 * The body of this loop calls EmitLiteral once and then EmitCopy one or
 * more times.  (The exception is that when we're close to exhausting
 * the input we goto emit_remainder.)
 *
 * In the first iteration of this loop we're just starting, so
 * there's nothing to copy, so calling EmitLiteral once is
 * necessary.  And we only start a new iteration when the
 * current iteration has determined that a call to EmitLiteral will
 * precede the next call to EmitCopy (if any).
 *
 * Step 1: Scan forward in the input looking for a 4-byte-long match.
 * If we get close to exhausting the input then goto emit_remainder.
 *
 * Heuristic match skipping: If 32 bytes are scanned with no matches
 * found, start looking only at every other byte. If 32 more bytes are
 * scanned, look at every third byte, etc.. When a match is found,
 * immediately go back to looking at every byte. This is a small loss
 * (~5% performance, ~0.1% density) for lcompressible data due to more
 * bookkeeping, but for non-compressible data (such as JPEG) it's a huge
 * win since the compressor quickly "realizes" the data is incompressible
 * and doesn't bother looking for matches everywhere.
 *
 * The "skip" variable keeps track of how many bytes there are since the
 * last match; dividing it by 32 (ie. right-shifting by five) gives the
 * number of bytes to move ahead for each iteration.
 */
            skip = 32;

            next_ip = ip;
            do {
                ip = next_ip;
                hval = next_hash;
                DCHECK_EQ(hval, hash(ip, shift));
                bytes_between_hash_lookups = skip++ >> 5;
                next_ip = ip + bytes_between_hash_lookups;
                if (unlikely(next_ip > ip_limit)) {
                    goto emit_remainder;
                }
                next_hash = hash(next_ip, shift);
                candidate = baseip + table[hval];
                DCHECK_GE(candidate, baseip);
                DCHECK_LT(candidate, ip);

                table[hval] = ip - baseip;
            } while (likely(UNALIGNED_LOAD32(ip) !=
                    UNALIGNED_LOAD32(candidate)));

/*
 * Step 2: A 4-byte match has been found.  We'll later see if more
 * than 4 bytes match.  But, prior to the match, input
 * bytes [next_emit, ip) are unmatched.  Emit them as "literal bytes."
 */
            DCHECK_LE(next_emit + 16, ip_end);
            op = emit_literal(op, next_emit, ip - next_emit, 1);

/*
 * Step 3: Call EmitCopy, and then see if another EmitCopy could
 * be our next move.  Repeat until we find no match for the
 * input immediately after what was consumed by the last EmitCopy call.
 *
 * If we exit this loop normally then we need to call EmitLiteral next,
 * though we don't yet know how big the literal will be.  We handle that
 * by proceeding to the next iteration of the main loop.  We also can exit
 * this loop via goto if we get close to exhausting the input.
 */
            input_bytes = 0;
            candidate_bytes = 0;

            do {
/*
 * We have a 4-byte match at ip, and no need to emit any
 *  "literal bytes" prior to ip.
 */
                base = ip;
                matched = 4 +
                    find_match_length(candidate + 4, ip + 4,
                              ip_end);
                ip += matched;
                offset = base - candidate;
                DCHECK_EQ(0, memcmp(base, candidate, matched));
                op = emit_copy(op, offset, matched);
/*
 * We could immediately start working at ip now, but to improve
 * compression we first update table[Hash(ip - 1, ...)].
 */
                insert_tail = ip - 1;
                next_emit = ip;
                if (unlikely(ip >= ip_limit)) {
                    goto emit_remainder;
                }
                input_bytes = UNALIGNED_LOAD64(insert_tail);
                prev_hash =
                    hash_bytes(get_u32_at_offset
                           (input_bytes, 0), shift);
                table[prev_hash] = ip - baseip - 1;
                cur_hash =
                    hash_bytes(get_u32_at_offset
                           (input_bytes, 1), shift);
                candidate = baseip + table[cur_hash];
                candidate_bytes = UNALIGNED_LOAD32(candidate);
                table[cur_hash] = ip - baseip;
            } while (get_u32_at_offset(input_bytes, 1) ==
                 candidate_bytes);

            next_hash =
                hash_bytes(get_u32_at_offset(input_bytes, 2),
                       shift);
            ++ip;
        }
    }

emit_remainder:
    /* Emit the remaining bytes as a literal */
    if (next_emit < ip_end)
        op = emit_literal(op, next_emit, ip_end - next_emit, 0);

    return op;
}

/*
 * -----------------------------------------------------------------------
 *  Lookup table for decompression code.  Generated by ComputeTable() below.
 * -----------------------------------------------------------------------
 */

/* Mapping from i in range [0,4] to a mask to extract the bottom 8*i bits */
static const u32 wordmask[] = {
    0u, 0xffu, 0xffffu, 0xffffffu, 0xffffffffu
};

/*
 * Data stored per entry in lookup table:
 *       Range   Bits-used       Description
 *      ------------------------------------
 *      1..64   0..7            Literal/copy length encoded in opcode byte
 *      0..7    8..10           Copy offset encoded in opcode byte / 256
 *      0..4    11..13          Extra bytes after opcode
 *
 * We use eight bits for the length even though 7 would have sufficed
 * because of efficiency reasons:
 *      (1) Extracting a byte is faster than a bit-field
 *      (2) It properly aligns copy offset so we do not need a <<8
 */
static const u16 char_table[256] = {
    0x0001, 0x0804, 0x1001, 0x2001, 0x0002, 0x0805, 0x1002, 0x2002,
    0x0003, 0x0806, 0x1003, 0x2003, 0x0004, 0x0807, 0x1004, 0x2004,
    0x0005, 0x0808, 0x1005, 0x2005, 0x0006, 0x0809, 0x1006, 0x2006,
    0x0007, 0x080a, 0x1007, 0x2007, 0x0008, 0x080b, 0x1008, 0x2008,
    0x0009, 0x0904, 0x1009, 0x2009, 0x000a, 0x0905, 0x100a, 0x200a,
    0x000b, 0x0906, 0x100b, 0x200b, 0x000c, 0x0907, 0x100c, 0x200c,
    0x000d, 0x0908, 0x100d, 0x200d, 0x000e, 0x0909, 0x100e, 0x200e,
    0x000f, 0x090a, 0x100f, 0x200f, 0x0010, 0x090b, 0x1010, 0x2010,
    0x0011, 0x0a04, 0x1011, 0x2011, 0x0012, 0x0a05, 0x1012, 0x2012,
    0x0013, 0x0a06, 0x1013, 0x2013, 0x0014, 0x0a07, 0x1014, 0x2014,
    0x0015, 0x0a08, 0x1015, 0x2015, 0x0016, 0x0a09, 0x1016, 0x2016,
    0x0017, 0x0a0a, 0x1017, 0x2017, 0x0018, 0x0a0b, 0x1018, 0x2018,
    0x0019, 0x0b04, 0x1019, 0x2019, 0x001a, 0x0b05, 0x101a, 0x201a,
    0x001b, 0x0b06, 0x101b, 0x201b, 0x001c, 0x0b07, 0x101c, 0x201c,
    0x001d, 0x0b08, 0x101d, 0x201d, 0x001e, 0x0b09, 0x101e, 0x201e,
    0x001f, 0x0b0a, 0x101f, 0x201f, 0x0020, 0x0b0b, 0x1020, 0x2020,
    0x0021, 0x0c04, 0x1021, 0x2021, 0x0022, 0x0c05, 0x1022, 0x2022,
    0x0023, 0x0c06, 0x1023, 0x2023, 0x0024, 0x0c07, 0x1024, 0x2024,
    0x0025, 0x0c08, 0x1025, 0x2025, 0x0026, 0x0c09, 0x1026, 0x2026,
    0x0027, 0x0c0a, 0x1027, 0x2027, 0x0028, 0x0c0b, 0x1028, 0x2028,
    0x0029, 0x0d04, 0x1029, 0x2029, 0x002a, 0x0d05, 0x102a, 0x202a,
    0x002b, 0x0d06, 0x102b, 0x202b, 0x002c, 0x0d07, 0x102c, 0x202c,
    0x002d, 0x0d08, 0x102d, 0x202d, 0x002e, 0x0d09, 0x102e, 0x202e,
    0x002f, 0x0d0a, 0x102f, 0x202f, 0x0030, 0x0d0b, 0x1030, 0x2030,
    0x0031, 0x0e04, 0x1031, 0x2031, 0x0032, 0x0e05, 0x1032, 0x2032,
    0x0033, 0x0e06, 0x1033, 0x2033, 0x0034, 0x0e07, 0x1034, 0x2034,
    0x0035, 0x0e08, 0x1035, 0x2035, 0x0036, 0x0e09, 0x1036, 0x2036,
    0x0037, 0x0e0a, 0x1037, 0x2037, 0x0038, 0x0e0b, 0x1038, 0x2038,
    0x0039, 0x0f04, 0x1039, 0x2039, 0x003a, 0x0f05, 0x103a, 0x203a,
    0x003b, 0x0f06, 0x103b, 0x203b, 0x003c, 0x0f07, 0x103c, 0x203c,
    0x0801, 0x0f08, 0x103d, 0x203d, 0x1001, 0x0f09, 0x103e, 0x203e,
    0x1801, 0x0f0a, 0x103f, 0x203f, 0x2001, 0x0f0b, 0x1040, 0x2040
};

struct snappy_decompressor {
    struct source *reader;    /* Underlying source of bytes to decompress */
    const char *ip;        /* Points to next buffered byte */
    const char *ip_limit;    /* Points just past buffered bytes */
    u32 peeked;        /* Bytes peeked from reader (need to skip) */
    int eof;        /* Hit end of input without an error? */
    char scratch[5];    /* Temporary buffer for peekfast boundaries */
};

static void
init_snappy_decompressor(struct snappy_decompressor *d, struct source *reader)
{
    d->reader = reader;
    d->ip = NULL;
    d->ip_limit = NULL;
    d->peeked = 0;
    d->eof = 0;
}

static void exit_snappy_decompressor(struct snappy_decompressor *d)
{
    skip(d->reader, d->peeked);
}

/*
 * Read the uncompressed length stored at the start of the compressed data.
 * On succcess, stores the length in *result and returns true.
 * On failure, returns false.
 */
static int read_uncompressed_length(struct snappy_decompressor *d,
                     u32 * result)
{
    u32 shift = 0;
    size_t n;
        const char *ip;
        unsigned char c;
    DCHECK(d->ip == NULL);    /*
                 * Must not have read anything yet
                 * Length is encoded in 1..5 bytes
                 */
    *result = 0;
    while (1) {
        if (shift >= 32)
            return 0;
        ip = peek(d->reader, &n);
        if (n == 0)
            return 0;
        c = *(const unsigned char *)ip;
        skip(d->reader, 1);
        *result |= (u32) (c & 0x7f) << shift;
        if (c < 128) {
            break;
        }
        shift += 7;
    }
    return 1;
}

static int refill_tag(struct snappy_decompressor *d);

/*
 * Process the next item found in the input.
 * Returns true if successful, false on error or end of input.
 */
static void decompress_all_tags(struct snappy_decompressor *d,
                struct writer *writer)
{
        unsigned char c;
        u32 avail;
        u32 copy_offset;
    size_t n;
    const char *ip = d->ip;

    /*
     * We could have put this refill fragment only at the beginning of the loop.
     * However, duplicating it at the end of each branch gives the compiler more
     * scope to optimize the <ip_limit_ - ip> expression based on the local
     * context, which overall increases speed.
     */
#define MAYBE_REFILL() \
        if (d->ip_limit - ip < 5) {        \
        d->ip = ip;            \
        if (!refill_tag(d)) return;    \
        ip = d->ip;            \
        }


    MAYBE_REFILL();
    for (;;) {
        if (d->ip_limit - ip < 5) {
            d->ip = ip;
            if (!refill_tag(d))
                return;
            ip = d->ip;
        }

        c = *(unsigned char *)(ip++);

        if ((c & 0x3) == LITERAL) {
            u32 literal_length = (c >> 2) + 1;
            if (writer_try_fast_append(writer, ip, d->ip_limit - ip, 
                           literal_length)) {
                DCHECK_LT(literal_length, 61);
                ip += literal_length;
                MAYBE_REFILL();
                continue;
            }
            if (unlikely(literal_length >= 61)) {
                /* Long literal */
                const u32 literal_ll = literal_length - 60;
                literal_length = (get_unaligned_le32(ip) &
                          wordmask[literal_ll]) + 1;
                ip += literal_ll;
            }

            avail = d->ip_limit - ip;
            while (avail < literal_length) {
                if (!writer_append(writer, ip, avail))
                    return;
                literal_length -= avail;
                skip(d->reader, d->peeked);
                ip = peek(d->reader, &n);
                avail = n;
                d->peeked = avail;
                if (avail == 0)
                    return;    /* Premature end of input */
                d->ip_limit = ip + avail;
            }
            if (!writer_append(writer, ip, literal_length))
                return;
            ip += literal_length;
            MAYBE_REFILL();
        } else {
            const u32 entry = char_table[c];
            const u32 trailer = get_unaligned_le32(ip) &
                wordmask[entry >> 11];
            const u32 length = entry & 0xff;
            ip += entry >> 11;

            /*
             * copy_offset/256 is encoded in bits 8..10.
             * By just fetching those bits, we get
             * copy_offset (since the bit-field starts at
             * bit 8).
             */
            copy_offset = entry & 0x700;
            if (!writer_append_from_self(writer,
                             copy_offset + trailer,
                             length))
                return;
            MAYBE_REFILL();
        }
    }
}

#undef MAYBE_REFILL

static int refill_tag(struct snappy_decompressor *d)
{
        unsigned char c;
        u32 entry;
        u32 needed;
        u32 nbuf;
        u32 to_add;
    const char *ip = d->ip;

    if (ip == d->ip_limit) {
        size_t n;
        /* Fetch a new fragment from the reader */
        skip(d->reader, d->peeked); /* All peeked bytes are used up */
        ip = peek(d->reader, &n);
        d->peeked = n;
        if (n == 0) {
            d->eof = 1;
            return 0;
        }
        d->ip_limit = ip + n;
    }

    /* Read the tag character */
    DCHECK_LT(ip, d->ip_limit);
    c = *(unsigned char *)(ip);
    entry = char_table[c];
    needed = (entry >> 11) + 1;    /* +1 byte for 'c' */
    DCHECK_LE(needed, sizeof(d->scratch));

    /* Read more bytes from reader if needed */
    nbuf = d->ip_limit - ip;

    if (nbuf < needed) {
        /*
         * Stitch together bytes from ip and reader to form the word
         * contents.  We store the needed bytes in "scratch".  They
         * will be consumed immediately by the caller since we do not
         * read more than we need.
         */
        memmove(d->scratch, ip, nbuf);
        skip(d->reader, d->peeked); /* All peeked bytes are used up */
        d->peeked = 0;
        while (nbuf < needed) {
            size_t length;
            const char *src = peek(d->reader, &length);
            if (length == 0)
                return 0;
            to_add = min_t(u32, needed - nbuf, length);
            memcpy(d->scratch + nbuf, src, to_add);
            nbuf += to_add;
            skip(d->reader, to_add);
        }
        DCHECK_EQ(nbuf, needed);
        d->ip = d->scratch;
        d->ip_limit = d->scratch + needed;
    } else if (nbuf < 5) {
        /*
         * Have enough bytes, but move into scratch so that we do not
         * read past end of input
         */
        memmove(d->scratch, ip, nbuf);
        skip(d->reader, d->peeked); /* All peeked bytes are used up */
        d->peeked = 0;
        d->ip = d->scratch;
        d->ip_limit = d->scratch + nbuf;
    } else {
        /* Pass pointer to buffer returned by reader. */
        d->ip = ip;
    }
    return 1;
}

static int internal_uncompress(struct source *r,
                   struct writer *writer, u32 max_len)
{
    struct snappy_decompressor decompressor;
    u32 uncompressed_len = 0;

    init_snappy_decompressor(&decompressor, r);

    if (!read_uncompressed_length(&decompressor, &uncompressed_len))
        return -EIO;
    /* Protect against possible DoS attack */
    if ((u64) (uncompressed_len) > max_len)
        return -EIO;

    writer_set_expected_length(writer, uncompressed_len);

    /* Process the entire input */
    decompress_all_tags(&decompressor, writer);

    exit_snappy_decompressor(&decompressor);
    return (decompressor.eof && writer_check_length(writer)) ? 0 : -EIO;
}

static OSPL_INLINE int compress(struct snappy_env *env, struct source *reader,
               struct sink *writer)
{
        int num_to_read;
        size_t bytes_read;
        int pending_advance;
    int err;
    int table_size;
        int max_output;
    size_t written = 0;
    size_t fragment_size;
    int N = available(reader);
    char ulength[kmax32];
    char *p = varint_encode32(ulength, N);
        u16 *table;
    char *dest;
    char *end;

    append(writer, ulength, p - ulength);
    written += (p - ulength);

    while (N > 0) {
        /* Get next block to compress (without copying if possible) */
        const char *fragment = peek(reader, &fragment_size);
        if (fragment_size == 0) {
            err = -EIO;
            goto out;
        }
        num_to_read = min_t(int, N, kblock_size);
        bytes_read = fragment_size;

        pending_advance = 0;
        if (bytes_read >= num_to_read) {
            /* Buffer returned by reader is large enough */
            pending_advance = num_to_read;
            fragment_size = num_to_read;
        }
        else {
                        size_t n;
            memcpy(env->scratch, fragment, bytes_read);
            skip(reader, bytes_read);

            while (bytes_read < num_to_read) {
                fragment = peek(reader, &fragment_size);
                n = min_t(size_t, fragment_size,
                      num_to_read - bytes_read);
                memcpy(((char*)env->scratch) + bytes_read, fragment, n);
                bytes_read += n;
                skip(reader, n);
            }
            DCHECK_EQ(bytes_read, num_to_read);
            fragment = env->scratch;
            fragment_size = num_to_read;
        }
        if (fragment_size < num_to_read)
            return -EIO;

        /* Get encoding table for compression */
        table = get_hash_table(env, num_to_read, &table_size);

        /* Compress input_fragment and append to dest */
        max_output = snappy_max_compressed_length(num_to_read);

        dest = sink_peek(writer, max_output);
        if (!dest) {
            /*
             * Need a scratch buffer for the output,
             * because the byte sink doesn't have enough
             * in one piece.
             */
            dest = env->scratch_output;
        }
        end = compress_fragment(fragment, fragment_size,
                          dest, table, table_size);
        append(writer, dest, end - dest);
        written += (end - dest);

        N -= num_to_read;
        skip(reader, pending_advance);
    }

    err = 0;
out:
    return err;
}

#ifdef SG

int snappy_compress_iov(struct snappy_env *env,
            struct iovec *iov_in,
            int iov_in_len,
            size_t input_length,
            struct iovec *iov_out,
            int iov_out_len,
            size_t *compressed_length)
{
    struct source reader = {
        .iov = iov_in,
        .iovlen = iov_in_len,
        .total = input_length
    };
    struct sink writer = {
        .iov = iov_out,
        .iovlen = iov_out_len,
    };
    int err = compress(env, &reader, &writer);

    /* Compute how many bytes were added */
    *compressed_length = writer.written;
    return err;
}
EXPORT_SYMBOL(snappy_compress_iov);

/**
 * snappy_compress - Compress a buffer using the snappy compressor.
 * @env: Preallocated environment
 * @input: Input buffer
 * @input_length: Length of input_buffer
 * @compressed: Output buffer for compressed data
 * @compressed_length: The real length of the output written here.
 *
 * Return 0 on success, otherwise an negative error code.
 *
 * The output buffer must be at least
 * snappy_max_compressed_length(input_length) bytes long.
 *
 * Requires a preallocated environment from snappy_init_env.
 * The environment does not keep state over individual calls
 * of this function, just preallocates the memory.
 */
int snappy_compress(struct snappy_env *env,
            const char *input,
            size_t input_length,
            char *compressed, size_t *compressed_length)
{
    struct iovec iov_in = {
        .iov_base = (char *)input,
        .iov_len = input_length,
    };
    struct iovec iov_out = {
        .iov_base = compressed,
        .iov_len = 0xffffffff,
    };        
    return snappy_compress_iov(env, 
                   &iov_in, 1, input_length, 
                   &iov_out, 1, compressed_length);
}
EXPORT_SYMBOL(snappy_compress);

int snappy_uncompress_iov(struct iovec *iov_in, int iov_in_len,
               size_t input_len, char *uncompressed)
{
    struct source reader = {
        .iov = iov_in,
        .iovlen = iov_in_len,
        .total = input_len
    };
    struct writer output = {
        .base = uncompressed,
        .op = uncompressed
    };
    return internal_uncompress(&reader, &output, 0xffffffff);
}
EXPORT_SYMBOL(snappy_uncompress_iov);

/**
 * snappy_uncompress - Uncompress a snappy compressed buffer
 * @compressed: Input buffer with compressed data
 * @n: length of compressed buffer
 * @uncompressed: buffer for uncompressed data
 *
 * The uncompressed data buffer must be at least
 * snappy_uncompressed_length(compressed) bytes long.
 *
 * Return 0 on success, otherwise an negative error code.
 */
int snappy_uncompress(const char *compressed, size_t n, char *uncompressed)
{
    struct iovec iov = {
        .iov_base = (char *)compressed,
        .iov_len = n
    };
    return snappy_uncompress_iov(&iov, 1, n, uncompressed);
}
EXPORT_SYMBOL(snappy_uncompress);

#else
/**
 * snappy_compress - Compress a buffer using the snappy compressor.
 * @env: Preallocated environment
 * @input: Input buffer
 * @input_length: Length of input_buffer
 * @compressed: Output buffer for compressed data
 * @compressed_length: The real length of the output written here.
 *
 * Return 0 on success, otherwise an negative error code.
 *
 * The output buffer must be at least
 * snappy_max_compressed_length(input_length) bytes long.
 *
 * Requires a preallocated environment from snappy_init_env.
 * The environment does not keep state over individual calls
 * of this function, just preallocates the memory.
 */
int snappy_compress(struct snappy_env *env,
            const char *input,
            size_t input_length,
            char *compressed, size_t *compressed_length)
{
    struct source reader;
    struct sink writer;
        int err;

        reader.ptr = input;
        reader.left = input_length;
        writer.dest = compressed;
    err = compress(env, &reader, &writer);

    /* Compute how many bytes were added */
    *compressed_length = (writer.dest - compressed);
    return err;
}
EXPORT_SYMBOL(snappy_compress)

/**
 * snappy_uncompress - Uncompress a snappy compressed buffer
 * @compressed: Input buffer with compressed data
 * @n: length of compressed buffer
 * @uncompressed: buffer for uncompressed data
 *
 * The uncompressed data buffer must be at least
 * snappy_uncompressed_length(compressed) bytes long.
 *
 * Return 0 on success, otherwise an negative error code.
 */
int snappy_uncompress(const char *compressed, size_t n, char *uncompressed)
{
    struct source reader;
    struct writer output;
        reader.ptr = compressed;
        reader.left = n;
        output.base = uncompressed;
        output.op = uncompressed;
    return internal_uncompress(&reader, &output, 0xffffffff);
}
EXPORT_SYMBOL(snappy_uncompress)
#endif

#ifdef SG
/**
 * snappy_init_env_sg - Allocate snappy compression environment
 * @env: Environment to preallocate
 * @sg: Input environment ever does scather gather
 *
 * If false is passed to sg then multiple entries in an iovec
 * are not legal.
 * Returns 0 on success, otherwise negative errno.
 * Must run in process context.
 */
int snappy_init_env_sg(struct snappy_env *env, int sg)
{
    env->hash_table = vmalloc(sizeof(u16) * kmax_hash_table_size);
    if (!env->hash_table)
        goto error;
    if (sg) {
        env->scratch = vmalloc(kblock_size);
        if (!env->scratch)
            goto error;
        env->scratch_output =
            vmalloc(snappy_max_compressed_length(kblock_size));
        if (!env->scratch_output)
            goto error;
    }
    return 0;
error:
    snappy_free_env(env);
    return -ENOMEM;
}
EXPORT_SYMBOL(snappy_init_env_sg)
#endif

/**
 * snappy_init_env - Allocate snappy compression environment
 * @env: Environment to preallocate
 *
 * Passing multiple entries in an iovec is not allowed
 * on the environment allocated here.
 * Returns 0 on success, otherwise negative errno.
 * Must run in process context.
 */
int snappy_init_env(struct snappy_env *env)
{
    env->hash_table = vmalloc(sizeof(u16) * kmax_hash_table_size);
    if (!env->hash_table)
        return -ENOMEM;
    return 0;
}
EXPORT_SYMBOL(snappy_init_env)

/**
 * snappy_free_env - Free an snappy compression environment
 * @env: Environment to free.
 *
 * Must run in process context.
 */
void snappy_free_env(struct snappy_env *env)
{
    vfree(env->hash_table);
#ifdef SG
    vfree(env->scratch);
    vfree(env->scratch_output);
#endif
    memset(env, 0, sizeof(struct snappy_env));
}
EXPORT_SYMBOL(snappy_free_env)
