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
#ifndef _FACE_CALLBACK_H_
#define _FACE_CALLBACK_H_

#include "FACE_common.h"
#include "FACE_TS_common.h"

namespace FACE
{
    template <typename TYPE>
    struct Read_Callback {
        typedef void (*send_event)(
                            /* in    */ const FACE::TRANSACTION_ID_TYPE &transaction_id,
                            /* inout */       TYPE                      &message,
                            /* in    */ const FACE::MESSAGE_TYPE_GUID   &message_type_id,
                            /* in    */ const FACE::MESSAGE_SIZE_TYPE   &message_size,
                            /* in    */ const FACE::WAITSET_TYPE        &waitset,
                            /* out   */       FACE::RETURN_CODE_TYPE    &return_code);
    };
}

#endif /* _FACE_CALLBACK_H_ */
