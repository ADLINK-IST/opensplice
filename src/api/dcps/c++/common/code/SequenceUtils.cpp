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
#include "c_stringSupport.h"
#include "SequenceUtils.h"
#include "ReportUtils.h"

namespace DDS {
namespace OpenSplice {
namespace Utils {

/*
 * private
 */
static DDS::ReturnCode_t
memcopyIntoArray(
    const void       *elems,
    const DDS::ULong  elemCnt,
    const DDS::ULong  elemSize,
    c_array     &array,
    c_long      &arrayCnt)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    assert(elemCnt > 0);

    /* Only re-alloc space when we don't have enough. */
    if (arrayCnt < (c_long)elemCnt) {
        if (array != NULL) {
            os_free(array);
        }
        array = (c_array)os_malloc(elemCnt * elemSize);
    }
    /* Remember the new size. */
    arrayCnt = elemCnt;

    /* Copy when everything went right. */
    if (array != NULL) {
        memcpy(array,
               elems,
               elemCnt * elemSize);
    } else {
        arrayCnt  = 0;
        result = DDS::RETCODE_OUT_OF_RESOURCES;
    }

    return result;
}

};
};
};


/*
 * Sequence validations
 */
DDS::ReturnCode_t
DDS::OpenSplice::Utils::stringSeqenceIsValid (
    const DDS::StringSeq &seq)
{
    ULong i;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    /* A sequence itself is always valid within c++:
     * Just check the content. */
    for ( i = 0; result == DDS::RETCODE_OK && (i < seq.length()); i++ ) {
        if ( !seq[i] ) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "StringSeq is invalid, element '%d' = NULL", i);
        }
    }

    return result;
}

/*
 * Sequence comparison
 */
