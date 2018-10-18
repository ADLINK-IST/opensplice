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
#ifndef IDL_GENSIMULINKHELPER_H
#define IDL_GENSIMULINKHELPER_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "idl_scope.h"
#include "idl_program.h"
#include "idl_typeSpecifier.h"
#include "ut_collection.h"

ut_table simulink_propertiesTable;
ut_table simulink_nameTable;
char *simulink_propertiesFileName;

c_char *simulinkScope(idl_scope typeSpec);
ut_table simulink_createTable();
void simulink_readProperties(char *fileName, ut_table table);
void simulink_writeProperties(ut_table table, const os_char *fileName);
c_char *simulinkGetDataType(idl_typeSpec typeSpec);
c_char *simulinkClassName(idl_typeSpec typeSpec);
c_char *idl_SimulinkTypeFromTypeSpec(idl_typeSpec typeSpec);
const c_char *simulinkKeyListAnnotation(idl_scope scope,const char *typeName);
const c_char *simulinkIdlFileAnnotation(idl_scope scope);
c_char *simulinkSeqClassName(idl_typeSeq seqSpec);
c_char *simulinkSeqClassRef(idl_typeSeq seqSpec, c_ulong showWarning);
c_ulong simulinkSequenceMax(idl_typeSeq typeSeq);
c_ulong simulinkStringMax(idl_typeBasic typeString);
c_char *simulinkSeqTypeName(idl_typeSpec typeSpec);
c_char *simulinkSeqWrappedTypeNameRef(idl_typeSpec typeSpec);
c_char *simulinkSeqDimensions(idl_typeSeq typeSeq);
c_char *simulinkTypeAnnotation(idl_typeSpec typeSpec);
c_ulong simulink_arrayNDimensions(idl_typeArray typeArray);
c_ulong simulink_getStrMaxDimension();
void simulink_setStrMaxDimension(c_ulong stringMax);
c_ulong simulink_containSeqType(idl_typeSpec typeSpec);
const c_char *simulink_getClassNameFromName(idl_scope scope, const char *name, c_ulong showWarning);
void simulink_recordStringBound(idl_typeSpec typeSpec, idl_scope scope, const char *name);
c_ulong simulink_seqTypeBound(idl_typeSeq typeSeq, idl_scope scope, const char *name);
void simulink_arrayGetDimensions(idl_typeArray typeArray,c_ulong dimensions[]);
c_char *simulink_formatDimensions(c_ulong dimensions[],c_ulong nDimensions);
c_char *simulink_sequenceTypeDimensions(idl_typeSpec typeSpec,c_ulong seqMax);
#endif /* IDL_GENSIMULINKHELPER_H */
