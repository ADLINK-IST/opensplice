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
#ifndef SACPP_MAPPING_H
#define SACPP_MAPPING_H

#ifdef _WIN32
#pragma warning( disable: 4251 )
#endif

#include "sacpp_dds_basic_types.h"
#include "sacpp_LocalObject.h"
#include "sacpp_ValueBase.h"
#include "sacpp_UserException.h"
#include "sacpp_SystemException.h"
#include "sacpp_Exception.h"
#include "sacpp_ExceptionInitializer.h"

#include "mapping/UVAL.h"
#include "mapping/ARRAY.h"
#include "mapping/IFACE.h"
#include "mapping/Memory.h"
#include "mapping/SEQ.h"
#include "mapping/String.h"
#include "mapping/STRUCT.h"
#include "mapping/UFL.h"
#include "mapping/BFL.h"
#include "mapping/UOBJ.h"
#include "mapping/UVL.h"
#include "mapping/BVL.h"
#include "mapping/VALUE.h"
#include "mapping/BSTR.h"

#endif /* SACPP_MAPPING_H */
