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

#include "v_instance.h"

#include "v_public.h"
#include "v__dataReaderInstance.h"

#include "vortex_os.h"
#include "os_report.h"

void
v_instanceInit(
    v_instance _this,
    v_entity entity)
{
    assert(C_TYPECHECK(_this, v_instance));

    /* Public part is initialised at reader or writer */

    v_publicInit(v_public(_this));
    _this->state = L_EMPTY | L_NOWRITERS;
    _this->entity = (c_voidp)entity;
}

void
v_instanceDeinit(
    v_instance _this)
{
    assert(C_TYPECHECK(_this, v_instance));

    _this->entity = NULL;
    v_publicDeinit(v_public(_this));
}