DDS::Boolean
DDS::OpenSplice::Utils::octSeqIsEqual (
    const DDS::octSeq &a,
    const DDS::octSeq &b)
{
    DDS::Boolean equal = FALSE;
    DDS::ULong i, j, n;

    if (&a != &b) {
        n = a.length ();
        j = b.length ();

        if (n == j) {
            for (i = 0; i < n && a[i] == b[i]; i++) {
                /* do nothing */
            }
            if (i == n) {
                equal = TRUE;
            }
        }
    } else {
        equal = TRUE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::stringSeqIsEqual (
    const DDS::StringSeq &a,
    const DDS::StringSeq &b)
{
    DDS::Boolean equal = FALSE;
    DDS::ULong i, j, n;

    if (&a != &b) {
        n = a.length ();
        j = b.length ();

        if (j == n) {
            for (i = 0; i < n && strcmp (a[i], b[i]) == 0; i++) {
                /* do nothing */
            }
            if (i == n) {
                equal = TRUE;
            }
        }
    } else {
        equal = TRUE;
    }

    return equal;
}


/*
 * Sequence conversions
 */

DDS::Char**
DDS::OpenSplice::Utils::stringSeqToStringArray(
        const DDS::StringSeq &from, DDS::Boolean emptyAllowed)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    unsigned long i;
    char **to = NULL;

    if(from.length() > 0) {
        to = (char**) os_malloc((os_size_t) (from.length() * sizeof(char *)));
        for (i = 0; i < from.length(); i++) {
            if (result == DDS::RETCODE_OK) {
                if (from[i]) {
                    to[i] = os_strdup(from[i]);
                } else {
                    to[i] = NULL;
                    if (!emptyAllowed) {
                        result = DDS::RETCODE_ERROR;
                        /* Continue the loop to reset all remaining strings,
                         * to be able to properly free the string array. */
                    }
                }
            } else {
                to[i] = NULL;
            }
        }
        if (result != DDS::RETCODE_OK) {
            DDS::OpenSplice::Utils::freeStringArray(to, from.length());
            to = NULL;
        }
    }

    return to;
}

void
DDS::OpenSplice::Utils::freeStringArray(
        DDS::Char** array, const DDS::ULong size)
{
    DDS::ULong i;

    if (array) {
        for (i =0; i < size; i++) {
            if (array[i]) {
                os_free((void*)array[i]);
            }
        }
        os_free((void*)array);
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceIn(
        const DDS::octSeq &from,
        c_array &to,
        c_long  &size)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (from.length() > 0) {
        /* This also takes care of the memory handling of the destination array. */
        result = memcopyIntoArray(&(from[0]), from.length(), sizeof(char), to, size);
    } else {
        if (to != NULL) {
            os_free(to);
        }
        to = NULL;
        size = 0;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceIn(
        const DDS::QosPolicyCountSeq &from,
        c_array &to,
        c_long  &size)
{
    OS_UNUSED_ARG(from);
    OS_UNUSED_ARG(to);
    OS_UNUSED_ARG(size);
    /* Not needed yet. */
    assert(FALSE);
    return DDS::RETCODE_ERROR;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceIn(
        const DDS::StringSeq &from,
        char *&to,
        const char *delimiter)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    unsigned long i;
    unsigned long size = 0;

    assert(delimiter);

    for ( i = 0; i < from.length(); i++ ) {
        size += strlen(from[i]);
    }
    if ( size > 0 ) {
        if (to != NULL) {
            os_free(to);
        }
        size += (from.length() * strlen(delimiter)) + 1;
        to = (c_string) os_malloc(size);
        to[0] = '\0';
        for ( i = 0; i < from.length(); i++ ) {
            if ( from[i] ) {
                if ( i != 0 ) {
                    os_strcat(to, delimiter);
                }
                os_strcat(to, from[i]);
            }
        }
    } else {
        if (to == NULL) {
            to = (c_string)os_malloc(1);
        }
        to[0] = '\0';
    }

    return result;
}



DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceOut(
        const c_array from,
        const c_long  size,
        DDS::octSeq  &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;

    to.length(size);
    if ((c_long)(to.length()) == size)
    {
        result = DDS::RETCODE_OK;
        if (size > 0)
        {
            memcpy(&(to[0]), from, size);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceOut(
        const c_array           from,
        const c_long            size,
        DDS::QosPolicyCountSeq &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
    int n=0;

    to.length(size);
    if ((c_long)(to.length()) == size)
    {
        for (DDS::Long i = 0; (i < V_POLICY_ID_COUNT) && (i < size); i++)
        {
          if (((c_long *)from)[i] != 0) {
              to[n].policy_id = i;
              to[n++].count = ((c_long *)from)[i];
          }
        }
        result = DDS::RETCODE_OK;
    }

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceOut(
        const c_long from[],
        const c_long size,
        DDS::QosPolicyCountSeq &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
    int n=0;

    to.length(size);
    if ((c_long)(to.length()) == size)
    {
        for (DDS::Long i = 0; (i < V_POLICY_ID_COUNT) && (i < size); i++)
        {
            if (from[i] != 0) {
                to[n].policy_id = i;
                to[n++].count = from[i];
            }
        }
        result = DDS::RETCODE_OK;
    }

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::Utils::copySequenceOut(
        const char *from,
        const char *delimiter,
        DDS::StringSeq &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::ULong size  = 0UL;
    DDS::ULong i;
    DDS::Char* str;
    c_iter iter;

    assert(delimiter);

    if ( from != NULL ) {
        iter = c_splitString(from, delimiter);
        if ( iter ) {
            size = c_iterLength(iter);
            to.length(size);
            for ( i = 0UL; i < size; i++ ) {
                str = (DDS::Char*)c_iterTakeFirst(iter);
                to[i] = DDS::string_dup(str);
                os_free(str);
                if ( to[i].in() == NULL ) {
                     result = DDS::RETCODE_BAD_PARAMETER;
                }
            }
            c_iterFree(iter);
        } else {
            to.length(0);
        }
    } else {
        to.length(0);
    }
    return result;
}
