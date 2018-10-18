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

#ifndef OS_RETCODE_H
#define OS_RETCODE_H

/** \brief Return code definitions to replace "result" types in OpenSplice
 *
 * Codes that have an equivalent in the DDS specification, are mapped to the
 * same value so that they may be propagated without the need for conversion.
 *
 * Codes without an equivalent in the DDS specification, must leave room for
 * future expansion of DDS specification mapped return codes.
 */
#define OS_RETCODE_OK (0)
#define OS_RETCODE_ERROR (1)
#define OS_RETCODE_UNSUPPORTED (2)
#define OS_RETCODE_BAD_PARAMETER (3)
#define OS_RETCODE_PRECONDITION_NOT_MET (4)
#define OS_RETCODE_OUT_OF_RESOURCES (5)
#define OS_RETCODE_NOT_ENABLED (6)
#define OS_RETCODE_IMMUTABLE_POLICY (7)
#define OS_RETCODE_INCONSISTENT_POLICY (8)
#define OS_RETCODE_ALREADY_DELETED (9)
#define OS_RETCODE_TIMEOUT (10)
#define OS_RETCODE_NO_DATA (11)
#define OS_RETCODE_ILLEGAL_OPERATION (12)

/** \brief Flag definitions required during transition to OS_RETCODE_*
 *
 * Although strictly speaking the abstraction layer can't have knowledge about
 * layers stacked on top of it, the names chosen for OS_RETCODE_IDENT_* are
 * there solely for the purpose of readability.
 */
#define OS_RETCODE_ID_SHIFT (8)

#define OS_RETCODE_ID_OS_RESULT (1 << OS_RETCODE_ID_SHIFT)
#define OS_RETCODE_ID_UT_RESULT (2 << OS_RETCODE_ID_SHIFT)
#define OS_RETCODE_ID_V_RESULT (3 << OS_RETCODE_ID_SHIFT)
#define OS_RETCODE_ID_V_WRITE_RESULT (4 << OS_RETCODE_ID_SHIFT)
#define OS_RETCODE_ID_V_DATAREADER_RESULT (5 << OS_RETCODE_ID_SHIFT)

#define OS_RETCODE_ID_MASK                \
    (                                     \
        OS_RETCODE_ID_OS_RESULT |         \
        OS_RETCODE_ID_UT_RESULT |         \
        OS_RETCODE_ID_V_RESULT |          \
        OS_RETCODE_ID_V_WRITE_RESULT |    \
        OS_RETCODE_ID_V_DATAREADER_RESULT \
    )

#endif /* OS_RETCODE_H */
