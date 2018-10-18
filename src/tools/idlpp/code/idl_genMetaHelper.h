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
#ifndef IDL_GENMETAHELPER_H
#define IDL_GENMETAHELPER_H

#include "idl_scope.h"

#include "c_metabase.h"

char *idl_genXMLmeta (c_type type, c_bool escapeQuote);

/*
 * dds#2745
 * Allows to replace
 * 'A very very big string'
 * by :
 * 'a very"
 * "very big"
 * "string'
 */
char *idl_cutXMLmeta (char *meta, c_ulong *nrOfElements, size_t *descriptorLength);

const char *idl_internalTypeNameForBuiltinTopic(const char *typeName);

#endif /* IDL_GENMETAHELPER_H */
