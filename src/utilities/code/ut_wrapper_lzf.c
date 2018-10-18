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
#include "os_errno.h"

typedef unsigned int (*_lzf_compress) (const void *const in_data,  unsigned int in_len,
                                       void             *out_data, unsigned int out_len);
typedef unsigned int (*_lzf_decompress) (const void *const in_data,  unsigned int in_len,
                                         void             *out_data, unsigned int out_len);

typedef struct wrapper_lzf {
    os_library lib;
    _lzf_compress compress;
    _lzf_decompress decompress;
} wrapper_lzf;

/* The lzf compress wrapper will return a buffer which the
 * following layout:
 * +=====================================================
 * |  0  |  1  |  2  |  3  |  4  |  5  |  6  | ... |  n
 * |flags|   uncompressed size   | compressed data
 * +=====================================================
 */
#define LZF_WRAPPER_OVERHEAD (5)

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
wrapper_lzf_compress_maxsize (
    void *param,
    os_size_t srcLen,
    os_size_t *maxDstLen)
{
    assert(maxDstLen);

    OS_UNUSED_ARG(param);
    OS_UNUSED_ARG(srcLen);

    *maxDstLen = srcLen + (srcLen >> 4) + 16 + LZF_WRAPPER_OVERHEAD;

    return UT_RESULT_OK;
}

static ut_result
wrapper_lzf_uncompress_maxsize (
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

    if (srcLen >= LZF_WRAPPER_OVERHEAD) {
        memcpy(&sizeBE, &ptr[1], 4);
        *maxDstLen = fromBE4u(sizeBE);
    } else {
        result = UT_RESULT_ILL_PARAM;
        OS_REPORT(OS_WARNING, OS_FUNCTION, result,
                  "Illegal parameter: Supplied source buffer size to small");
    }

    return result;
}

static ut_result
wrapper_lzf_compress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *compressedLen)
{
    ut_result result = UT_RESULT_OK;
    unsigned int res;
    wrapper_lzf *wrapper = param;
    os_size_t length;
    os_boolean allocation = OS_FALSE;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(compressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    (void)wrapper_lzf_compress_maxsize(param, srcLen, &length);
    if ((*dst == NULL) && (*dstLen == 0)) {
        /* Let wrapper do the allocation */
        allocation = OS_TRUE;
        if ((*dst = os_malloc(length)) == NULL) {
            result = UT_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to allocate memory (%"PA_PRIuSIZE" bytes) for compression buffer", length);
        } else {
            *dstLen = length;
        }
    } else if (length > *dstLen) {
        result = UT_RESULT_COUNT;
    }

    if (result == UT_RESULT_OK) {
        os_uchar *ptr = *dst;
        os_uint32 sizeBE;
        void *buffer = (void*)(&ptr[5]);

        if ((res = wrapper->compress(src, (unsigned)srcLen, buffer, (unsigned)*dstLen)) == 0) {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to compress with unknown error");
            if (allocation) {
                os_free(*dst);
                *dst = NULL;
                *dstLen = 0;
            }
        } else {
            ptr[0] = 0;
            sizeBE = toBE4u((os_uint32)srcLen);
            memcpy(&ptr[1], &sizeBE, 4);
            *compressedLen = res + LZF_WRAPPER_OVERHEAD;
        }
    }

    return result;
}

static ut_result
wrapper_lzf_uncompress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *uncompressedLen)
{
    ut_result result = UT_RESULT_OK;
    unsigned int res;
    wrapper_lzf *wrapper = param;
    os_size_t length;
    os_boolean allocation = OS_FALSE;
    const os_uchar *ptr = src;
    os_uint32 sizeBE;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(uncompressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    if (srcLen >= LZF_WRAPPER_OVERHEAD) {
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
                "Illegal parameter: Supplied source buffer size to small");
    }

    if (result == UT_RESULT_OK) {
        void *buffer = (void*)(&ptr[5]);
        unsigned int buffer_sz = (unsigned)srcLen - LZF_WRAPPER_OVERHEAD;
        res = wrapper->decompress (buffer, buffer_sz, *dst, (unsigned)length);
        if (res != 0) {
            *uncompressedLen = res;
            result = UT_RESULT_OK;
        } else if (os_getErrno() == E2BIG) {
            result = UT_RESULT_COUNT;
        } else {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to decompress with unknown error");
        }
        if ((result != UT_RESULT_OK) &&
            (allocation == OS_TRUE)) {
            os_free(*dst);
            *dst = NULL;
            *dstLen = 0;
        }
    }

    return result;
}

static void
wrapper_lzf_exit (
    void *param)
{
    wrapper_lzf *wrapper = param;
    assert(param);
    (void)os_libraryClose(wrapper->lib);
    os_free (param);
}

ut_result
ut__wrapper_lzf_init (
    ut_compressor compressor,
    const char* libName)
{
    ut_result result = UT_RESULT_ERROR;
    os_libraryAttr attr;
    wrapper_lzf *wrapper;
    char *version = "unknown";

    assert(compressor);

    if (libName == NULL) {
#ifndef INCLUDE_LZF
        result = UT_RESULT_NOT_IMPLEMENTED;
        OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                  "The built-in lzf compressor is not available for this platform");
        return result;
#endif
        version = "1.5";
        libName = "lzf-ospl";
    }

    os_libraryAttrInit (&attr);
    wrapper = os_malloc(sizeof(wrapper_lzf));
    wrapper->lib = os_libraryOpen(libName, &attr);
    if (wrapper->lib != NULL) {
        /* Writing: wrapper->init_env = (lzf_init_env)os_libraryGetSymbol (compressor->lib, lzf_init_env);
         * would seem more natural, but results in: warning: ISO C forbids conversion of object pointer
         * to function pointer type [-pedantic]
         * dlopen man page suggests using the used assignment based on the POSIX.1-2003 (Technical
         * Corrigendum 1) workaround.
         */
        *(void **)&wrapper->compress = os_libraryGetSymbol (wrapper->lib, "lzf_compress");
        *(void **)&wrapper->decompress = os_libraryGetSymbol (wrapper->lib, "lzf_decompress");

        if (wrapper->compress &&
            wrapper->decompress) {
            compressor->param = wrapper;
            compressor->version = version;
            compressor->compfn = wrapper_lzf_compress;
            compressor->uncompfn = wrapper_lzf_uncompress;
            compressor->compmaxfn = wrapper_lzf_compress_maxsize;
            compressor->uncompmaxfn = wrapper_lzf_uncompress_maxsize;
            compressor->exitfn = wrapper_lzf_exit;
            result = UT_RESULT_OK;
        } else {
            os_libraryClose(wrapper->lib);
            os_free(wrapper);
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "At least one of required functions 'lzf_compress,lzf_decompress' not found");
        }
    } else {
        os_free(wrapper);
    }
    return result;
}

