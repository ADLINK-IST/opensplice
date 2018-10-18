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
#ifndef DDS_OPENSPLICE_MISCUTILS_H
#define DDS_OPENSPLICE_MISCUTILS_H

#include "u_user.h"
#include "ccpp.h"
#include "cpp_dcps_if.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


namespace DDS {
namespace OpenSplice {
namespace Utils {


/*
 * There isn't currently a better spot to hold these functions.
 */


/*
 * Validations
 */
DDS::ReturnCode_t booleanIsValid (const DDS::Boolean    value);
DDS::ReturnCode_t durationIsValid(const DDS::Duration_t &duration);
DDS::ReturnCode_t timeIsValid    (const DDS::Time_t     &time, os_int64 maxSupportedSeconds);

/*
 * Comparison
 */
DDS::Boolean durationIsEqual (
    const DDS::Duration_t &a,
    const DDS::Duration_t &b);

DDS::Boolean timeIsEqual (
    const DDS::Time_t &a,
    const DDS::Time_t &b);

/*
 * Conversions
 */
DDS::ReturnCode_t  copyDurationIn (const DDS::Duration_t &from, v_duration      &to); /* deprecated */
DDS::ReturnCode_t  copyDurationOut(const v_duration      &from, DDS::Duration_t &to); /* deprecated */

DDS::ReturnCode_t  copyDurationIn (const DDS::Duration_t &from, os_duration     &to);
DDS::ReturnCode_t  copyDurationOut(const os_duration     &from, DDS::Duration_t &to);

DDS::ReturnCode_t  copyTimeIn (const DDS::Time_t &from, os_timeW    &to, os_int64 maxSupportedSeconds);
DDS::ReturnCode_t  copyTimeOut(const os_timeW    &from, DDS::Time_t &to);

OS_API const DDS::Char* returnCodeToString(DDS::ReturnCode_t code);

DDS::ReturnCode_t observableExists(u_observable observable);

} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#undef OS_API

#endif /* DDS_OPENSPLICE_MISCUTILS_H */


