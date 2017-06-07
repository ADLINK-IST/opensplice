/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "os_heap.h"
#include "os_abstract.h"
#include <string.h>

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_report.h"

#define MEM_ALIGNMENT   8
#define HMM_MAGIC       (0xabcdefed)

#define ALIGN_SIZE(value) ((((value) + MEM_ALIGNMENT - 1)/MEM_ALIGNMENT)*MEM_ALIGNMENT)

#define CHECK_REF (0)

#if CHECK_REF
#define CHECK_REF_DEPTH (32)
static char* CHECK_REF_FILE = NULL;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(16); \
        os_sprintf(CHECK_REF_FILE, "heap.log"); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);              \
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    fprintf(stream, "\n\n\n"); \
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
#else
#define UT_TRACE(msgFormat, ...)
#endif

typedef struct {
    DDS_deallocatorType deallocator;
    DDS_unsigned_long magic;
    DDS_unsigned_long refCount;
    void *alloc_addr;
} contextHeader;

const DDS_unsigned_long CONTEXTHEADER_SIZE = ALIGN_SIZE(sizeof(contextHeader));

void *
DDS__malloc (
    DDS_deallocatorType deallocator,
    DDS_unsigned_long hl,
    DDS_unsigned_long len)
{
    unsigned long totlen;
    void *header;
    void *data = NULL;
    contextHeader *context;

    totlen = ALIGN_SIZE(hl) + CONTEXTHEADER_SIZE + len;
    header = os_malloc(totlen);
    memset(header, 0, totlen);
    context = (contextHeader *)((PA_ADDRCAST)header + ALIGN_SIZE(hl));
    data = (void *)((PA_ADDRCAST)context + CONTEXTHEADER_SIZE);
    context->deallocator = deallocator;
    context->magic = HMM_MAGIC;
    context->alloc_addr = header;
    context->refCount = 1;
    return data;
}

void *
DDS_alloc (
    DDS_unsigned_long len,
    DDS_deallocatorType deallocator)
{
    unsigned long totlen;
    void *data = NULL;
    contextHeader *header;

    totlen = CONTEXTHEADER_SIZE + len;
    header = os_malloc(totlen);
    memset(header, 0, totlen);
    header->deallocator = deallocator;
    header->magic = HMM_MAGIC;
    header->alloc_addr = header;
    header->refCount = 1;
    data = (void *)((PA_ADDRCAST)header + CONTEXTHEADER_SIZE);
    return data;
}

