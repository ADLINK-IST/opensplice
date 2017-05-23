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

/****************************************************************
 * Implementation for heap memory management                    *
 ****************************************************************/

/** \file os/code/os_heap.c
 *  \brief Heap memory management - allocate and free memory from heap
 *
 * Heap memory management provides an abstraction which enables the
 * possibility to set alternative heap memory management services.
 * This may be required for debugging purposes or improve the memory
 * management if the standard is not sufficient.
 *
 * Heap memory management provides services for allocating memory
 * from heap and releasing allocated memory to heap.
 */

#include "os_heap.h"

/* include OS specific heap memory management implementation 	*/
#include "code/os_heap.c"
