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


/**
 * @file
 */

#include "org/opensplice/domain/Domain.hpp"

#include "u_domain.h"

uint32_t org::opensplice::domain::default_id()
{
    /* Be aware: the semantic meaning of the user layer terminology is
     * exactly opposite to that of the language bindings. So ANY on the
     * User Layer is default on the language binding.
     */
    return U_DOMAIN_ID_ANY;
}
