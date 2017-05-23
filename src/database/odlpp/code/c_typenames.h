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

#ifndef C_TYPENAMES_H
#define C_TYPENAMES_H

#include "c_typebase.h"
#include "c_metabase.h"

typedef enum c_scopeWhen {
    C_SCOPE_ALWAYS,
    C_SCOPE_NEVER,
    C_SCOPE_SMART
} c_scopeWhen;

c_char *c_getScopedTypeName(c_metaObject scope,
                            c_type type,
                            c_char *separator,
                            c_scopeWhen scopeWhen);

c_char *c_getScopedConstName(c_metaObject scope,
                             c_constant c,
                             c_char *separator,
                             c_scopeWhen scopeWhen);


#endif /* C_TYPENAMES_H */
