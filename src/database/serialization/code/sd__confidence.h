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
/** \file services/serialization/code/sd__confidence.h
 *  \brief Macro for doing confidence checks, to be used
 *         by all serialization code.
 *
 *  If NDEBUG is not defined, the confidence check will expand to a normal
 *  assert statement. Otherwise, it will us os_report to report an error.
 */

#ifndef SD__CONFIDENCE_H
#define SD__CONFIDENCE_H

#include "os_report.h"


#if !defined(NDEBUG)

#define SD_CONFIDENCE(expr) assert(expr)


#elif defined(SPLICE_HOST_test)

#define SD_CONFIDENCE_DEFAULT_TYPE     OS_ERROR
#define SD_CONFIDENCE_DEFAULT_CONTEXT  "Serialization/deserialization service"
#define SD_CONFIDENCE_DEFAULT_CODE     0

#define SD_CONFIDENCE(expr)                      \
    if (!(expr)) {                               \
        OS_REPORT(SD_CONFIDENCE_DEFAULT_TYPE,    \
                  SD_CONFIDENCE_DEFAULT_CONTEXT, \
                  SD_CONFIDENCE_DEFAULT_CODE,    \
                  #expr);                        \
    }


#elif defined(SPLICE_HOST_release)

#define SD_CONFIDENCE(expr)

#else

#define SD_CONFIDENCE(expr)

#endif /* NDEBUG */


#endif  /* SD__CONFIDENCE_H */
