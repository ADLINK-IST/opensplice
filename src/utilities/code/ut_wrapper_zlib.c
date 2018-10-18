/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "os_report.h"
#include "ut_compressor.h"

/* ZLIB definitions from zlib.h
 */
#define ZLIB_VERSION "1.2.3.4"
#define Z_OK                     0
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)

typedef int (*_compress2)(unsigned char *dest, unsigned long *destLen,
                          const unsigned char *source, unsigned long sourceLen,
                          int level);
typedef int (*_uncompress)(unsigned char *dest,   unsigned long *destLen,
                           const unsigned char *source, unsigned long sourceLen);
typedef unsigned long (*_compressBound) (unsigned long sourceLen);
typedef const char *(*_zlibVersion)(void);

typedef struct wrapper_zlib {
    os_library lib;
    int level;
    _compress2 compress2;
    _uncompress uncompress;
    _compressBound compressBound;
    _zlibVersion zlibVersion;
} wrapper_zlib;

/* The zlib compress wrapper will return a buffer which the
 * following layout:
 * +=====================================================
 * |  0  |  1  |  2  |  3  |  4  |  5  |  6  | ... |  n
 * |flags|   uncompressed size   | compressed data
 * +=====================================================
 */
#define ZLIB_WRAPPER_OVERHEAD (5)

static unsigned toBE4u (unsigned x)
{
#ifdef PA_LITTLE_ENDIAN
    return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
#else
    return x;
#endif
}

static unsigned fromBE4u (unsigned x)
{
#ifdef PA_LITTLE_ENDIAN
    return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
#else
    return x;
#endif
}

static ut_result
wrapper_zlib_compress_maxsize (
    void *param,
    os_size_t srcLen,
    os_size_t *maxDstLen)
{
    ut_result result = UT_RESULT_OK;
    wrapper_zlib *wrapper = param;
    assert(param);

    *maxDstLen = wrapper->compressBound(srcLen) + ZLIB_WRAPPER_OVERHEAD;

    return result;
}

static ut_result
wrapper_zlib_uncompress_maxsize (
    void *param,
    const void *src,
    os_size_t srcLen,
    os_size_t *maxDstLen)
{
    ut_result result = UT_RESULT_OK;
    const os_uchar *ptr = src;
    os_uint32 sizeBE;

    assert(src);
    assert(maxDstLen);

    OS_UNUSED_ARG(param);

    if (srcLen >= ZLIB_WRAPPER_OVERHEAD) {
        memcpy(&sizeBE, &ptr[1], 4);
        *maxDstLen = fromBE4u(sizeBE);
    } else {
        result = UT_RESULT_ILL_PARAM;
        OS_REPORT(OS_WARNING, OS_FUNCTION, result,
                  "Illegal parameter: Supplied buffer size to small");
    }

    return result;
}

static ut_result
wrapper_zlib_compress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *compressedLen)
{
    ut_result result = UT_RESULT_OK;
    int res;
    wrapper_zlib *wrapper = param;
    unsigned long length;
    os_boolean allocation = OS_FALSE;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(compressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    length = wrapper->compressBound(srcLen);
    if ((*dst == NULL) && (*dstLen == 0)) {
        /* Let wrapper do the allocation */
        allocation = OS_TRUE;
        if ((*dst = os_malloc(length + ZLIB_WRAPPER_OVERHEAD)) == NULL) {
            result = UT_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to allocate memory (%lu bytes) for compression buffer", length);
        } else {
            *dstLen = length;
        }
    } else if (length > *dstLen) {
        result = UT_RESULT_COUNT;
    }

    if (result == UT_RESULT_OK) {
        os_uchar *ptr = *dst;
        os_uint32 sizeBE;
        unsigned char *buffer = (unsigned char *)(&ptr[5]);

        if ((res = wrapper->compress2(buffer, &length,(const unsigned char*) src, srcLen, wrapper->level)) == Z_OK) {
            ptr[0] = 0;
            sizeBE = toBE4u((os_uint32)srcLen);
            memcpy(&ptr[1], &sizeBE, 4);
            *compressedLen = length + ZLIB_WRAPPER_OVERHEAD;
        } else {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to compress with error '%d'", res);
            if (allocation) {
                os_free(*dst);
                *dst = NULL;
                *dstLen = 0;
            }
        }
    }

    return result;
}

static ut_result
wrapper_zlib_uncompress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *uncompressedLen)
{
    ut_result result = UT_RESULT_OK;
    int res;
    wrapper_zlib *wrapper = param;
    os_boolean allocation = OS_FALSE;
    const os_uchar *ptr = src;
    os_uint32 sizeBE;
    os_size_t length;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(uncompressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    if (srcLen >= ZLIB_WRAPPER_OVERHEAD) {
        memcpy(&sizeBE, &ptr[1], 4);
        length = fromBE4u(sizeBE);
        if ((*dst == NULL) && (*dstLen == 0)) {
            /* Let wrapper do the allocation */
            allocation = OS_TRUE;
            if ((*dst = os_malloc(length)) == NULL) {
                result = UT_RESULT_OUT_OF_MEMORY;
                OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                          "Failed to allocate memory (%"PA_PRIuSIZE" bytes) for decompression buffer", length);
            } else {
                *dstLen = length;
            }
        } else if (length > *dstLen) {
            result = UT_RESULT_COUNT;
        }
    } else {
        result = UT_RESULT_ILL_PARAM;
        OS_REPORT(OS_WARNING, OS_FUNCTION, result,
                "Illegal parameter: Supplied buffer size to small");
    }

    if (result == UT_RESULT_OK) {
        const unsigned char *buffer = (const unsigned char *)(&ptr[5]);
        unsigned long tmp_dstLen = *dstLen;
        unsigned long tmp_srcLen = srcLen - ZLIB_WRAPPER_OVERHEAD;
        res = wrapper->uncompress (*dst, &tmp_dstLen, buffer, tmp_srcLen);
        if (res == Z_OK) {
            *uncompressedLen = tmp_dstLen;
        } else {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to uncompress with error '%d'", res);
            if (allocation) {
                os_free(*dst);
                *dst = NULL;
                *dstLen = 0;
            }
        }
    }
    return result;
}

