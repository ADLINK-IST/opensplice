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
#ifndef CMA__TYPES_H_
#define CMA__TYPES_H_

#include "c_typebase.h"

#define C_TYPEDEF(type, name) C_STRUCT(type) *name

/* C_SIZEOF uses C_STRUCT to determine the size; this is not safe for cma_object;
 * instead the macro could be defined as C_SIZEOF(name) sizeof(*name), but to
 * prevent confusion the macro is undeffed. */
#undef C_SIZEOF

C_CLASS(cma_object);
C_CLASS(cma_thread);
C_CLASS(cma_service);
C_CLASS(cma_configuration);

/* Classes that aren't reference-counted */
C_CLASS(cma_threadStates);
C_CLASS(cma_logConfig);

#endif /* CMA__TYPES_H_ */