void *
DDS__header (
    void *object)
{
    contextHeader *context;

    if (object != NULL) {
        context = (contextHeader *)((PA_ADDRCAST)object - CONTEXTHEADER_SIZE);
        if (context->magic == HMM_MAGIC) {
            return context->alloc_addr;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

DDS_unsigned_long
DDS__refCount(
    void *object)
{
    DDS_unsigned_long count = 0;
    contextHeader *context;

    if (object != NULL) {
        context = (contextHeader *)((PA_ADDRCAST)object - CONTEXTHEADER_SIZE);
        if (context->magic == HMM_MAGIC) {
            count = context->refCount;
        }
    }
    return count;
}

DDS_ReturnCode_t
DDS_keep (
    void *object)
{
    contextHeader *context;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (object != NULL) {
        context = (contextHeader *)((PA_ADDRCAST)object - CONTEXTHEADER_SIZE);
        if (context->magic == HMM_MAGIC) {
            context->refCount++;
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    return result;
}

DDS_ReturnCode_t
DDS__free (
    void *object)
{
    contextHeader *context;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (object != NULL) {
        context = (contextHeader *)((PA_ADDRCAST)object - CONTEXTHEADER_SIZE);
        if (context->magic == HMM_MAGIC) {
            if (context->refCount == 1) {
                if (context->deallocator != NULL) {
                    result = context->deallocator(object);
                }
                if (result == DDS_RETCODE_OK) {
                    context->magic = 0;
                    context->refCount = 0;
                    os_free(context->alloc_addr);
                }
            } else if (context->refCount > 1) {
                context->refCount--;
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Invalid object reference");
        }
    }
    return result;
}

void
DDS_free (
    void *object)
{
    DDS_ReturnCode_t result;

    /*
     * DDS_free should not be able to free interfaces; that should be
     * done in the related factories, when possible.
     *
     * Problem is, we receive a void pointer and there's no way we can
     * figure out what the actual data type is from that.
     * We can not determine if we received a DDS_string, DDS_sequence,
     * DDS_Object or whatever.
     * But we have to know if it is an DDS_Object to be able to get the
     * object kind to be able to determine if it is an interface or not.
     *
     * Conclusion:
     * For now, the interface deletion restriction is not implemented.
     */

    SAC_REPORT_STACK();

    /* Let the internal free do the checks, calls, frees and whatnot. */
    result = DDS__free(object);
    SAC_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
}

DDS_char *
DDS_string_alloc (
    DDS_unsigned_long len)
{
    return DDS_alloc(len+1, NULL);
}

DDS_char *
DDS_string_dup (
    const DDS_char *src)
{
    DDS_char *dst;

    if (src != NULL) {
        size_t len = strlen(src);
        assert (len == (DDS_unsigned_long)len);
        dst = DDS_string_alloc((DDS_unsigned_long)len);
        memcpy(dst, src, len+1);
    } else {
        dst = NULL;
    }
    return dst;
}

DDS_char *
DDS_string_dup_no_spaces (
    const DDS_char *src)
{
    DDS_char *dst;
    const DDS_char *p1;
    DDS_char *p2;
    DDS_boolean skip = TRUE;

    if (src != NULL) {
        size_t len = strlen(src);
        assert (len == (DDS_unsigned_long)len);
        dst = DDS_string_alloc((DDS_unsigned_long)len);
        if (dst != NULL) {
            p1 = src;
            p2 = dst;
            while (*p1 != '\0') {
                if (skip) {
                    if ( *p1 == ' ' ) {
                        p1++;
                    } else if ( *p1 == '<' ) {
                        skip = FALSE;
                        *p2++ = *p1++;
                    } else {
                        *p2++ = *p1++;
                    }
                } else {
                    if ( *p1 == '>' ) {
                        skip = TRUE;
                    }
                    *p2++ = *p1++;
                }
            }
            *p2 = '\0';
        }
    } else {
        dst = NULL;
    }
    return dst;
}

void
DDS_string_clean (
    DDS_char **string)
{
    if ((string != NULL) && (*string != NULL)) {
        DDS_free (*string);
        *string = NULL;
    }
}

void
DDS_string_replace (
    const DDS_char *src,
    DDS_char **dst)
{
    if (*dst != NULL) {
        DDS_free (*dst);
    }
    if (src) {
        *dst = DDS_string_dup (src);
    } else {
        *dst = NULL;
    }
}

struct DDS_TopicListener *
DDS_TopicListener__alloc (
    void)
{
    return (struct DDS_TopicListener *)
        DDS_alloc(sizeof(struct DDS_TopicListener), NULL);
}

struct DDS_ExtTopicListener *
DDS_ExtTopicListener__alloc (
    void)
{
    return (struct DDS_ExtTopicListener *)
        DDS_alloc(sizeof(struct DDS_ExtTopicListener), NULL);

}

struct DDS_DataWriterListener *
DDS_DataWriterListener__alloc (
    void)
{
    return (struct DDS_DataWriterListener *)
        DDS_alloc(sizeof(struct DDS_DataWriterListener), NULL);

}

struct DDS_PublisherListener *
DDS_PublisherListener__alloc (
    void)
{
    return(struct DDS_PublisherListener *)
        DDS_alloc(sizeof(struct DDS_PublisherListener), NULL);
}

struct DDS_DataReaderListener *
DDS_DataReaderListener__alloc (
    void)
{
    return (struct DDS_DataReaderListener *)
        DDS_alloc(sizeof(struct DDS_DataReaderListener), NULL);
}

struct DDS_SubscriberListener *
DDS_SubscriberListener__alloc (
    void)
{
    return (struct DDS_SubscriberListener *)
        DDS_alloc(sizeof(struct DDS_SubscriberListener), NULL);

}

struct DDS_DomainParticipantListener *
DDS_DomainParticipantListener__alloc (
    void)
{
    return (struct DDS_DomainParticipantListener *)
        DDS_alloc(sizeof(struct DDS_DomainParticipantListener), NULL);
}