static void
wrapper_zlib_exit (
    void *param)
{
    wrapper_zlib *wrapper = param;
    assert(param);
    os_libraryClose(wrapper->lib);
    os_free (param);
}

ut_result
ut__wrapper_zlib_init (
    ut_compressor compressor,
    const char *libName,
    const char *parameter)
{
    ut_result result = UT_RESULT_ERROR;
    os_libraryAttr attr;
    wrapper_zlib *wrapper;
    int level;

    assert(compressor);

    if (libName == NULL) {
#ifdef _WIN32
        libName = "zlib1";
#else
        libName = "z";
#endif
    }

    if (parameter != NULL) {
        level = atoi(parameter);
        if ((level < Z_BEST_SPEED || level > Z_BEST_COMPRESSION)
            && level != Z_DEFAULT_COMPRESSION
            && level != Z_NO_COMPRESSION ) {
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Supplied compression level '%d' is invalid", level);
            return UT_RESULT_ILL_PARAM;
        }
    } else {
        level = Z_DEFAULT_COMPRESSION;
    }

    os_libraryAttrInit (&attr);
    wrapper = os_malloc(sizeof(wrapper_zlib));
    wrapper->lib = os_libraryOpen(libName, &attr);
    if (wrapper->lib != NULL) {
        /* Writing: wrapper->init_env = (zlib_init_env)os_libraryGetSymbol (compressor->lib, zlib_init_env);
         * would seem more natural, but results in: warning: ISO C forbids conversion of object pointer
         * to function pointer type [-pedantic]
         * dlopen man page suggests using the used assignment based on the POSIX.1-2003 (Technical
         * Corrigendum 1) workaround.
         */
        *(void **)&wrapper->compress2 = os_libraryGetSymbol (wrapper->lib, "compress2");
        *(void **)&wrapper->uncompress = os_libraryGetSymbol (wrapper->lib, "uncompress");
        *(void **)&wrapper->compressBound = os_libraryGetSymbol (wrapper->lib, "compressBound");
        *(void **)&wrapper->zlibVersion = os_libraryGetSymbol (wrapper->lib, "zlibVersion");

        if (wrapper->compress2 &&
            wrapper->uncompress &&
            wrapper->compressBound &&
            wrapper->zlibVersion) {
            static const char *my_version = ZLIB_VERSION;
            const char *version = wrapper->zlibVersion();
            if (my_version[0] == version[0]) {
                wrapper->level = level;
                compressor->param = wrapper;
                compressor->version = version;
                compressor->compfn = wrapper_zlib_compress;
                compressor->uncompfn = wrapper_zlib_uncompress;
                compressor->compmaxfn = wrapper_zlib_compress_maxsize;
                compressor->uncompmaxfn = wrapper_zlib_uncompress_maxsize;
                compressor->exitfn = wrapper_zlib_exit;
                result = UT_RESULT_OK;
            } else {
                os_libraryClose(wrapper->lib);
                os_free(wrapper);
                OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                          "The zlib library version (%s) is incompatible with assumed version (%s)",
                          version, my_version);
            }
        } else {
            os_libraryClose(wrapper->lib);
            os_free(wrapper);
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "At least one of required functions 'compress2,uncompress,compressBound,zlibVersion' not found");
        }
    } else {
        os_free(wrapper);
    }

    return result;
}
