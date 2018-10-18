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
#ifndef DDS_OPENSPLICE_SEQUENCEUTILS_H
#define DDS_OPENSPLICE_SEQUENCEUTILS_H


#include "ccpp.h"
#include "u_user.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


namespace DDS {
namespace OpenSplice {
namespace Utils {


/*
 * Sequence validations
 */
DDS::ReturnCode_t stringSeqenceIsValid(const DDS::StringSeq &seq);

DDS::Boolean
octSeqIsEqual (
    const DDS::octSeq &a,
    const DDS::octSeq &b);

DDS::Boolean
stringSeqIsEqual (
    const DDS::StringSeq &a,
    const DDS::StringSeq &b);

/*
 * Sequence conversions
 * The memory of the output buffers is managed by these functions as a convenience.
 */
DDS::ReturnCode_t copySequenceIn( const DDS::octSeq            &from,   c_array  &to,   c_long &size);
DDS::ReturnCode_t copySequenceIn( const DDS::QosPolicyCountSeq &from,   c_array  &to,   c_long &size);
DDS::ReturnCode_t copySequenceIn( const DDS::StringSeq         &from,   char *&to,   const char *delimiter);

DDS::Char*        stringSeqToString(const DDS::StringSeq &sequence, const DDS::Char* delimiter);
DDS::Char**       stringSeqToStringArray(const DDS::StringSeq &sequence, DDS::Boolean emptyAllowed);
DDS::ReturnCode_t stringToStringSeq(const DDS::Char* string, const DDS::Char* delimiter, DDS::StringSeq &sequence);

void              freeStringArray(DDS::Char** array, const DDS::ULong size);


DDS::ReturnCode_t copySequenceOut( const c_array  from, const c_long size,          DDS::octSeq            &to);
DDS::ReturnCode_t copySequenceOut( const c_array  from, const c_long size,          DDS::QosPolicyCountSeq &to);
DDS::ReturnCode_t copySequenceOut( const char *from,    const char *delimiter,      DDS::StringSeq         &to);
DDS::ReturnCode_t copySequenceOut( const c_long from[], const c_long size,          DDS::QosPolicyCountSeq &to);


template<typename SEQ, typename TYPE>
void appendSequenceItem(SEQ &sequence, TYPE item) {
    DDS::ULong length;

    /* Increase buffer when needed.
     * Buffer content is retained, but that means that increasing the buffer is
     * a somewhat heavy action. Increasing the buffer with more items at ones
     * is a small performance gain. */
    length = sequence.length();
    if (length == sequence.maximum()) {
        /* Increase the buffer. */
        sequence.length(length + 10);
        /* But we still only have filled it to the previous length. */
        sequence.length(length);
    }

    /* Insert data. */
    sequence.length(length + 1);
    sequence[length] = item;
}


} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#endif /* DDS_OPENSPLICE_SEQUENCEUTILS_H */


