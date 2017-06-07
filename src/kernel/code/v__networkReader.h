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

#ifndef V__NETWORKREADER_H
#define V__NETWORKREADER_H

#include "v_networkReader.h"

#define NW_BLOCKING_READER

/* ----------------------------- v_networkReader ----------------------- */

/* Protected methods to be used by v_reader only */

c_bool
v_networkReaderSubscribeGroup(
    v_networkReader _this,
    v_group group);

c_bool
v_networkReaderUnSubscribeGroup(
    v_networkReader _this,
    v_group group);


/* Protected methods to be used by v_networkReaderEntry only */

c_bool
v_networkReaderWrite(
    v_networkReader _this,
    v_message message,
    v_networkReaderEntry entry,
    c_ulong sequenceNumber,
    v_gid sender,
    c_bool sendTo,
    v_gid receiver);

#endif
