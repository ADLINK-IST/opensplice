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

#ifndef U__PUBLISHER_H
#define U__PUBLISHER_H

#include "u_publisher.h"

u_result
u_publisherAddWriter (
    u_publisher _this,
    u_writer writer);

u_result
u_publisherRemoveWriter (
    u_publisher _this,
    u_writer writer);

u_result
u_publisherInit (
    u_publisher _this,
    u_participant participant);

#endif
