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

#ifndef U__DATAREADER_H
#define U__DATAREADER_H

#include "u_dataReader.h"

u_result
u_dataReaderInit (
    u_dataReader _this,
    u_subscriber s);

u_result
u_dataReaderAddView(
    u_dataReader _this,
    u_dataView view);

u_result
u_dataReaderRemoveView(
    u_dataReader _this,
    u_dataView view);

u_result
u_dataReaderTopicName(
    u_dataReader _this,
    c_char **name);

#endif
